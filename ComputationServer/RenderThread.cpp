#include "RenderThread.h"
#include <QElapsedTimer>
#include <QObject>
#include <QImage>
#include <QMutexLocker>
#include <QSize>
#include <QString>
#include <QTextStream>
#include <QThread>


using namespace Mandelbrot::ComputationServer;

int RenderThread::numPasses = RenderThread::NumberPassesMin;

RenderThread::RenderThread(QObject *parent)
    : QThread(parent),
    m_descriptor(0),
    m_centerX(0),
    m_centerY(0),
    m_scaleFactor(0),
    m_devicePixelRatio(0),
    m_baseColor(0),
    m_restart(false),
    m_abort(false)
{
}

RenderThread::~RenderThread()
{
    m_mutex.lock();
    m_abort = true;
    m_condition.wakeOne();
    m_mutex.unlock();

    wait();
}

void RenderThread::render(qintptr descriptor,
        double centerX, double centerY, double scaleFactor,
        QSize resultSize, double devicePixelRatio, QRgb color)
{
    QMutexLocker locker(&m_mutex);

    this->m_descriptor = descriptor;
    this->m_centerX = centerX;
    this->m_centerY = centerY;
    this->m_scaleFactor = scaleFactor;
    this->m_devicePixelRatio = devicePixelRatio;
    this->m_resultSize = resultSize;
    this->m_baseColor = color;

    if (!isRunning()) {
        start(LowPriority);
    } else {
        m_restart = true;
        m_condition.wakeOne();
    }
}

void RenderThread::run()
{
    QElapsedTimer timer;
    forever {
        m_mutex.lock();
        const qintptr descript = this->m_descriptor;
        const double devicePixelRatio = this->m_devicePixelRatio;
        const QSize resultSize = this->m_resultSize * devicePixelRatio;
        const QRgb resultBaseColor = this->m_baseColor;
        const double requestedScaleFactor = this->m_scaleFactor;
        const double scaleFactor = requestedScaleFactor / devicePixelRatio;
        const double centerX = this->m_centerX;
        const double centerY = this->m_centerY;
        m_mutex.unlock();

        const QColor c(resultBaseColor);
        const int r = c.red();
        const int g = c.green();
        const int b = c.blue();

        uint colormap[ColormapSize];
        for (int i = 0; i < ColormapSize; ++i)
            colormap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize), r, g, b);
		
		const int halfWidth = resultSize.width() / 2;
        const int halfHeight = resultSize.height() / 2;
        QImage image(resultSize, QImage::Format_RGB32);
        image.setDevicePixelRatio(devicePixelRatio);

        int pass = 0;
        while (pass < numPasses) {
            const int MaxIterations = (1 << (2 * pass + 6)) + 32;
            constexpr int Limit = 4;
            bool allBlack = true;

            timer.restart();

            for (int y = -halfHeight; y < halfHeight; ++y) {
                if (m_restart)
                    break;
                if (m_abort)
                    return;

                auto scanLine =
                        reinterpret_cast<uint *>(image.scanLine(y + halfHeight));
                const double ay = centerY + (y * scaleFactor);

                for (int x = -halfWidth; x < halfWidth; ++x) {
                    const double ax = centerX + (x * scaleFactor);
                    double a1 = ax;
                    double b1 = ay;
                    int numIterations = 0;

                    do {
                        ++numIterations;
                        const double a2 = (a1 * a1) - (b1 * b1) + ax;
                        const double b2 = (2 * a1 * b1) + ay;
                        if ((a2 * a2) + (b2 * b2) > Limit)
                            break;

                        ++numIterations;
                        a1 = (a2 * a2) - (b2 * b2) + ax;
                        b1 = (2 * a2 * b2) + ay;
                        if ((a1 * a1) + (b1 * b1) > Limit)
                            break;
                    } while (numIterations < MaxIterations);

                    if (numIterations < MaxIterations) {
                        *scanLine++ = colormap[numIterations % ColormapSize];
                        allBlack = false;
                    } else {
                        *scanLine++ = qRgb(0, 0, 0);
                    }
                }
            }

            if (allBlack && pass == 0) {
                pass = 4;
            } else {
                if (!m_restart) {
                    QString message;
                    QTextStream str(&message);
                    str << " Pass " << (pass + 1) << '/' << numPasses
                        << ", max iterations: " << MaxIterations << ", time: ";
                    const auto elapsed = timer.elapsed();
                    if (elapsed > 2000)
                        str << (elapsed / 1000) << 's';
                    else
                        str << elapsed << "ms";
                    image.setText(infoKey(), message);

                    emit renderedImage(descript, image, requestedScaleFactor);
                }
                ++pass;
            }
        }
		
		m_mutex.lock();
        if (!m_restart) {
            m_condition.wait(&m_mutex);
        }
        m_restart = false;
        m_mutex.unlock();
    }
}

uint RenderThread::rgbFromWaveLength(double wave, double r, double g, double b)
{
    if (wave >= 380.0 && wave <= 440.0) {
        r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
        b = 1.0;
    } else if (wave >= 440.0 && wave <= 490.0) {
        g = (wave - 440.0) / (490.0 - 440.0);
        b = 1.0;
    } else if (wave >= 490.0 && wave <= 510.0) {
        g = 1.0;
        b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
    } else if (wave >= 510.0 && wave <= 580.0) {
        r = (wave - 510.0) / (580.0 - 510.0);
        g = 1.0;
    } else if (wave >= 580.0 && wave <= 645.0) {
        r = 1.0;
        g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
    } else if (wave >= 645.0 && wave <= 780.0) {
        r = 1.0;
    }

    double s = 1.0;
    if (wave > 700.0)
        s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
    else if (wave <  420.0)
        s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

    r = std::pow(r * s, 0.8);
    g = std::pow(g * s, 0.8);
    b = std::pow(b * s, 0.8);
    return qRgb(int(r * 255), int(g * 255), int(b * 255));
}
