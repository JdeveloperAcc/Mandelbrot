#include "MouseHoverEater.h"
#include "RenderThread.h"
#include "Widget.h"
#include <QBuffer>
#include <QColor>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGestureEvent>
#include <QImage>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QMessageLogger>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QPinchGesture>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QRectF>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSlider>
#include <QSslError>
#include <QString>
#include <Qt>
#include <QTranslator>
#include <QUrl>
#include <QWidget>


using namespace Mandelbrot::WidgetApp;

constexpr double DefaultCenterX = -0.637011;
constexpr double DefaultCenterY = -0.0395159;
constexpr double DefaultScale = 0.00403897;
constexpr double ScaleMax = 0.010;
constexpr int ScaleConst = 2000;

constexpr double ZoomInFactor = 0.8;
constexpr double ZoomOutFactor = 1 / ZoomInFactor;
constexpr int ScrollStep = 20;
constexpr int TilesCountMax = 8;

bool Widget::serverUsage = false;
const char* Widget::userAgent = "A Mandelbrot Set Testing App in Qt";

Widget::Widget(QWidget* parent) :
	QWidget(parent),
	m_centerX(DefaultCenterX),
	m_centerY(DefaultCenterY),
	m_pixmapScale(DefaultScale),
	m_changedScale(DefaultScale),
	m_curScale(DefaultScale),
	m_color(Qt::black),
	m_changedColor(Qt::black),
	m_imagesDir("./"),
	m_tiles(nullptr),
	m_widgetOptions(nullptr),
	m_optionsPane(false),
	m_host(QHostAddress::LocalHost),
	m_port(0)
{
	MouseHoverEater* hoverEater = new MouseHoverEater();
	m_menu = new QPushButton("Menu", this);
	m_menu->setGeometry(QRect(QPoint(10, 10), QSize(70, 30)));
	QFont font(m_menu->fontInfo().family(), 10);
	m_menu->setFont(font);
	m_menu->installEventFilter(hoverEater);

	m_button = new QPushButton("Save", this);
	m_button->setGeometry(QRect(QPoint(this->size().width() - 70 - 10, 10), QSize(70, 30)));
	m_button->setFont(font);
	m_button->installEventFilter(hoverEater);

	connect(m_menu, &QPushButton::released, this, &Widget::handleMenu);
	connect(m_button, &QPushButton::released, this, &Widget::handleButton);

	m_help = tr("Zoom with mouse wheel, +/- keys or pinch.  Scroll with arrow keys or by dragging.");
	if (Widget::isServerUsage());
	else
		connect(&m_thread, &RenderThread::renderedImage, this, &Widget::updatePixmap);

	setWindowTitle(tr("Mandelbrot"));
#if QT_CONFIG(cursor)
	setCursor(Qt::CrossCursor);
#endif

	m_manager = new QNetworkAccessManager(this);

	connect(m_manager, &QNetworkAccessManager::finished, this, &Widget::replyFinished);
}

Widget::~Widget()
{
	if (m_tiles)
		delete[] m_tiles;
}

bool Widget::loadConfig(QString name)
{
	QFile loadFile(name);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(
			this,
			tr("Mandelbrot"),
			tr(QString("Can't open the config file " % name).toUtf8().constData()));

		return false;
	}

	QByteArray data = loadFile.readAll();
	QJsonDocument loadDoc = QJsonDocument::fromJson(data);

	read(loadDoc.object());

	return true;
}

void Widget::read(const QJsonObject& json)
{
	if (json.contains("host_ip") && json["host_ip"].isString()) {
		const QString ip = json["host_ip"].toString().trimmed();
		if (ip.toLower() == "localhost")
			m_host = QHostAddress(QHostAddress::LocalHost);
		else
			m_host = QHostAddress(ip);
	}

	if (json.contains("host_port") && json["host_port"].isDouble()) {
		m_port = json["host_port"].toInt();
	}

	if (json.contains("images_dir") && json["images_dir"].isString())
		m_imagesDir = json["images_dir"].toString().trimmed();
}

void Widget::setChangedPixmapScale(double scale)
{
	m_changedScale = scale;
}

void Widget::setChangedColor(Qt::GlobalColor color)
{
	m_changedColor = color;
}

bool Widget::isOptionsPane() const
{
	return m_optionsPane;
}

void Widget::setOptionsPane(bool status)
{
	m_optionsPane = status;
}

void Widget::handleButton()
{
	QDir dir(m_imagesDir);
	if (!dir.exists()) {
		QMessageBox::warning(
			this,
			tr("Mandelbrot"),
			tr(QString("Cannot find the \"" % m_imagesDir % "\"").toUtf8().constData()));
	}
	else {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
			dir.filePath("untitled.jpg"),
			tr("Images (*.jpg *.png)"));

		if (fileName.size() > 0) {
			QFile file(fileName);
			if (!file.open(QIODevice::WriteOnly))
				QMessageBox::warning(this,
					tr("Mandelbrot"),
					tr(QString("Cannot write to the file " % fileName).toUtf8().constData()));
			else
				m_pixmap.save(&file, fileName.toLower().contains(".png") ? "PNG" : "JPG");
		}
	}
}

void Widget::handleMenu()
{
	m_menu->setDisabled(true);

	m_changedScale = m_curScale;
	m_changedColor = m_color;

	m_widgetOptions = new QGroupBox("Options", this);
	m_widgetOptions->setAutoFillBackground(true);
	m_widgetOptions->setGeometry(QRect(QPoint(10, 55), QSize(200, 250)));
	m_widgetOptions->setFocus();

	QLabel* scaleTitle = new QLabel(m_widgetOptions);
	scaleTitle->setText(QString("Pixel Scale: %1").arg(QString::number(m_changedScale, 'f', 4)));
	scaleTitle->setGeometry(QRect(QPoint(20, 40), QSize(150, 20)));
	scaleTitle->setAutoFillBackground(true);
	if (m_changedScale > ScaleMax) {
		QPalette sPal = scaleTitle->palette();
		sPal.setColor(QPalette::WindowText, QColor(Qt::red));
		scaleTitle->setPalette(sPal);
	}

	QSlider* slider = new QSlider(Qt::Horizontal, m_widgetOptions);
	slider->setGeometry(QRect(QPoint(20, 60), QSize(m_widgetOptions->size().width() - 34, 20)));
	slider->setRange(1, 20);
	const int nPos = (int)(m_changedScale * ScaleConst) / 1;
	slider->setSingleStep(1);
	slider->setSliderPosition(nPos);

	connect(slider, &QSlider::valueChanged, [scaleTitle, this](int value) {
		double v = (double)value / ScaleConst;
		scaleTitle->setText(QString("Pixel Scale: %1").arg(QString::number(v, 'f', 4)));
		this->setChangedPixmapScale(v);
		QPalette sPal = scaleTitle->palette();
		if (v > ScaleMax)
			sPal.setColor(QPalette::WindowText, QColor(Qt::red));
		else
			sPal.setColor(QPalette::WindowText, QColor(Qt::black));
		scaleTitle->setPalette(sPal);
		});

	QLabel* colorTitle = new QLabel(m_widgetOptions);
	colorTitle->setText(QString("Color Palette: %1").arg(QVariant::fromValue(m_changedColor).toString()));
	colorTitle->setGeometry(QRect(QPoint(20, 100), QSize(150, 20)));

	m_tiles = new QPushButton[TilesCountMax];

	const int lineCnt = 2;
	const int posCnt = TilesCountMax / 2;
	Qt::GlobalColor nColor;
	for (int i = 0; i < lineCnt; ++i)
		for (int j = 0; j < posCnt; ++j) {
			int inx = i * posCnt + j;

			QPalette pal;
			switch (inx) {
			case 0:
				nColor = Qt::black;
				break;
			case 1:
				nColor = Qt::white;
				break;
			case 2:
				nColor = Qt::red;
				break;
			case 3:
				nColor = Qt::magenta;
				break;
			case 4:
				nColor = Qt::yellow;
				break;
			case 5:
				nColor = Qt::green;
				break;
			case 6:
				nColor = Qt::blue;
				break;
			case 7:
				nColor = Qt::cyan;
				break;
			default:
				nColor = Qt::black;
			}

			pal.setBrush(QPalette::Button, QColor(nColor));

			QPushButton* tile = m_tiles + inx;
			tile->setObjectName(QVariant::fromValue(nColor).toString());
			tile->setParent(m_widgetOptions);
			tile->setGeometry(QRect(QPoint(20 + j * 42, 125 + i * 22), QSize(40, 20)));
			tile->setFlat(true);
			tile->setAutoFillBackground(true);
			tile->setPalette(pal);

			connect(tile, &QPushButton::pressed, [colorTitle, tile, this]() {
				QString name = tile->objectName();
				auto&& metaEnum = QMetaEnum::fromType<Qt::GlobalColor>();
				Qt::GlobalColor receivedVal = static_cast<Qt::GlobalColor>(metaEnum.keyToValue(name.toStdString().c_str()));
				QString text = QVariant::fromValue(receivedVal).toString();
				colorTitle->setText(QString("Color Palette: %1").arg(text));
				this->setChangedColor(receivedVal);
				});
		}

	QPushButton* setUp = new QPushButton("Set Up", m_widgetOptions);
	setUp->move(QPoint(m_widgetOptions->size().width() - 85 - 90, m_widgetOptions->size().height() - 35));
	QPushButton* cancel = new QPushButton("Cancel", m_widgetOptions);
	cancel->move(QPoint(m_widgetOptions->size().width() - 90, m_widgetOptions->size().height() - 35));

	connect(setUp, &QPushButton::pressed, this, &Widget::handleSetUp);
	connect(cancel, &QPushButton::pressed, this, &Widget::handleCancel);

	MouseHoverEater* hoverEater = new MouseHoverEater();
	m_widgetOptions->installEventFilter(hoverEater);
	m_widgetOptions->show();

	setOptionsPane(true);
}

void Widget::handleSetUp()
{
	m_curScale = m_changedScale;
	m_color = m_changedColor;

	QRgb rgb = QColor(m_color).rgb();
	if (Widget::isServerUsage())
		sendRequestToRenderUnit(generateRequestUrl(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb));
	else
		m_thread.render(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb);

	freeUpOptionsPane();
}

void Widget::handleCancel()
{
	freeUpOptionsPane();
}

void Widget::freeUpOptionsPane()
{
	if (m_widgetOptions)
		m_widgetOptions->close();

	m_menu->setEnabled(true);

	if (m_tiles)
		delete[] m_tiles;
	m_tiles = nullptr;
	m_widgetOptions = nullptr;
}

void Widget::paintEvent(QPaintEvent* /* event */)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::black);

	if (m_pixmap.isNull()) {
		painter.setPen(Qt::white);
		painter.drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap,
			tr("Rendering initial image, please wait..."));
		return;
	}

	if (qFuzzyCompare(m_curScale, m_pixmapScale) || isOptionsPane()) {
		painter.drawPixmap(m_pixmapOffset, m_pixmap);
	}
	else {
		const auto previewPixmap = qFuzzyCompare(m_pixmap.devicePixelRatio(), qreal(1))
			? m_pixmap
			: m_pixmap.scaled(m_pixmap.deviceIndependentSize().toSize(), Qt::KeepAspectRatio,
				Qt::SmoothTransformation);
		const double scaleFactor = m_pixmapScale / m_curScale;
		const int newWidth = int(previewPixmap.width() * scaleFactor);
		const int newHeight = int(previewPixmap.height() * scaleFactor);
		const int newX = m_pixmapOffset.x() + (previewPixmap.width() - newWidth) / 2;
		const int newY = m_pixmapOffset.y() + (previewPixmap.height() - newHeight) / 2;

		painter.save();
		painter.translate(newX, newY);
		painter.scale(scaleFactor, scaleFactor);

		const QRectF exposed = painter.transform().inverted().mapRect(rect())
			.adjusted(-1, -1, 1, 1);
		painter.drawPixmap(exposed, previewPixmap, exposed);
		painter.restore();
	}

	const QFontMetrics metrics = painter.fontMetrics();
	if (!m_info.isEmpty()) {
		const int infoWidth = metrics.horizontalAdvance(m_info);
		const int infoHeight = (infoWidth / width() + 1) * (metrics.height() + 5);

		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(0, 0, 0, 127));
		painter.drawRect((width() - infoWidth) / 2 - 5, 0, infoWidth + 10, infoHeight);

		painter.setPen(Qt::white);
		painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, m_info);
	}

	const int helpWidth = metrics.horizontalAdvance(m_help);
	const int helpHeight = (helpWidth / width() + 1) * (metrics.height() + 5);

	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(0, 0, 0, 127));
	painter.drawRect((width() - helpWidth) / 2 - 5, height() - helpHeight, helpWidth + 10,
		helpHeight);

	painter.setPen(Qt::white);
	painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWordWrap, m_help);
}

void Widget::resizeEvent(QResizeEvent* /* event */)
{
	QSize btnSize = m_button->frameSize();
	QSize containerSize = this->size();
	int newX = containerSize.width() - btnSize.width() - 10;  // 10 is an arbitrary margin
	int newY = 10;
	m_button->move(newX, newY);

	QRgb rgb = QColor(m_color).rgb();
	if (Widget::isServerUsage())
		sendRequestToRenderUnit(generateRequestUrl(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb));
	else
		m_thread.render(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb);
}

void Widget::keyPressEvent(QKeyEvent* event)
{
	if (!isOptionsPane()) {
		switch (event->key()) {
		case Qt::Key_Plus:
			zoom(ZoomInFactor);
			break;
		case Qt::Key_Minus:
			zoom(ZoomOutFactor);
			break;
		case Qt::Key_Left:
			scroll(-ScrollStep, 0);
			break;
		case Qt::Key_Right:
			scroll(+ScrollStep, 0);
			break;
		case Qt::Key_Down:
			scroll(0, -ScrollStep);
			break;
		case Qt::Key_Up:
			scroll(0, +ScrollStep);
			break;
		case Qt::Key_Q:
			close();
			break;
		default:
			QWidget::keyPressEvent(event);
		}
	}
	else
		QWidget::keyPressEvent(event);

	if (m_widgetOptions == nullptr)
		setOptionsPane(false);
}

void Widget::wheelEvent(QWheelEvent* event)
{
	if (m_widgetOptions == nullptr) {
		const int numDegrees = event->angleDelta().y() / 8;
		const double numSteps = numDegrees / double(15);
		zoom(pow(ZoomInFactor, numSteps));
	}
}

void Widget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && !isOptionsPane())
		m_lastDragPos = event->position().toPoint();
	else if (m_widgetOptions == nullptr)
		setOptionsPane(false);
}

#ifndef QT_NO_GESTURES
bool Widget::gestureEvent(QGestureEvent* event)
{
	if (auto* pinch = static_cast<QPinchGesture*>(event->gesture(Qt::PinchGesture))) {
		if (pinch->changeFlags().testFlag(QPinchGesture::ScaleFactorChanged))
			zoom(1.0 / pinch->scaleFactor());
		return true;
	}
	return false;
}

bool Widget::event(QEvent* event)
{
	if (event->type() == QEvent::Gesture && m_widgetOptions == nullptr)
		return gestureEvent(static_cast<QGestureEvent*>(event));
	return QWidget::event(event);
}
#endif

void Widget::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton && !isOptionsPane()) {
		m_pixmapOffset += event->position().toPoint() - m_lastDragPos;
		m_lastDragPos = event->position().toPoint();
		update();
	}
	else if (m_widgetOptions == nullptr)
		setOptionsPane(false);
}

void Widget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && !isOptionsPane()) {
		m_pixmapOffset += event->position().toPoint() - m_lastDragPos;
		m_lastDragPos = QPoint();

		const auto pixmapSize = m_pixmap.deviceIndependentSize().toSize();
		const int deltaX = (width() - pixmapSize.width()) / 2 - m_pixmapOffset.x();
		const int deltaY = (height() - pixmapSize.height()) / 2 - m_pixmapOffset.y();
		scroll(deltaX, deltaY);
	}
	else if (m_widgetOptions == nullptr)
		setOptionsPane(false);
}

void Widget::updatePixmap(const QImage& image, double scaleFactor)
{
	if (!m_lastDragPos.isNull())
		return;

	if (!Widget::isServerUsage())
		m_info = image.text(RenderThread::infoKey());

	m_pixmap = QPixmap::fromImage(image);
	m_pixmapOffset = QPoint();
	m_lastDragPos = QPoint();
	m_pixmapScale = scaleFactor;
	update();
}

void Widget::zoom(double zoomFactor)
{
	m_curScale *= zoomFactor;
	update();
	QRgb rgb = QColor(m_color).rgb();
	if (Widget::isServerUsage())
		sendRequestToRenderUnit(generateRequestUrl(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb));
	else
		m_thread.render(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb);
}

void Widget::scroll(int deltaX, int deltaY)
{
	m_centerX += deltaX * m_curScale;
	m_centerY += deltaY * m_curScale;
	update();
	QRgb rgb = QColor(m_color).rgb();
	if (Widget::isServerUsage())
		sendRequestToRenderUnit(generateRequestUrl(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb));
	else
		m_thread.render(m_centerX, m_centerY, m_curScale, size(), devicePixelRatio(), rgb);
}

QUrl Widget::generateRequestUrl(double centerX, double centerY, double scaleFactor,
	QSize resultSize, double devicePixelRatio, QRgb color) const
{
	QString formatted = QString("http://%1:%2/?centerX=%3&centerY=%4&scaleFactor=%5&resultWidth=%6&resultHeight=%7&pixelRatio=%8&color=%9")
		.arg(m_host.toString())
		.arg(m_port)
		.arg(QString::number(centerX))
		.arg(QString::number(centerY))
		.arg(QString::number(scaleFactor))
		.arg(resultSize.width())
		.arg(resultSize.height())
		.arg(QString::number(devicePixelRatio))
		.arg(color);
	return QUrl(formatted);
}

void Widget::sendRequestToRenderUnit(QUrl url)
{
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", Widget::userAgent);

	QNetworkReply* reply = m_manager->get(request);
	connect(reply, &QIODevice::readyRead, this, &Widget::receivedReadyRead);
	connect(reply, &QNetworkReply::errorOccurred, this, &Widget::receivedError);
	connect(reply, &QNetworkReply::sslErrors, this, &Widget::receivedSslErrors);
}

void Widget::receivedReadyRead()
{
}

void Widget::receivedError(QNetworkReply::NetworkError error)
{
	QString str = QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error);
	QString listed = str.split(QRegularExpression("(?<=[a-z])(?=[A-Z])")).join(" ");

	qDebug("The received error from the server: %s", listed.toUtf8().constData());
}

void Widget::receivedSslErrors(const QList<QSslError>&)
{
	qDebug() << "The application doesn't use a SSL connection.";
}

void Widget::replyFinished(QNetworkReply* reply)
{
	if (reply) {
		const auto contentType = reply->header(QNetworkRequest::ContentTypeHeader);
		const auto eTag = reply->header(QNetworkRequest::ETagHeader);
		const QString infoDefined(reply->rawHeader("Info"));
		const QString scaleDefined(reply->rawHeader("Scale-Factor"));

		double scaleFactor(m_pixmapScale);
		QImage image;

		if (!contentType.isValid() || contentType != "text/plain") {
			qDebug() << "Network Reply : Content-Type != text/plain";
			return;
		}

		if (!eTag.isValid()) {
			qDebug() << "Network Reply : A concurrent token is invalid.";
			return;
		}

		if (infoDefined.isNull() || infoDefined.isEmpty()) {
			qDebug() << "Network Reply : A render info is invalid.";
			return;
		}
		else
			m_info = infoDefined;


		if (scaleDefined.isNull() || scaleDefined.isEmpty() || scaleDefined.toDouble() == 0) {
			qDebug() << "Network Reply : A scale factor is invalid.";
			return;
		}
		else
			scaleFactor = scaleDefined.toDouble();

		QByteArray arr = QByteArray::fromBase64(reply->readAll());
		if (image.loadFromData(arr, "BMP"))
			updatePixmap(image, scaleFactor);
		else
			qDebug() << "Network Reply : An image loaded with an import error, a format BMP.";

		connect(reply, &QObject::deleteLater, [reply]() mutable {
			if (reply) {
				disconnect(reply, &QNetworkReply::readyRead, NULL, NULL);
				disconnect(reply, &QNetworkReply::errorOccurred, NULL, NULL);
				disconnect(reply, &QNetworkReply::sslErrors, NULL, NULL);
				delete reply;
				reply = nullptr;
			}});
	}
}
