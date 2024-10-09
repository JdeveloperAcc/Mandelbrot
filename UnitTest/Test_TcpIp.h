#include <QObject>


namespace Mandelbrot
{
	namespace UnitTest
	{       
		class Test_TcpIp: public QObject
		{
			Q_OBJECT
		private slots:
			void parseRequestMessage();
			void parseRespondMessage();
			void checkServerDateFormat();
		};
	}
}
