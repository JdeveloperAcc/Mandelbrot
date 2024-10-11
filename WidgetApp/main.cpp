#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QJsonObject>
#include <QString>
#include "Widget.h"


using namespace Mandelbrot::WidgetApp;
using namespace Qt::Literals::StringLiterals;

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QCommandLineParser parser;
	parser.setApplicationDescription(u"Qt Mandelbrot Set in Example"_s);
	parser.addHelpOption();
	parser.addVersionOption();
	QCommandLineOption configOption(u"config"_s, u"Config file path"_s, u"path"_s);
	parser.addOption(configOption);
	QCommandLineOption serverOption(u"server"_s, u"Server data feed in use"_s, u"is_server"_s);
	parser.addOption(serverOption);
	parser.process(app);

	if (parser.isSet(serverOption)) {
		const auto serverStr = parser.value(serverOption).toLower();
		bool ok = serverStr == "false" || serverStr == "true" || serverStr == "0" || serverStr == "1";
		const bool isServer = serverStr == "true" || serverStr == "1";
		if (!ok) {
			qWarning() << "Invalid value:" << serverStr.toUtf8().constData();
			return EXIT_FAILURE;
		}
		Widget::setServerUsage(isServer);
	}

	Widget widget;
	if (parser.isSet(configOption)) {
		const auto cfgPath = parser.value(configOption);
		widget.loadConfig(cfgPath);
	}
	widget.grabGesture(Qt::PinchGesture);
	widget.show();
	return app.exec();
}
