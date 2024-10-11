#include "MouseHoverEater.h"
#include <QEvent>
#include <Qt>
#include <QWidget>


using namespace Mandelbrot::WidgetApp;

bool MouseHoverEater::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::HoverEnter) {
		QWidget* widget = dynamic_cast<QWidget*>(obj);
		if (widget) {
			widget->setCursor(Qt::PointingHandCursor);
		}

		return true;
	}
	if (event->type() == QEvent::HoverLeave) {
		QWidget* widget = dynamic_cast<QWidget*>(obj);
		if (widget) {
			widget->setCursor(Qt::CrossCursor);
		}

		return true;
	}

	return QObject::eventFilter(obj, event);
}
