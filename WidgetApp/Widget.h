#ifndef MANDELBROTWIDGET_H
#define MANDELBROTWIDGET_H

#include <QCoreApplication>
#include <QEvent>
#include <QGestureEvent>
#include <QGroupBox>
#include <QImage>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QResizeEvent>
#include <QSize>
#include <QSlider>
#include <QString>
#include <Qt>
#include <QWheelEvent>
#include <QWidget>
#include "RenderThread.h"


namespace Mandelbrot
{
	namespace WidgetApp
	{
		class Widget : public QWidget
		{
			Q_DECLARE_TR_FUNCTIONS(Widget)

		public:
			Widget(QWidget *parent = nullptr);
			virtual ~Widget();

			bool loadConfig(QString name);
			void read(const QJsonObject &json);

			void setChangedPixmapScale(double scale);
			void setChangedColor(Qt::GlobalColor color);

			bool isOptionsPane() const;
			void setOptionsPane(bool status);

		protected:
			QSize sizeHint() const override { return {1024, 768}; };
			void paintEvent(QPaintEvent *event) override;
			void resizeEvent(QResizeEvent *event) override;
			void keyPressEvent(QKeyEvent *event) override;
		#if QT_CONFIG(wheelevent)
			void wheelEvent(QWheelEvent *event) override;
		#endif
			void mousePressEvent(QMouseEvent *event) override;
			void mouseMoveEvent(QMouseEvent *event) override;
			void mouseReleaseEvent(QMouseEvent *event) override;
		#ifndef QT_NO_GESTURES
			bool event(QEvent *event) override;
		#endif

		private slots:
			void handleButton();
			void handleMenu();
			void handleSetUp();
			void handleCancel();

		private:
			void updatePixmap(const QImage &image, double scaleFactor);
			void zoom(double zoomFactor);
			void scroll(int deltaX, int deltaY);
		#ifndef QT_NO_GESTURES
			bool gestureEvent(QGestureEvent *event);
		#endif
			void freeUpOptionsPane();

			RenderThread m_thread;
			QPixmap m_pixmap;
			QPoint m_pixmapOffset;
			QPoint m_lastDragPos;
			Qt::GlobalColor m_color;
			Qt::GlobalColor m_changedColor;
			QPushButton *m_tiles;
			QPushButton *m_menu;
			QPushButton *m_button;
			QGroupBox *m_widgetOptions;
			QString m_imagesDir;
			QString m_help;
			QString m_info;
			double m_centerX;
			double m_centerY;
			double m_pixmapScale;
			double m_changedScale;
			double m_curScale;
			bool m_optionsPane;
		};
	}
}

#endif
