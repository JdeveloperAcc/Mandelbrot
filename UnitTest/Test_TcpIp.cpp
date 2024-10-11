#include <QByteArray>
#include <QDateTime>
#include <QLocale>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QTextStream>
#include "Test_TcpIp.h"


using namespace Mandelbrot::UnitTest;

void Test_TcpIp::parseRequestMessage()
{
	// test case 1
	QStringList listed;
	listed << "GET /?centerX=0.087303&centerY=-1.23480 HTTP/1.1"
		<< "Host: 127.0.0.1:8055"
		<< "Accept: text/plain"
		<< "Connection: keep-alive";
	QString message = listed.join("\r\n");

	QByteArray buffer(message.toStdString().c_str(), message.size());

	QTextStream stream(buffer);
	QStringList parsed;
	QStringList line;

	parsed << stream.readLine();
	while (!stream.atEnd())
		parsed << stream.readLine();

	if (parsed.size() > 0)
		line = parsed[0].split(' ');
	else
		QVERIFY2(parsed.size() > 0, "Missing a request line!");

	QString method;
	QString uri;
	QString version;
	// A request line:
	if (line.size() == 3) {
		method = line[0];
		uri = line[1];
		version = line[2];
	}

	QString hostName;
	QString hostValue;
	QString acceptName;
	QString acceptValue;
	QString connectionName;
	QString connectionValue;
	QString tmpName, tmpValue;
	for (int i = 1; i < parsed.size(); ++i) {
		line.clear();
		if (parsed[i].trimmed().size() == 0)
			// A header section has been finished.
			break;
		qsizetype index = parsed[i].indexOf(':');
		line << parsed[i].left(index).trimmed() << parsed[i].sliced(index + 1).trimmed();
		if (line.size() == 2) {
			tmpName = line[0];
			tmpValue = line[1];

			// Checked Fields:
			// . Mandatory Host
			if (tmpName.toLower() == "host") {
				hostName = tmpName;
				hostValue = tmpValue;
			}
			// . Optional
			else if (tmpName.toLower() == "accept") {
				acceptName = tmpName;
				acceptValue = tmpValue;
			}
			else if (tmpName.toLower() == "connection") {
				connectionName = tmpName;
				connectionValue = tmpValue;
			}
		}
		else
			QVERIFY2(line.size() == 2, "A bad header field detected.");
	}

	QVERIFY(method == "GET");
	QVERIFY(uri == "/?centerX=0.087303&centerY=-1.23480");
	QVERIFY(version == "HTTP/1.1");
	QVERIFY(acceptName.toLower() == QString("Accept").toLower());
	QVERIFY(acceptValue == "text/plain");
	QVERIFY(connectionName.toLower() == QString("Connection").toLower());
	QVERIFY(connectionValue == "keep-alive");

	// test case 2
	QStringList queryString(uri.sliced(2).split('&'));
	QString name1 = "centerX";
	QString name2 = "centerY";
	QString value1, value2;

	for (int i = 0; i < queryString.size(); ++i) {
		line = queryString[i].split('=');
		if (line.size() == 2) {
			tmpName = line[0].trimmed();
			tmpValue = line[1].trimmed();

			// Checked Fields:
			if (tmpName == name1) {
				value1 = tmpValue;
			}
			else if (tmpName == name2) {
				value2 = tmpValue;
			}
		}
		else
			QVERIFY2(line.size() == 2, "A bad query string field detected.");
	}

	QVERIFY(name1 == "centerX" && value1 == "0.087303");
	QVERIFY(name2 == "centerY" && value2 == "-1.23480");

	// test case 3
	QString line3 = "Host  : localhost ";
	QString tmpName3, tmpValue3;

	line = line3.split(':');
	if (line.size() == 2) {
		tmpName3 = line[0].trimmed();
		tmpValue3 = line[1].trimmed();
	}
	else
		QVERIFY2(line.size() == 2, "A bad header field detected.");

	// test case 4
	QString line4 = "Host  : 127.0.0.1:8008 ";
	QString tmpName4, tmpValue4;

	line = line4.split(':');
	if (line.size() == 2) {
		tmpName4 = line[0].trimmed();
		tmpValue4 = line[1].trimmed();
		QFAIL("Header's field detection failed.");
	}
	else
		QVERIFY2(line.size() == 3, "A line has got 3 tokens.");

	// test case 5
	QString uri5("/?centerX  = 0.087303 & centerY = -1.23480 ");
	QStringList queryString5(uri5.sliced(2).split('&'));
	QString name51 = "centerX";
	QString name52 = "centerY";
	QString value51, value52;

	for (int i = 0; i < queryString.size(); ++i) {
		line = queryString[i].split('=');
		if (line.size() == 2) {
			tmpName = line[0].trimmed();
			tmpValue = line[1].trimmed();

			// Checked Fields:
			if (tmpName == name51) {
				value51 = tmpValue;
			}
			else if (tmpName == name52) {
				value52 = tmpValue;
			}
		}
		else
			QVERIFY2(line.size() == 2, "A bad query string field detected.");
	}

	QVERIFY(name51 == "centerX" && value51 == "0.087303");
	QVERIFY(name52 == "centerY" && value52 == "-1.23480");
}

void Test_TcpIp::parseRespondMessage()
{
	//test case 1
	QStringList listed;
	listed << "HTTP/1.1 200 OK"
		<< "Date: Mon, 23 May 2005 22:38:34 GMT"
		<< "Content-Type: text/html; charset=UTF-8"
		<< "Content-Length: 155"
		<< "Last-Modified: Wed, 08 Jan 2003 23:11:55 GM"
		<< "Server: Apache/1.3.3.7 (Unix) (Red-Hat/Linux)"
		<< "ETag: \"3f80f-1b6-3e1cb03b\""
		<< "Accept-Ranges: bytes"
		<< "Connection: close"
		<< ""
		<< "<html>"
		<< "  <head>"
		<< "    <title>An Example Page</title>"
		<< "  </head>"
		<< "  <body>"
		<< "    <p>Hello World, this is a very simple HTML document.</p>"
		<< "  </body>"
		<< "</html>";
	QString message = listed.join("\r\n");

	QByteArray buffer(message.toStdString().c_str(), message.size());

	QTextStream stream(buffer);
	QStringList parsed;
	QStringList line;

	parsed << stream.readLine();
	while (!stream.atEnd())
		parsed << stream.readLine();

	if (parsed.size() > 0)
		line = parsed[0].split(' ');
	else
		QVERIFY2(parsed.size() > 0, "Missing a status line!");

	QString version;
	QString statusCode;
	QString reasonPhrase;
	// A status line:
	if (line.size() >= 3) {
		version = line[0];
		statusCode = line[1];
		reasonPhrase = line.sliced(2).join(" ");
	}

	QString dateName;
	QString dateValue;
	QString contentTypeName;
	QString contentTypeValue;
	QString contentLengthName;
	QString contentLengthValue;
	QString serverName;
	QString serverValue;
	QString connectionName;
	QString connectionValue;
	QString tmpName, tmpValue;
	int i = 1;
	for (; i < parsed.size(); ++i) {
		line.clear();
		if (parsed[i].trimmed().size() == 0)
			// A header section has been finished.
			break;
		qsizetype index = parsed[i].indexOf(':');
		line << parsed[i].left(index).trimmed() << parsed[i].sliced(index + 1).trimmed();
		if (line.size() == 2) {
			tmpName = line[0];
			tmpValue = line[1];

			// Checked Fields:
			if (tmpName.toLower() == "date") {
				dateName = tmpName;
				dateValue = tmpValue;
			}
			else if (tmpName.toLower() == "content-type") {
				contentTypeName = tmpName;
				contentTypeValue = tmpValue;
			}
			else if (tmpName.toLower() == "content-length") {
				contentLengthName = tmpName;
				contentLengthValue = tmpValue;
			}
			else if (tmpName.toLower() == "server") {
				serverName = tmpName;
				serverValue = tmpValue;
			}
			else if (tmpName.toLower() == "connection") {
				connectionName = tmpName;
				connectionValue = tmpValue;
			}
		}
		else
			QVERIFY2(line.size() == 2, "A bad header field detected.");
	}

	QVERIFY(version == "HTTP/1.1");
	QVERIFY(statusCode == "200");
	QVERIFY(reasonPhrase == "OK");
	QVERIFY(dateName.toLower() == QString("Date").toLower());
	QVERIFY(dateValue == "Mon, 23 May 2005 22:38:34 GMT");
	QVERIFY(contentTypeName.toLower() == QString("Content-Type").toLower());
	QVERIFY(contentTypeValue == "text/html; charset=UTF-8");
	QVERIFY(contentLengthName.toLower() == QString("Content-Length").toLower());
	QVERIFY(contentLengthValue == "155");
	QVERIFY(serverName.toLower() == QString("Server").toLower());
	QVERIFY(serverValue == "Apache/1.3.3.7 (Unix) (Red-Hat/Linux)");
	QVERIFY(connectionName.toLower() == QString("Connection").toLower());
	QVERIFY(connectionValue == "close");

	// test case 2
	if (i + 1 < parsed.size()) {
		QString content = parsed.sliced(i + 1).join("\r\n");
		qsizetype lenght = contentLengthValue.toInt();
		QVERIFY(content.length() == lenght);
	}
	else
		QFAIL("Missing content.");
}

void Test_TcpIp::checkServerDateFormat()
{
	// test case 1
	QDateTime dt;
	dt.setDate(QDate(2005, 5, 23));;
	dt.setTime(QTime(22, 38, 34, 0));
	dt.setTimeZone(QTimeZone::utc());

	QString origin = dt.toString();

	QString text = QString("%1 %2").arg(dt.toString("MM/dd/yyyy hh:mm:ss"), "GMT");
	QVERIFY(text == "05/23/2005 22:38:34 GMT");

	// test case 2
	QDateTime dtUtc = QDateTime::currentDateTime().toUTC();
	QLocale testLocale = QLocale(QLocale::English, QLocale::UnitedKingdom);
	QString dateTimeText = testLocale.toString(dtUtc, "dddd d of MMMM, yyyy, hh:mm:ss %1").arg("GMT");
}

void Test_TcpIp::checkRegularExpression()
{
	// test case 1
	QNetworkReply::NetworkError error(QNetworkReply::NetworkError::NetworkSessionFailedError);
	QString text = QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error);

	//  "NetworkSessionFailedError"
	//1."N      S      F     E    "
	//2." etwork ession ailed rror"
	//3."Ne t w o r k Se s s i o n Fa i l e d Er r o r "
	//4." Network Session Failed Error"
	//5."Network Session Failed Error"
	//6."Networ essio aile rror"
	//7."NetworkSessionFailedError"
	//8."NetworkSessionFailedError"
	QString listed1 = text.split(QRegularExpression("[a-z]")).join(" ");
	QString listed2 = text.split(QRegularExpression("[A-Z]")).join(" ");
	QString listed3 = text.split(QRegularExpression("(?<=[a-z])")).join(" ");
	QString listed4 = text.split(QRegularExpression("(?=[A-Z])")).join(" ");
	QString listed5 = text.split(QRegularExpression("(?<=[a-z])(?=[A-Z])")).join(" ");
	QString listed6 = text.split(QRegularExpression("([a-z])([A-Z])")).join(" ");
	QString listed7 = text.split(QRegularExpression("(?=[a-z])(?=[A-Z])")).join(" ");
	QString listed8 = text.split(QRegularExpression("(?<=[a-z])(?<=[A-Z])")).join(" ");
	QVERIFY(listed1 == "N      S      F     E    ");
	QVERIFY(listed2 == " etwork ession ailed rror");
	QVERIFY(listed3 == "Ne t w o r k Se s s i o n Fa i l e d Er r o r ");
	QVERIFY(listed4 == " Network Session Failed Error");
	QVERIFY(listed5 == "Network Session Failed Error");
	QVERIFY(listed6 == "Networ essio aile rror");
	QVERIFY(listed7 == "NetworkSessionFailedError");
	QVERIFY(listed8 == "NetworkSessionFailedError");
}

QTEST_MAIN(Test_TcpIp)
