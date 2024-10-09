#ifndef MANDELBROTSERVER_H
#define MANDELBROTSERVER_H 

#include <QList>
#include <QMap>
#include <QTcpServer>
#include <QTcpSocket>
#include "RenderThread.h"


namespace Mandelbrot
{
	namespace ComputationServer
	{
		class Server : public QTcpServer
		{
		public:
			Server(QObject* parent = nullptr);

			bool loadConfig(QString name);
			void read(const QJsonObject& json);

			quint16 listen();

		private slots:
			void useConnection();

		private:
			void parseRequest(qintptr descriptor, QTextStream& data);
			void respondImage(qintptr descriptor, const QImage& image, double scaleFactor);

			typedef QMap<QString, QString> HttpData;

			bool lexicalHttpParser(QTextStream& stream, HttpData& result) const;
			HttpData readQueryString(const QString& uri) const;

			void replyMessage(qintptr descriptor, const QByteArray& buffered);

			QTextStream& errorResponse(QTextStream& stream, int statusCode,
				const char* reasonPhrase, bool connection = false) const;
			QTextStream& normalResponse(QTextStream& stream, const QString& info,
				double scaleFactor, const QByteArray& content, qsizetype length, bool connection = false) const;

			// optionally
			static QString generateToken();

			static QString utcTimeEnglishText();

			QList<QTcpSocket*> m_connected;
			RenderThread m_renderer;
			QHostAddress m_address;
			quint16 m_port;
		};
	}
}

#endif
