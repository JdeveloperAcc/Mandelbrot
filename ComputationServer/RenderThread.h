#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QSize>
#include <QString>
#include <QThread>
#include <QWaitCondition>


namespace Mandelbrot
{
    namespace ComputationServer
    {
        class RenderThread : public QThread
        {
            Q_OBJECT

        public:
            RenderThread(QObject* parent = nullptr);
            ~RenderThread();

            void render(qintptr descriptor,
                double centerX, double centerY, double scaleFactor, QSize resultSize,
                double devicePixelRatio, QRgb color);

            static void setNumPasses(int n) { numPasses = n; }

            static QString infoKey() { return QStringLiteral("info"); }

        signals:
            void renderedImage(qintptr descriptor, const QImage& image, double scaleFactor);

        protected:
            void run() override;

        private:
            static uint rgbFromWaveLength(double wave, double r, double g, double b);

            qintptr m_descriptor;
            QMutex m_mutex;
            QWaitCondition m_condition;
            double m_centerX;
            double m_centerY;
            double m_scaleFactor;
            double m_devicePixelRatio;
            QSize m_resultSize;
            QRgb m_baseColor;
            static int numPasses;
            bool m_restart;
            bool m_abort;

            static constexpr int NumberPassesMin = 2;
            static constexpr int ColormapSize = 512;
        };
    }
}

#endif
