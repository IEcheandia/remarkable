#ifndef QTCANVAS_H_
#define QTCANVAS_H_

#include <Poco/Path.h>
#include <overlay/overlayCanvas.h>
#include <common/frame.h>

#include <QRasterWindow>

#include "overlay/overlayPrimitive.h"


namespace precitec {
namespace filter {

/**
 * This is an implementation of the OverlayCanvas as a QRasterWindow.
 */
class QtCanvas : public QRasterWindow, public image::OverlayCanvas
{
public:

	QtCanvas(int width =640, int height =480);
	~QtCanvas() override;

	void showBitmap(const Poco::Path &filePath);
	void drawFrame(interface::ImageFrame &rFrame) override;
	void drawPixel(int x, int y, image::Color c) override;
	void drawLine(int x0, int y0, int x1, int y1, image::Color c) override;
	void drawCircle(int x, int y, int r, image::Color c)  override;
	void drawText(std::string text, image::Font font, geo2d::Rect bounds, image::Color c)  override;
	void drawRect(geo2d::Rect rectangle, image::Color c) override;
    void drawPixelList(geo2d::Point p_oPosition, const std::vector<int> & y, image::Color c) override;
    void drawConnectedPixelList(geo2d::Point p_oPosition, const std::vector<int> & y, image::Color c) override;
    void drawImage(geo2d::Point p_oPosition, const image::BImage& p_rImage, const image::OverlayText& p_rTitle) override;


	void setTitle(const std::string& title) override;

    bool isPaused() const
    {
        return m_paused;
    }
    void setPaused()
    {
        m_paused = true;
    }
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent* event) override;
    
private:
    QImage m_backBuffer;
    bool m_paused{false};
};

} // namespace filter
} // namespace precitec

#endif
