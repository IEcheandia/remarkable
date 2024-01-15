#include "QtCanvas.h"

#include <QGuiApplication>
#include <QColor>
#include <QKeyEvent>
#include <QPainter>

namespace precitec {
namespace filter {
    
QString toQt(const std::string &string)
{
    return QString::fromLocal8Bit(string.c_str());
}

static QColor toQt(const image::Color &c)
{
    return QColor(c.red, c.green, c.blue, c.alpha);
}

static QRect toQt(const geo2d::Rect rect)
{
    return QRect(rect.x().start(), rect.y().start(), rect.width(), rect.height());
}

static QFont toQt(const image::Font &font)
{
    QFont f(toQt(font.name), font.size);
    f.setBold(font.bold);
    f.setItalic(font.italic);
    return f;
}

QtCanvas::QtCanvas(int width, int height)
    : QRasterWindow()
    , OverlayCanvas(width, height)
    , m_backBuffer(width, height, QImage::Format_ARGB32_Premultiplied)
{
    m_backBuffer.fill(Qt::black);
    setWidth(width);
    setHeight(height);
    show();
    update();
}

QtCanvas::~QtCanvas() = default;

void QtCanvas::showBitmap(const Poco::Path &filePath)
{
    QImage img(toQt(filePath.toString()));
    QPainter p;
    p.begin(&m_backBuffer);
    p.drawImage(0, 0, img);
    p.end();
    update();
}

void QtCanvas::drawFrame(interface::ImageFrame &rFrame)
{
    // send out previous frame
    QCoreApplication::processEvents();
    
    QImage img(rFrame.data().data(), rFrame.data().width(), rFrame.data().height(), QImage::Format_Grayscale8);
    if (img.isNull()) {
        return;
    }
    QPainter p;
    p.begin(&m_backBuffer);
    const auto& rTrafo = *rFrame.context().trafo();
    p.drawImage(rTrafo.dx(), rTrafo.dy(), img);
    p.end();
    update();
}

void QtCanvas::drawPixel(int x, int y, image::Color c)
{
    QPainter p;
    p.begin(&m_backBuffer);
    p.setPen(toQt(c));
    p.drawPoint(x, y);
    p.end();
    update();
}

void QtCanvas::drawPixelList(geo2d::Point p_oPosition, const std::vector<int> & y, image::Color c)
{
    QPainter p;
    p.begin(&m_backBuffer);
    p.setPen(toQt(c));
    for (unsigned int i = 0, iEnd = y.size(); i < iEnd; ++i)
    {
        p.drawPoint(i+p_oPosition.x, y[i]+p_oPosition.y);
    }
    p.end();
    update();
}

void QtCanvas::drawConnectedPixelList(geo2d::Point p_oPosition, const std::vector<int> & y, image::Color c)
{
    if (y.size() <=1 )
    {
        drawPixelList(p_oPosition, y, c);
        return;
    }

    QPainter p;
    p.begin(&m_backBuffer);
    p.setPen(toQt(c));

    int x0 = p_oPosition.x;
    int y0 = y[0] + p_oPosition.y;
    for (unsigned int i = 1, iEnd = y.size(); i < iEnd; ++i)
    {
        int x1 = i + p_oPosition.x;
        int y1 = y[i] + p_oPosition.y;
        p.drawLine(x0, y0, x1, y1);
        x0 = x1;
        y0 = y1;
    }

    p.end();
    update();
}

void QtCanvas::drawImage(geo2d::Point p_oPosition, const image::BImage& p_rImage, const image::OverlayText& p_rTitle)
{
    QImage image(p_rImage.width(), p_rImage.height(), QImage::Format_Grayscale8);

    if ( p_rImage.height() > 2 && p_rImage.isContiguos() && (p_rImage.rowBegin(1) - p_rImage.rowBegin(0)) % 4 == 0)
    {
        //QImage constructor: data must be 32-bit aligned
        std::copy(p_rImage.begin(), p_rImage.end(), image.bits());
    }
    else
    {
        for (auto y = 0; y < p_rImage.height(); ++y)
        {
            std::copy(p_rImage.rowBegin(y), p_rImage.rowEnd(y), image.scanLine(y));
        }
    }

    QPainter p;
    p.begin(&m_backBuffer);
    p.drawImage(p_oPosition.x, p_oPosition.y, image);

    p.setFont(toQt(p_rTitle.font_));
    p.setPen(toQt(p_rTitle.color_));
    p.drawText(toQt(p_rTitle.bounds_), toQt(p_rTitle.text_));

    p.end();
    update();
}

void QtCanvas::drawLine(int x0, int y0, int x1, int y1, image::Color c)
{
    QPainter p;
    p.begin(&m_backBuffer);
    p.setPen(toQt(c));
    p.drawLine(x0, y0, x1, y1);
    p.end();
    update();
}

void QtCanvas::drawCircle(int x, int y, int r, image::Color c) 
{
    QPainter p;
    p.begin(&m_backBuffer);
    p.setPen(toQt(c));
    p.drawEllipse(QPoint(x, y), r, r);
    p.end();
    update();
}

void QtCanvas::drawText(std::string text, image::Font font, geo2d::Rect bounds, image::Color c)
{
    QPainter p;
    p.begin(&m_backBuffer);
    p.setFont(toQt(font));
    p.setPen(toQt(c));
    p.drawText(toQt(bounds), toQt(text));
    p.end();
    update();
}

void QtCanvas::drawRect(geo2d::Rect rectangle, image::Color c)
{
    QPainter p;
    p.begin(&m_backBuffer);
    p.setPen(toQt(c));
    p.drawRect(toQt(rectangle));
    p.end();
    update();
}

void QtCanvas::setTitle(const std::string& title)
{
    QRasterWindow::setTitle(toQt(title));
}

void QtCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.drawImage(0, 0, m_backBuffer);
}

void QtCanvas::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
    {
        return;
    }
    if (event->key() == Qt::Key_Space)
    {
        m_paused = !m_paused;
    }
}

}
}
