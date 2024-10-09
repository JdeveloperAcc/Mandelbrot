#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "RenderThread.h"
#include "Server.h"


using namespace Mandelbrot::ComputationServer;
using namespace Qt::Literals::StringLiterals;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

	QCommandLineParser parser;
    parser.setApplicationDescription(u"Qt Mandelbrot Set Computation Server"_s);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption configOption(u"config"_s, u"Config file path"_s, u"path"_s);
    parser.addOption(configOption);
    QCommandLineOption passesOption(u"passes"_s, u"Number of passes (1-8)"_s, u"passes"_s);
    parser.addOption(passesOption);
    parser.process(app);

    if (parser.isSet(passesOption)) {
        const auto passesStr = parser.value(passesOption);
        bool ok;
        const int passes = passesStr.toInt(&ok);
        if (!ok || passes < 1 || passes > 8) {
            qWarning() << "Invalid value:" << passesStr;
            return EXIT_FAILURE;
        }
        RenderThread::setNumPasses(passes);
    }
 
	Server server;
	if (parser.isSet(configOption)) {
        const auto cfgPath = parser.value(configOption);
        server.loadConfig(cfgPath);
    }
	
	const auto port = server.listen();
    if (!port) {
        qWarning() << QCoreApplication::translate("ComputationServer",
            "Server failed to listen on a port.");
        return EXIT_FAILURE;
    }

    const auto address = server.serverAddress().toString();
    qInfo().noquote()
        << QCoreApplication::translate("ComputationServer",
            "Running on http://%1:%2/"
            "(Press CTRL+C to quit)").arg(address, port);

    return app.exec();
}
