#include "HttpProtocol.h"


using namespace Mandelbrot::ComputationServer;

const char* HttpProtocol::VERSION = "HTTP/1.1";

const char HttpProtocol::DELIMITER_TERM = ' ';
const char HttpProtocol::DELIMITER_FIELD = ':';
const char* HttpProtocol::DELIMITER_LINE = "\r\n";

const char* HttpProtocol::Method::GET = "GET";

const int HttpProtocol::StatusCode::OK = 200;
const int HttpProtocol::StatusCode::BAD_REQUEST = 400;
const int HttpProtocol::StatusCode::NOT_FOUND = 404;
const int HttpProtocol::StatusCode::NOT_ACCEPTABLE = 406;
const int HttpProtocol::StatusCode::INTERNAL_SERVER_ERROR = 500;
const int HttpProtocol::StatusCode::NOT_IMPLEMENTED = 501;
const int HttpProtocol::StatusCode::SERVICE_UNAVAILABLE = 503;
const int HttpProtocol::StatusCode::GATEWAY_TIMEOUT = 504;

const char* HttpProtocol::ReasonPhrase::OK = "OK";
const char* HttpProtocol::ReasonPhrase::BAD_REQUEST = "Bad Request";
const char* HttpProtocol::ReasonPhrase::NOT_FOUND = "Not Found";
const char* HttpProtocol::ReasonPhrase::NOT_ACCEPTABLE = "Not Acceptable";
const char* HttpProtocol::ReasonPhrase::INTERNAL_SERVER_ERROR = "Internal Server Error";
const char* HttpProtocol::ReasonPhrase::NOT_IMPLEMENTED = "Not Implemented";
const char* HttpProtocol::ReasonPhrase::SERVICE_UNAVAILABLE = "Service Unavailable";
const char* HttpProtocol::ReasonPhrase::GATEWAY_TIMEOUT = "Gateway Timeout";

const char* HttpProtocol::HeaderField::Name::HOST = "Host";
const char* HttpProtocol::HeaderField::Name::ACCEPT = "Accept";
const char* HttpProtocol::HeaderField::Name::CONNECTION = "Connection";
const char* HttpProtocol::HeaderField::Name::DATE = "Date";
const char* HttpProtocol::HeaderField::Name::CONTENT_TYPE = "Content-Type";
const char* HttpProtocol::HeaderField::Name::CONTENT_LENGTH = "Content-Length";
const char* HttpProtocol::HeaderField::Name::SERVER = "Server";
const char* HttpProtocol::HeaderField::Name::ETAG = "ETag";
const char* HttpProtocol::HeaderField::Name::INFO = "Info";
const char* HttpProtocol::HeaderField::Name::SCALE_FACTOR = "Scale-Factor";

const char* HttpProtocol::HeaderField::Value::ACCEPT = "text/plain";
const char* HttpProtocol::HeaderField::Value::CONNECTION_CLOSE = "close";
const char* HttpProtocol::HeaderField::Value::CONNECTION_KEEP_ALIVE = "keep-alive";
const char* HttpProtocol::HeaderField::Value::CONTENT_TYPE_IMAGE_JPEG = "image/jpeg";
const char* HttpProtocol::HeaderField::Value::CONTENT_TYPE_IMAGE_PNG = "image/png";
const char* HttpProtocol::HeaderField::Value::CONTENT_TYPE_TEXT_PLAIN = "text/plain";
const char* HttpProtocol::HeaderField::Value::SERVER = "Test Environment (Qt)";
