/*
 * QnxCanvas
 *
 *  Created on: 23.02.2011
 *      Author: Stefan Birmanns
 */
// local includes
#include "QnxCanvas.h"
// C includes
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <bitset>
// photon includes
#include <photon/PtWidget.h>
#include <photon/PtWindow.h>
#include <photon/PdDirect.h>
#include <photon/PxImage.h>

///////////////////////////////////////////////////////////////////////////////
// Canvas
///////////////////////////////////////////////////////////////////////////////

namespace precitec {
	using namespace geo2d;
namespace filter {

/**
 * QnxCanvas - Constructor
 * Eine Implementierung des Canvas fuer QNX mit Hilfe von Photon.
 *
 * \param width Breite des Canvas
 * \param height Hoehe des Canvas
 */
QnxCanvas::QnxCanvas(int width, int height) : OverlayCanvas(width, height)
{
	PtArg_t args[4];
	PhDim_t oDim = { (short)width, (short)height };
	PhPoint_t oPos = { 50, 50 };

	// initialize connection to Photon
	PtInit("/dev/photon");
	// set arguments
	PtSetArg(&args[0], Pt_ARG_POS, &oPos, 0);
	PtSetArg(&args[1], Pt_ARG_DIM, &oDim, 0);
	PtSetArg(&args[2], Pt_ARG_CONTAINER_FLAGS, Pt_TRUE, Pt_SHOW_TITLE);
	// create window
	m_pWin = PtCreateWidget(PtWindow, Pt_NO_PARENT, 3, args);
	// realize/open it
	PtRealizeWidget(m_pWin);
}

/**
 * Destructor
 */
QnxCanvas::~QnxCanvas()
{
	/// hide the window and destroy it.
	PtUnrealizeWidget(m_pWin);
	PtDestroyWidget(m_pWin);
}

/**
 * Draw a pixel - simply calls drawLine, if this would get used excessively one should reimplement this...
 *
 * \param x x coordinate
 * \param y y coordinate
 * \param c image::Color object
 */
void QnxCanvas::drawPixel(int x, int y, image::Color c)
{
	drawLine(x,y,x,y,c);
}

/**
 * Draw a line using PgDrawILine
 *
 * \param x0 start x coordinate
 * \param y0 start y coordinate
 * \param x1 end x coordinate
 * \param y1 end y coordinate
 * \param c image::Color object
 */
void QnxCanvas::drawLine(int x0, int y0, int x1, int y1, image::Color c)
{
	// get the widget's canvas
	PhRect_t oCanvas;
	PtWidgetCanvas(m_pWin, &oCanvas);
	// set the clipping area to be the raw widget's canvas.
	PtClipAdd ( m_pWin, &oCanvas);
	// set the drawing position to the widgets upper left coordinate
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);
	// set the color
	PgColor_t oColor = PgRGB( c.red, c.green, c.blue );
	PgSetStrokeColor( oColor );

	// draw the line
	PgDrawILine( x0, y0, x1, y1 );

	// restore translation
	oCanvas.ul.x *= -1;
	oCanvas.ul.y *= -1;
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);
	// unset the clipping area
	PtClipRemove();
}

/**
 * Draw a circle - PgDrawArc
 *
 * \param x center x coordinate
 * \param y center y coordinate
 * \param r radius
 * \param c image::Color object
 */
void QnxCanvas::drawCircle (int x, int y, int r, image::Color c)
{
	// get the widget's canvas
	PhRect_t oCanvas;
	PtWidgetCanvas(m_pWin, &oCanvas);
	// set the clipping area to be the raw widget's canvas.
	PtClipAdd ( m_pWin, &oCanvas);
	// set the drawing position to the widgets upper left coordinate
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);
	// set the color
	PgColor_t oColor = PgRGB( c.red, c.green, c.blue);
	PgSetStrokeColor( oColor );

	// set coordinates
	PhPoint_t oCenter  = { (short)x, (short)y };
	PhPoint_t oRadius  = { (short)r, (short)r };
    // draw unfilled circle.
	PgDrawArc( &oCenter, &oRadius, 0, 0, Pg_DRAW_STROKE | Pg_ARC );

	// restore translation
	oCanvas.ul.x *= -1;
	oCanvas.ul.y *= -1;
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);
	// unset the clipping area
	PtClipRemove();
}

/**
 * Draw text. This function calls PgDrawText and tries to find an appropriate font on the Photon side. If Windows font names are used, this will not work.
 *
 * \param text string that is supposed to be shown
 * \param font image::Font object - see comment above...
 * \param c image::Color object
 */
void QnxCanvas::drawText(std::string text, image::Font font, Rect bounds, image::Color c)
{
	PhPoint_t p = { (short)bounds.x().start(), (short)(bounds.y().start()+font.size) };

	// we have to generate a Photon fontname based on the WM font name
    char pFontName[MAX_FONT_TAG];
    int iFlags = 0;
    if (font.italic) iFlags |= PF_STYLE_ITALIC;
    if (font.bold)   iFlags |= PF_STYLE_BOLD;
    if (PfGenerateFontName( font.name.c_str(), iFlags, font.size, pFontName ) != NULL)
       PgSetFont( pFontName );
    else
    	std::cout << "Warning: Could not find appropriate system font " << font.name.c_str() << std::endl;

	PgColor_t oColor = PgRGB( c.red, c.green, c.blue);
	PgSetTextColor( oColor );
    PgDrawText( text.c_str(), text.size(), &p, 0 );
}

/**
 * Draw a rectangle using PgDrawIRect.
 *
 * \param rectangle Rect object with the coordinates and size of the rectangle
 * \param c image::Color object
 */
void QnxCanvas::drawRect(Rect rectangle, image::Color c)
{
	// get the widget's canvas
	PhRect_t oCanvas;
	PtWidgetCanvas(m_pWin, &oCanvas);
	// set the clipping area to be the raw widget's canvas.
	PtClipAdd ( m_pWin, &oCanvas);
	// set the drawing position to the widgets upper left coordinate
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);
	// set the color
	PgColor_t oColor = PgRGB( c.red, c.green, c.blue);
	PgSetStrokeColor( oColor );

    // draw rectangle
	PgDrawIRect( rectangle.x().start(), rectangle.y().start(), rectangle.x().start()+rectangle.width(), rectangle.y().start()+rectangle.height(), Pg_DRAW_STROKE );

	// restore translation
	oCanvas.ul.x *= -1;
	oCanvas.ul.y *= -1;
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);
	// unset the clipping area
	PtClipRemove();
}

/**
 * Draw an ImageFrame using PgDrawImage. This is not the fastest method to draw/blit an image as it relies on the QNX message interface.
 * For faster methods see the following article: http://www.qnx.com/developers/articles/article_291_2.html
 *
 * \param oFrame interface::ImageFrame object that gets rendered
 */
void QnxCanvas::drawFrame(interface::ImageFrame &rFrame)
{
	// get the widget's canvas
	PhRect_t oCanvas;
	PtWidgetCanvas(m_pWin, &oCanvas);

	// draw the image
	PhDim_t oDim = { (short)rFrame.data().width(), (short)rFrame.data().height() };
	PgSetFillColor( Pg_BLACK );
	PgSetTextColor( Pg_WHITE );
	const interface::Trafo& 	rTrafo = *rFrame.context().trafo();
	Ph_point		oPos;
	oPos.x	= oCanvas.ul.x + rTrafo.dx();
	oPos.y	= oCanvas.ul.y + rTrafo.dy();
	PgSetRegion(PtWidgetRid(m_pWin));
	PgDrawImagemx( rFrame.data().begin(), Pg_IMAGE_GRADIENT_BYTE, &oPos, &oDim, rFrame.data().width(), 0 );
	PgFlush();
}

/**
 * Show a BMP/Bitmap image by loading the file from harddisk. This function relies on the PxLoadImage Photon function and thereby supports a wide range of BMP files, much more
 * than our other functions (see bit-depths, compression, etc).
 */
void QnxCanvas::showBitmap(const Poco::Path &filePath)
{
	// load the image from a file
	PhImage_t* pImg = PxLoadImage( filePath.toString().c_str(), NULL );

	if (pImg == NULL)
	{
		std::cout << "Failed to load image: " << filePath.toString() << std::endl;
		return;
	}

	// get the widget's canvas
	PhRect_t oCanvas;
	PtWidgetCanvas(m_pWin, &oCanvas);
	// set the clipping area to be the raw widget's canvas.
	PtClipAdd ( m_pWin, &oCanvas);
	// set the drawing position to the widgets upper left coordinate
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);

	// draw the image
	PhPoint_t oPos  = { 0, 0 };
	PgDrawPhImage(&oPos, pImg, 0);

	// restore translation
	oCanvas.ul.x *= -1;
	oCanvas.ul.y *= -1;
	PgSetTranslation (&oCanvas.ul, Pg_RELATIVE);
	// unset the clipping area
	PtClipRemove();

	// display filename
	drawText(filePath.toString(), image::Font(12,false,false,"Helvetica"),  Rect(0, 0, 300, 40), image::Color::Green() );
}

/**
 * Set the window title
 *
 * \param title window title
 */
void QnxCanvas::setTitle(const std::string& title)
{
	PtSetResource(m_pWin, Pt_ARG_WINDOW_TITLE, title.c_str(), 0);
}


} // namespace filter
} // namespace precitec

