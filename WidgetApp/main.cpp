#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QJsonObject>
#include <QString>
#include "Widget.h"


using namespace Mandelbrot::WidgetApp;
using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Qt Mandelbrot Set in Example"_s);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption configOption(u"config"_s, u"Config file path"_s, u"path"_s);
    parser.addOption(configOption);
    parser.process(app);

    Widget widget;
    if (parser.isSet(configOption)) {
        const auto cfgPath = parser.value(configOption);
        widget.loadConfig(cfgPath);
    }
    widget.grabGesture(Qt::PinchGesture);
    widget.show();
    return app.exec();
}
