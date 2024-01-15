/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR
 * 	@date		2010
 *	@brief		Overlay Objekte basieren auf den fliplib Objekten. Aufgrund der Importhierarchie werden in WM diese Objekte und nicht die aus fliplib verwendet
 */

#ifndef OVERLAYCANVAS_H_20131210_INCLUDED
#define OVERLAYCANVAS_H_20131210_INCLUDED

#include "InterfacesManifest.h"
#include <memory>

#include "overlay/overlayLayer.h"
#include "overlay/layerType.h"
#include "common/frame.h"
#include "geo/rect.h"
#include "geo/point.h"

#include "overlay/color.h"
#include "overlay/font.h"
#include "message/serializer.h"

#include "fliplib/OverlayCanvas.h"

namespace precitec {
namespace image {
	class OverlayText;

	// Der Canvas stellt eine einfache Leinwand. Sie besteht aus mehreren Layern in die gezeichnet werden
	// kann
	class INTERFACES_API OverlayCanvas : public fliplib::OverlayCanvasInterface, public precitec::system::message::Serializable
	{
	public:
		OverlayCanvas();
		OverlayCanvas(int width, int height);

		// Einfachste Zeichnungsfunktionen welche nur vom Shape verwendet werden
		virtual void drawPixel	(int x, int y, Color c);
		virtual void drawLine 	(int x0, int y0, int x1, int y1, Color c);
		virtual void drawCircle (int x, int y, int r, Color c);
		virtual void drawText   (std::string text, Font font, geo2d::Rect bounds, Color c);
		virtual void drawRect	(geo2d::Rect rectangle, Color c);
		virtual void drawInfoBox(
			ContentType						p_oContentType, 
			int								p_oId, 
			const std::vector<OverlayText>&	p_rLines,
			const geo2d::Rect&				p_rBoundingBox);
		virtual void drawImage(geo2d::Point p_oPosition, const image::BImage& p_rImage, const OverlayText& p_rTitle);
        virtual void drawPixelList(geo2d::Point p_oPosition, const std::vector<int>& y, precitec::image::Color c);
        virtual void drawConnectedPixelList(geo2d::Point p_oPosition, const std::vector<int>& y, precitec::image::Color c);

		virtual void drawFrame(interface::ImageFrame& p_rFrame); // filtertest

		virtual void writePixel(int layer, int x, int y, const Color &c);
		virtual void writeLine(int layer, int x0, int y0, int x1, int y1, const Color &c);
		virtual void writeCircle(int layer, int x, int y, int r, const Color &c);
		virtual void writeText(int layer, const std::string &text, const Font &font, const geo2d::Rect &bounds, const Color &c, int index);
		virtual void writeRect(int layer, const geo2d::Rect &rectangle, const Color &c);
		virtual void writeInfoBox(
			int								p_oLayer, 
			ContentType						p_oContentType, 
			int								p_oId, 
			const std::vector<OverlayText>&	p_rLines,
			const geo2d::Rect&				p_rBoundingBox);
		virtual void writeImage(int layer, const geo2d::Point &p_oPosition, const image::BImage&	p_rImage, const OverlayText& p_rTitle);
		virtual void writeFrame(int layer, interface::ImageFrame &p_rFrame);
        virtual void writePixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c);
        virtual void writeConnectedPixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c);


		virtual void setTitle(const std::string& p_rTitle);

		// Liefert ein Layerobjekt im zOrder. Je groesser der zOrder desto naeher am Betrachter.
		// Layer 0 liegt auf dem Bild
		// enum is not used because it cannot be iterated
		OverlayLayer& getLayer(unsigned int p_oLayer)	{ return layers_[p_oLayer]; }

		const OverlayLayer &getLayer(unsigned int p_oLayer) const { return layers_[p_oLayer]; }

		// Get the layer for a certain group of overlay primitives

		OverlayLayer& getLayerLine()				{ return layers_[eLayerLine]; }
		OverlayLayer& getLayerContour()				{ return layers_[eLayerContour]; }
		OverlayLayer& getLayerPosition()			{ return layers_[eLayerPosition]; }
		OverlayLayer& getLayerText()				{ return layers_[eLayerText]; }
		OverlayLayer& getLayerGridTransp()			{ return layers_[eLayerGridTransp]; }
		OverlayLayer& getLayerLineTransp()			{ return layers_[eLayerLineTransp]; }
		OverlayLayer& getLayerContourTransp()		{ return layers_[eLayerContourTransp]; }
		OverlayLayer& getLayerPositionTransp()		{ return layers_[eLayerContourTransp]; }
		OverlayLayer& getLayerTextTransp()			{ return layers_[eLayerTextTransp]; }
		OverlayLayer& getLayerInfoBox()				{ return layers_[eLayerInfoBox]; }
		OverlayLayer& getLayerImage()				{ return layers_[eLayerImage]; }

		// Veranlasst dass sich saemtliche DrawPrimitiven in den Overlay zeichen
	  	void draw();

		void write();

	  	void clearShapes();

		std::ostream &operator <<(std::ostream &os);
		virtual void setCanvasPalette(image::Color col);

        void swap(OverlayCanvas& p_rOther); // convenience for filter test

	public:
		void serialize ( system::message::MessageBuffer &buffer ) const;

		void deserialize( system::message::MessageBuffer const&buffer );

	protected:
		LayerList layers_;
		/// color for painting within the canvas
		image::Color m_oCol;
	};

} // namespace image
} // namespace precitec

#endif /*OVERLAYCANVAS_H_20131210_INCLUDED*/
