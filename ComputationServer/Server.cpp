#include "HttpProtocol.h"
#include "RenderThread.h"
#include "Server.h"
#include <QBuffer>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QList>
#include <QLocale>
#include <QFile>
#include <QHostAddress>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <cstring>


using namespace Mandelbrot::ComputationServer;

Server::Server(QObject *parent) :
	QTcpServer(parent),
	m_address(QHostAddress::LocalHost),
	m_port(0)
{
	connect(&m_renderer, &RenderThread::renderedImage, this, &Server::respondImage);
	connect(this, &QTcpServer::newConnection, this, &Server::useConnection);
}

bool Server::loadConfig(QString name)
{
	QFile loadFile(name);
    if (!loadFile.open(QIODevice::ReadOnly)) {
			qWarning() << tr("ComputationServer") <<
				tr(QString("Can't open the config file " % name).toUtf8().constData());

        return false;
    }

    const QByteArray data = loadFile.readAll();
    const QJsonDocument loadDoc = QJsonDocument::fromJson(data);

    read(loadDoc.object());

    return true;
}

void Server::read(const QJsonObject &json)
{
	if (json.contains("listening_ip") && json["listening_ip"].isString()) {
		const QString ip = json["listening_ip"].toString().trimmed();
		if (ip.toLower() == "localhost")
			m_address = QHostAddress(QHostAddress::LocalHost);
		else
			m_address = QHostAddress(ip);
	}
	
	if (json.contains("listening_port") && json["listening_port"].isDouble()) {
		m_port = json["listening_port"].toInt();		
	}
}

quint16 Server::listen()
{
	if (QTcpServer::listen(m_address, m_port))
		return m_port;
	else
		return 0;
}

void Server::useConnection()
{
	QTcpSocket* newSocket = nextPendingConnection();
	if (newSocket) {
		QByteArray buffered;
		int msWait = 3000;
		while (newSocket->waitForReadyRead(msWait)) {
			//qint64 size = newSocket->bytesAvailable();
			buffered.append(newSocket->readAll());
			msWait = 200;
		}

		if (m_connected.indexOf(newSocket) == -1)
			m_connected.append(newSocket);

		QTextStream stream(buffered);
		parseRequest(newSocket->socketDescriptor(), stream);
	}
	else {
		qWarning() << tr("ComputationServer")
			<< tr("Can't open a socket.");
	}
}

void Server::parseRequest(qintptr descriptor, QTextStream& data)
{
	QString message;
	QTextStream stream(&message);

	HttpData request;
	if (lexicalHttpParser(data, request) == true) {
		// Server Rules:
		try {
			QString ip = request[HttpProtocol::HeaderField::Name::HOST].toLower().toLower();
			qsizetype inx = ip.indexOf(':');
			QHostAddress host(ip.left(inx));
			if (host != m_address)
				errorResponse(stream, HttpProtocol::StatusCode::NOT_ACCEPTABLE, HttpProtocol::ReasonPhrase::NOT_ACCEPTABLE);
			else if (request["Method"] != HttpProtocol::Method::GET)
				errorResponse(stream, HttpProtocol::StatusCode::NOT_IMPLEMENTED, HttpProtocol::ReasonPhrase::NOT_IMPLEMENTED);
			else if (request["Uri"].isNull() || request["Uri"].isEmpty() || request["Uri"].length() <= 2 ||
				request["Uri"][0] != '/' || request["Uri"][1] != '?')
				errorResponse(stream, HttpProtocol::StatusCode::NOT_FOUND, HttpProtocol::ReasonPhrase::NOT_FOUND);
			else if (request["Version"] != HttpProtocol::VERSION)
				errorResponse(stream, HttpProtocol::StatusCode::NOT_IMPLEMENTED, HttpProtocol::ReasonPhrase::NOT_IMPLEMENTED);
			else {
				HttpData arguments = readQueryString(request["Uri"]);

				if (arguments["centerX"].isNull() || arguments["centerX"].isEmpty() ||
					arguments["centerY"].isNull() || arguments["centerY"].isEmpty() ||
					arguments["scaleFactor"].isNull() || arguments["scaleFactor"].isEmpty() ||
					arguments["resultWidth"].isNull() || arguments["scaleFactor"].isEmpty() ||
					arguments["resultHeight"].isNull() || arguments["resultHeight"].isEmpty() ||
					arguments["pixelRatio"].isNull() || arguments["pixelRatio"].isEmpty() ||
					arguments["color"].isNull() || arguments["color"].isEmpty())
					errorResponse(stream, HttpProtocol::StatusCode::BAD_REQUEST, HttpProtocol::ReasonPhrase::BAD_REQUEST);
				else {
					double centerX = arguments["centerX"].toDouble();
					double centerY = arguments["centerY"].toDouble();
					double scaleFactor = arguments["scaleFactor"].toDouble();
					int resultWidth = arguments["resultWidth"].toInt();
					int resultHeight = arguments["resultHeight"].toInt();
					QSize size(resultWidth, resultHeight);
					double pixelRatio = arguments["pixelRatio"].toDouble();
					QRgb color = arguments["color"].toUInt();

					m_renderer.render(descriptor, centerX, centerY, scaleFactor, size, pixelRatio, color);
				}
			}
		}
		catch (...) {
			errorResponse(stream, HttpProtocol::StatusCode::INTERNAL_SERVER_ERROR, HttpProtocol::ReasonPhrase::INTERNAL_SERVER_ERROR);
		}
	}
	else
		errorResponse(stream, HttpProtocol::StatusCode::BAD_REQUEST, HttpProtocol::ReasonPhrase::BAD_REQUEST);

	QByteArray buffered(message.toUtf8().constData(), message.length());
	replyMessage(descriptor, buffered);
}

void Server::respondImage(qintptr descriptor, const QImage& image, double scaleFactor)
{
	const QString info = image.text(RenderThread::infoKey());
	QByteArray arr;
	QBuffer buffer(&arr);
	image.save(&buffer, "BMP");
	const QByteArray imgBase64 = arr.toBase64();
	QString message;
	QTextStream stream(&message);

	normalResponse(stream, info, scaleFactor, imgBase64, imgBase64.length(), true);

	QByteArray buffered(message.toUtf8().constData(), message.length());
	replyMessage(descriptor, buffered);
}

bool Server::lexicalHttpParser(QTextStream& stream, HttpData& result) const
{
	QStringList parsed;
	QStringList line;
	bool requestLine = false;
	bool complete = false;

	parsed << stream.readLine();
	while (!stream.atEnd())
		parsed << stream.readLine();

	if (parsed.size() > 0)
		line = parsed[0].split(' ');
	else
		qWarning() << tr("ComputationServer")
				<< tr("Missing a request line!");

	// A request line:
	QString method;
	QString uri;
	QString version;
	if (line.size() == 3) {
		method = line[0];
		uri = line[1];
		version = line[2];

		result["Method"] = method;
		result["Uri"] = uri;
		result["Version"] = version;

		requestLine = true;
	}

	QString tmpName, tmpValue;
	for (int i = 1; i < parsed.size(); ++i) {
		line.clear();
		if (parsed[i].trimmed().size() == 0)
			// A header section has been finished.
			break;
		qsizetype index = parsed[i].indexOf(HttpProtocol::DELIMITER_FIELD);
		line << parsed[i].left(index).trimmed() << parsed[i].sliced(index + 1).trimmed();
		if (line.size() == 2) {
			tmpName = line[0];
			tmpValue = line[1];

			// Checked Fields:
			// . Mandatory Host
			if (tmpName.toLower() == QString(HttpProtocol::HeaderField::Name::HOST).toLower()) {
				result[HttpProtocol::HeaderField::Name::HOST] = tmpValue;

				if (requestLine)
					complete = true;
			}
			// . Optional
			else if (tmpName.toLower() == QString(HttpProtocol::HeaderField::Name::ACCEPT).toLower()) {
				result[HttpProtocol::HeaderField::Name::ACCEPT] = tmpValue;
			}
			else if (tmpName.toLower() == QString(HttpProtocol::HeaderField::Name::CONNECTION).toLower()) {
				result[HttpProtocol::HeaderField::Name::CONNECTION] = tmpValue;
			}
		}
		else
			qWarning() << tr("ComputationServer")
					<< tr("A bad header field detected.");
	}

	return complete;
}

Server::HttpData Server::readQueryString(const QString& uri) const
{
	HttpData container;

	const qsizetype queryMark = uri.indexOf('?');
	const qsizetype fragmentMark = uri.indexOf('#');
	const qsizetype posStart = queryMark + 1;
	const qsizetype posEnd = fragmentMark + 1;

	QStringList line, queryTokens(fragmentMark == -1 ?
		uri.sliced(posStart).split('&') :
		uri.sliced(posStart, posEnd - posStart).split('&'));
	QString tmpName, tmpValue;

	for (int i = 0; i < queryTokens.size(); ++i) {
		line = queryTokens[i].split('=');
		if (line.size() == 2) {
			tmpName = line[0].trimmed();
			tmpValue = line[1].trimmed();

			container[tmpName] = tmpValue;
		}
		else
			qWarning() << tr("ComputationServer")
				<< tr("A bad query string field detected.");
	}

	return container;
}

void Server::replyMessage(qintptr descriptor, const QByteArray &buffered)
{
	// The parent sends a message...
	QTcpSocket *socket = nullptr;
	for (QTcpSocket* s : m_connected) {
		if (s->socketDescriptor() == descriptor) {
			socket = s;
			break;
		}
	}

	if (socket && socket->isOpen()) {
		qint64 sentCnt = socket->write(buffered);
		qint64 cnt = buffered.length();
		if (sentCnt != cnt) {
			qWarning() << tr("ComputationServer") <<
				tr(QString("Response sending with error bytes in %1 counted instead of %2 buffered.")
					.arg(sentCnt).arg(cnt)
					.toUtf8().constData());
		}
		if (sentCnt > 0) {
			socket->flush();
			socket->close();
			m_connected.removeAt(m_connected.indexOf(socket));
		}
	}
}

QTextStream& Server::errorResponse(QTextStream& stream, int statusCode, 
	const char* reasonPhrase, bool connection) const
{
	/*
		HTTP/1.1 404 Not Found
		Date: Wed, 9 October 2024 09:12:18 GMT
		Server: Test Environment (Qt)
		Connection: close
	*/

	stream << HttpProtocol::VERSION << HttpProtocol::DELIMITER_TERM
		<< statusCode << HttpProtocol::DELIMITER_TERM << reasonPhrase << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::DATE << HttpProtocol::DELIMITER_FIELD << HttpProtocol::DELIMITER_TERM
		<< Server::utcTimeEnglishText() << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::SERVER << HttpProtocol::DELIMITER_FIELD	<< HttpProtocol::DELIMITER_TERM
		<< HttpProtocol::HeaderField::Value::SERVER << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::CONNECTION << HttpProtocol::DELIMITER_FIELD	<< HttpProtocol::DELIMITER_TERM
		<< (connection ? HttpProtocol::HeaderField::Value::CONNECTION_KEEP_ALIVE : HttpProtocol::HeaderField::Value::CONNECTION_CLOSE);

	return stream;
}

QTextStream& Server::normalResponse(QTextStream& stream, const QString& info,
	double scaleFactor, const QByteArray& content, qsizetype length, bool connection) const
{
	/*
		HTTP/1.1 200 OK
		Date: Mon, 23 May 2005 22:38:34 GMT
		Content-Type: text/plain
		Content-Length: 155
		Info: Information about an image processing time [ms]
		E-Token: 9daba689bfc0c5b7ef021e0b0304822c
		Server: Test Environment (Qt)
		Scale-Factor: 0.06855
		Connection: keep-alive

		Content...
	*/

	const QString scale = QString::number(scaleFactor, 'f', 5);
	stream << HttpProtocol::VERSION << HttpProtocol::DELIMITER_TERM
		<< HttpProtocol::StatusCode::OK << HttpProtocol::DELIMITER_TERM << HttpProtocol::ReasonPhrase::OK << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::DATE << HttpProtocol::DELIMITER_FIELD << HttpProtocol::DELIMITER_TERM
		<< Server::utcTimeEnglishText() << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::CONTENT_TYPE << HttpProtocol::DELIMITER_FIELD << HttpProtocol::DELIMITER_TERM
		<< HttpProtocol::HeaderField::Value::CONTENT_TYPE_TEXT_PLAIN << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::CONTENT_LENGTH << HttpProtocol::DELIMITER_FIELD << HttpProtocol::DELIMITER_TERM
		<< length << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::INFO << HttpProtocol::DELIMITER_FIELD << HttpProtocol::DELIMITER_TERM
		<< info << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::E_TOKEN << HttpProtocol::DELIMITER_FIELD << HttpProtocol::DELIMITER_TERM
		<< Server::generateToken() << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::SERVER << HttpProtocol::DELIMITER_FIELD	<< HttpProtocol::DELIMITER_TERM
		<< HttpProtocol::HeaderField::Value::SERVER << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::SCALE_FACTOR << HttpProtocol::DELIMITER_FIELD << HttpProtocol::DELIMITER_TERM
		<< scale << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::HeaderField::Name::CONNECTION << HttpProtocol::DELIMITER_FIELD	<< HttpProtocol::DELIMITER_TERM
		<< (connection ? HttpProtocol::HeaderField::Value::CONNECTION_KEEP_ALIVE : HttpProtocol::HeaderField::Value::CONNECTION_CLOSE) << HttpProtocol::DELIMITER_LINE
		<< HttpProtocol::DELIMITER_LINE
		<< content;

	return stream;
}

QString Server::utcTimeEnglishText()
{
	const QDateTime dtUtc = QDateTime::currentDateTime().toUTC();
	const QLocale testLocale = QLocale(QLocale::English, QLocale::UnitedKingdom);
	const QString dtText = testLocale.toString(dtUtc, "ddd, d MMMM yyyy hh:mm:ss %1").arg("GMT");
	return dtText;
}

QString Server::generateToken()
{
	const QDateTime dtUtc = QDateTime::currentDateTime().toUTC();
	const QLocale testLocale = QLocale(QLocale::English, QLocale::UnitedKingdom);
	const char* text = testLocale.toString(dtUtc).toUtf8().constData();
	QCryptographicHash md5(QCryptographicHash::Md5);
	md5.addData(text, std::strlen(text));

	return QString(md5.result().toHex());
}
