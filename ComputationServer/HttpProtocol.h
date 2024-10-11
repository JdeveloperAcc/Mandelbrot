#ifndef HTTPPROTOCOL_H
#define HTTPPROTOCOL_H


namespace Mandelbrot
{
	namespace ComputationServer
	{
		class HttpProtocol
		{
		public:
			// A Http version
			static const char* VERSION;

			static const char DELIMITER_TERM;

			static const char DELIMITER_FIELD;

			static const char* DELIMITER_LINE;

			// A request line
			class Method
			{
			public:
				static const char* GET;
			};

			// A status line
			class StatusCode
			{
			public:
				static const int OK;

				static const int BAD_REQUEST;

				static const int NOT_FOUND;

				static const int NOT_ACCEPTABLE;

				static const int INTERNAL_SERVER_ERROR;

				static const int NOT_IMPLEMENTED;

				static const int SERVICE_UNAVAILABLE;

				static const int GATEWAY_TIMEOUT;
			};

			class ReasonPhrase
			{
			public:
				static const char* OK;

				static const char* BAD_REQUEST;

				static const char* NOT_FOUND;

				static const char* NOT_ACCEPTABLE;

				static const char* INTERNAL_SERVER_ERROR;

				static const char* NOT_IMPLEMENTED;

				static const char* SERVICE_UNAVAILABLE;

				static const char* GATEWAY_TIMEOUT;
			};

			// A Header field
			class HeaderField
			{
			public:
				class Name
				{
				public:
					static const char* HOST;

					static const char* ACCEPT;

					static const char* CONNECTION;

					static const char* DATE;

					static const char* CONTENT_TYPE;

					static const char* CONTENT_LENGTH;

					static const char* SERVER;

					static const char* ETAG;

					// User defined ones
					static const char* INFO;

					static const char* SCALE_FACTOR;
				};

				class Value
				{
				public:
					static const char* ACCEPT;

					static const char* CONNECTION_CLOSE;

					static const char* CONNECTION_KEEP_ALIVE;

					static const char* CONTENT_TYPE_IMAGE_JPEG;

					static const char* CONTENT_TYPE_IMAGE_PNG;

					static const char* CONTENT_TYPE_TEXT_PLAIN;

					// User defined ones
					static const char* SERVER;
				};
			};

		private:
			HttpProtocol() {};
			HttpProtocol(const HttpProtocol&) {};

			const HttpProtocol& operator=(const HttpProtocol&) { return *this; }
		};
	}
}

#endif
