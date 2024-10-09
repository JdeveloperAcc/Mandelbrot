#ifndef MOUSEHOVEREATER_H
#define MOUSEHOVEREATER_H

#include <QObject>


namespace Mandelbrot
{
	namespace WidgetApp
	{
		class MouseHoverEater : public QObject
		{
			Q_OBJECT

		protected:
			bool eventFilter(QObject* obj, QEvent* event) override;
		};
	}
}

#endif
