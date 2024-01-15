/*
 * QnxCanvas
 *
 *  Created on: 23.02.2011
 *      Author: Stefan Birmanns
 */

#ifndef QNXCANVAS_H_
#define QNXCANVAS_H_

// photon includes
#include <photon/PtWidget.h>
// poco/fliplib includes
#include <Poco/Path.h>
#include <overlay/OverlayCanvas.h>
#include <common/frame.h>

namespace precitec {
namespace filter {

/**
 * This is an implementation of the OverlayCanvas for QNX using the Photon window manager.
 */
class QnxCanvas : public image::OverlayCanvas
{
public:

	/**
	 * QnxCanvas - Constructor
	 * Eine Implementierung des Canvas fuer QNX mit Hilfe von Photon.
	 *
	 * \param width Breite des Canvas
	 * \param height Hoehe des Canvas
	 */
	QnxCanvas(int width =640, int height =480);
	/**
	 * Destructor
	 */
	virtual ~QnxCanvas();

	/**
	 * Show a BMP/Bitmap image by loading the file from harddisk. This function relies on the PxLoadImage Photon function and thereby supports a wide range of BMP files, much more
	 * than our other functions (see bit-depths, compression, etc).
	 */
	void showBitmap(const Poco::Path &filePath);
	/**
	 * Draw an ImageFrame using PgDrawImage. This is not the fastest method to draw/blit an image as it relies on the QNX message interface.
	 * For faster methods see the following article: http://www.qnx.com/developers/articles/article_291_2.html
	 *
	 * \param rFrame interface::ImageFrame object that gets rendered
	 */
	void drawFrame(interface::ImageFrame &rFrame);

	/**
	 * Draw a pixel - simply calls drawLine, if this would get used excessively one should reimplement this...
	 *
	 * \param x x coordinate
	 * \param y y coordinate
	 * \param c image::Color object
	 */
	virtual void drawPixel	(int x, int y, image::Color c);
	/**
	 * Draw a line using PgDrawILine
	 *
	 * \param x0 start x coordinate
	 * \param y0 start y coordinate
	 * \param x1 end x coordinate
	 * \param y1 end y coordinate
	 * \param c image::Color object
	 */
	virtual void drawLine 	(int x0, int y0, int x1, int y1, image::Color c);
	/**
	 * Draw a circle - PgDrawArc
	 *
	 * \param x center x coordinate
	 * \param y center y coordinate
	 * \param r radius
	 * \param c image::Color object
	 */
	virtual void drawCircle (int x, int y, int r, image::Color c);
	/**
	 * Draw text. This function calls PgDrawText and tries to find an appropriate font on the Photon side. If Windows font names are used, this will not work.
	 *
	 * \param text string that is supposed to be shown
	 * \param font image::Font object - see comment above...
	 * \param c image::Color object
	 */
	virtual void drawText   (std::string text, image::Font font, geo2d::Rect bounds, image::Color c);
	/**
	 * Draw a rectangle using PgDrawIRect.
	 *
	 * \param rectangle Rect object with the coordinates and size of the rectangle
	 * \param c image::Color object
	 */
	virtual void drawRect	(geo2d::Rect rectangle, image::Color c);

	/**
	 * Set the window title
	 *
	 * \param title window title
	 */
	void setTitle(const std::string& title);
private:
	PtWidget_t* m_pWin;
};

} // namespace filter
} // namespace precitec

#endif
