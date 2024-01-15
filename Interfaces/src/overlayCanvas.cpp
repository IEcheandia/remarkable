/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR
 * 	@date		2010
 *	@brief		Overlay Objekte basieren auf den fliplib Objekten. Aufgrund der Importhierarchie werden in WM diese Objekte und nicht die aus fliplib verwendet
 */

#include "overlay/overlayCanvas.h"

#include <string>
#include <vector>

#include "message/messageBuffer.h"

namespace precitec {
namespace image {
		OverlayCanvas::OverlayCanvas() : OverlayCanvasInterface(1024, 1024), layers_(eNbLayers), m_oCol(image::Color::White()) {}
		OverlayCanvas::OverlayCanvas(int width, int height) : OverlayCanvasInterface(width, height) , layers_(eNbLayers), m_oCol(image::Color::White()) {}

		// Einfachste Zeichnungsfunktionen welche nur vom Shape verwendet werden
		void OverlayCanvas::drawPixel	(int x, int y, Color c)
			{ std::cout << "drawPixel (x,y,c):" << x << "," << y << "," << std::hex << c.toARGB() << std::dec << std::endl; }
		void OverlayCanvas::drawLine 	(int x0, int y0, int x1, int y1, Color c)
			{ std::cout << "drawLine (x0,y0, x1, y1,c):" << x0 << "," << y0 << "," << x1 << "," << y1<< "," << std::hex << c.toARGB() << std::dec << std::endl; }
		void OverlayCanvas::drawCircle (int x, int y, int r, Color c)
			{ std::cout << "drawCircle (x,y,c):" << x << "," << y << "," << std::hex << c.toARGB() << std::dec << std::endl; }
		void OverlayCanvas::drawText   (std::string text, Font font, geo2d::Rect bounds, Color c)
			{ std::cout << "drawText (text, font, bounds, Color): '" << text << "'," << font.name << "," << bounds << "," << std::hex << c.toARGB() << std::dec << std::endl; }
		void OverlayCanvas::drawRect	(geo2d::Rect rectangle, Color c)
			{ std::cout << "drawRect (Rect,c):" << rectangle << "," << std::hex  << c.toARGB() << std::dec << std::endl; }
		void OverlayCanvas::drawInfoBox(
			ContentType						p_oContentType, 
			int								p_oId, 
			const std::vector<OverlayText>&	p_rLines,
			const geo2d::Rect&				p_rBoundingBox)
		{
			std::cout << "drawInfoBox(p_rId): " << p_oId << std::endl;
		}
		void OverlayCanvas::drawImage(geo2d::Point p_oPosition, const image::BImage& p_rImage, const OverlayText& p_rTitle)
		{
			std::cout << "drawImage(p_oPosition, p_rImage.size()): " << p_oPosition << ", " << p_rImage.size() << std::endl;
		}
		
		void OverlayCanvas::drawPixelList(geo2d::Point p_oPosition, const std::vector<int> & y_list, image::Color c)
		{
			std::cout << "drawPixelList(p_oPosition, y_list.size()) : " << p_oPosition << ", " << y_list.size() << std::endl;
		}

		void OverlayCanvas::drawConnectedPixelList(geo2d::Point p_oPosition, const std::vector<int> & y_list, image::Color c)
		{
			std::cout << "drawConnectedPixelList(p_oPosition, y_list.size()) : " << p_oPosition << ", " << y_list.size() << std::endl;
		}
		
		void OverlayCanvas::drawFrame(interface::ImageFrame& p_rFrame) // filtertest
		{
			std::cout << "drawFrame(imageFrame, Color): " << p_rFrame.context() << std::endl;
		}

		void OverlayCanvas::writePixel(int layer, int x, int y, const Color &c)
		{
			
		}
		void OverlayCanvas::writeLine(int layer, int x0, int y0, int x1, int y1, const Color &c)
		{
			
		}
		void OverlayCanvas::writeCircle(int layer, int x, int y, int r, const Color &c)
		{
			
		}
		void OverlayCanvas::writeText(int layer, const std::string &text, const Font &font, const geo2d::Rect &bounds, const Color &c, int index)
		{
			
		}
		void OverlayCanvas::writeRect(int layer, const geo2d::Rect &rectangle, const Color &c)
		{
		
		}
		void OverlayCanvas::writeInfoBox(
			int								p_oLayer, 
			ContentType						p_oContentType, 
			int								p_oId, 
			const std::vector<OverlayText>&	p_rLines,
			const geo2d::Rect&				p_rBoundingBox)

		{

		}
		void OverlayCanvas::writeImage(int layer, const geo2d::Point &p_oPosition, const image::BImage&	p_rImage, const OverlayText& p_rTitle)
		{

		}
		void OverlayCanvas::writeFrame(int layer, interface::ImageFrame &p_rFrame)
		{
			
		}

        void OverlayCanvas::writePixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c)
        {

        }

        void OverlayCanvas::writeConnectedPixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c)
        {

        }

		void OverlayCanvas::setTitle(const std::string& p_rTitle)
		{
			std::cout << "setTitle(p_rTitle): " << p_rTitle << std::endl;
		}

		// Veranlasst dass sich saemtliche DrawPrimitiven in den Overlay zeichen
	  	void OverlayCanvas::draw()
	  	{
	  		for(LayerList::iterator it=layers_.begin();it!=layers_.end();++it)
	  			(it)->draw(*this);
	  	}

		void OverlayCanvas::write()
		{
			for (unsigned int zOrder = eLayerMin; zOrder < eNbLayers; zOrder++){
				auto& rLayer = getLayer(zOrder);

				rLayer.write(*this, (int) zOrder);
			}
			
				
		}

	  	void OverlayCanvas::clearShapes()
	  	{
	  		for(LayerList::iterator it=layers_.begin();it!=layers_.end();++it)
				(it)->clearShapeList();
	  	}

		std::ostream& OverlayCanvas::operator <<(std::ostream &os) {
			os << "OverlayCanvas: " << layers_.size(); return os;
		}

		void OverlayCanvas::setCanvasPalette(image::Color col)
		{
			m_oCol = col;
		};

        void OverlayCanvas::swap(OverlayCanvas& p_rOther)
        {
            std::swap(width_, p_rOther.width_);
            std::swap(height_, p_rOther.height_);
            std::swap(layers_, p_rOther.layers_);
            std::swap(m_oCol, p_rOther.m_oCol);
        }

		void OverlayCanvas::serialize ( system::message::MessageBuffer &buffer ) const
		{
			marshal(buffer, width_);
			marshal(buffer, height_);
			marshal(buffer, layers_, layers_.size());
		}

		void OverlayCanvas::deserialize( system::message::MessageBuffer const&buffer )
		{
			deMarshal(buffer, width_);
			deMarshal(buffer, height_);
			deMarshal(buffer, layers_, 0);
		}

} // namespace image
} // namespace precitec
