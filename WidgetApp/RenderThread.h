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
    namespace WidgetApp
    {
        class RenderThread : public QThread
        {
            Q_OBJECT

        public:
            RenderThread(QObject* parent = nullptr);
            ~RenderThread();

            void render(double centerX, double centerY, double scaleFactor, QSize resultSize,
                double devicePixelRatio, QRgb color);

            static void setNumPasses(int n) { numPasses = n; }

            static QString infoKey() { return QStringLiteral("info"); }

        signals:
            void renderedImage(const QImage& image, double scaleFactor);

        protected:
            void run() override;

        private:
            static uint rgbFromWaveLength(double wave, double r, double g, double b);

            QMutex mutex;
            QWaitCondition condition;
            double centerX;
            double centerY;
            double scaleFactor;
            double devicePixelRatio;
            QSize resultSize;
            QRgb baseColor;
            static int numPasses;
            bool restart = false;
            bool abort = false;

            static constexpr int NumberPassesMin = 2;
            static constexpr int ColormapSize = 512;
        };
    }
}

#endif
