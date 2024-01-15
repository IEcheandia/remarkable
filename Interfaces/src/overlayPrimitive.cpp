/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, HS
 * 	@date		2010
 *	@brief		Einfache Zeichungsprimitve fuer die Overlaytechnik. Die Primitiven sind in erster Linie serialisierbar.
 */

#include "overlay/overlayPrimitive.h"

namespace precitec {
namespace image {

	OverlayPoint::OverlayPoint(system::message::MessageBuffer const&buffer) { deserialize(buffer); }
	OverlayPoint::OverlayPoint(geo2d::Point p, Color const& c) : p_(p), color_(c) {}
	OverlayPoint::OverlayPoint(int x, int y, Color const& c) : p_(x,y), color_(c) {}

	int OverlayPoint::type() const { return ePoint; }

	void OverlayPoint::serialize ( system::message::MessageBuffer &buffer ) const
	{
		marshal(buffer, p_);
		marshal(buffer, color_);
	}

	void OverlayPoint::deserialize( system::message::MessageBuffer const&buffer )
	{
		deMarshal(buffer, p_);
		deMarshal(buffer, color_);
	}

    void OverlayPoint::draw(OverlayCanvas& canvas) { canvas.drawPixel(p_.x, p_.y, color_); }
    void OverlayPoint::write(OverlayCanvas& canvas, int zOrder) { canvas.writePixel(zOrder, p_.x, p_.y, color_); }


	OverlayLine::OverlayLine(system::message::MessageBuffer const&buffer) { deserialize(buffer); }
	OverlayLine::OverlayLine(geo2d::Point from, geo2d::Point to, Color const& c) : p0_(from), p1_(to), color_(c) {}
	OverlayLine::OverlayLine(int x0, int y0, int x1, int y1, Color const& c) : p0_(x0, y0), p1_(x1, y1), color_(c) {}

	int OverlayLine::type() const { return eLine; }

	void OverlayLine::serialize ( system::message::MessageBuffer &buffer ) const
	{
		marshal(buffer, p0_);
		marshal(buffer, p1_);
		marshal(buffer, color_);
	}

	void OverlayLine::deserialize( system::message::MessageBuffer const&buffer )
	{
		deMarshal(buffer, p0_);
		deMarshal(buffer, p1_);
		deMarshal(buffer, color_);
	}

	void OverlayLine::draw(OverlayCanvas& canvas) { canvas.drawLine(p0_.x, p0_.y, p1_.x, p1_.y, color_); }
	void OverlayLine::write(OverlayCanvas& canvas, int zOrder){ canvas.writeLine(zOrder, p0_.x, p0_.y, p1_.x, p1_.y, color_); }


	OverlayCross::OverlayCross(system::message::MessageBuffer const&buffer) { deserialize(buffer); }
	OverlayCross::OverlayCross(geo2d::Point p, int r, Color const& c) : p_(p), r_(r), color_(c) {}
	OverlayCross::OverlayCross(geo2d::Point p, Color const& c) : p_(p), r_(CROSS_SIZE), color_(c) {}
	OverlayCross::OverlayCross(int x, int y, int r, Color const& c) : p_(x,y), r_(r), color_(c) {}
	OverlayCross::OverlayCross(int x, int y, Color const& c) : p_(x,y), r_(CROSS_SIZE), color_(c) {}

	int OverlayCross::type() const { return eCross; }

	void OverlayCross::serialize ( system::message::MessageBuffer &buffer ) const
	{
		marshal(buffer, p_);
		marshal(buffer, r_);
		marshal(buffer, color_);
	}

	void OverlayCross::deserialize( system::message::MessageBuffer const&buffer )
	{
		deMarshal(buffer, p_);
		deMarshal(buffer, r_);
		deMarshal(buffer, color_);
	}

	void OverlayCross::draw(OverlayCanvas& canvas)
	{
		// Horizontale Linie
		canvas.drawLine(p_.x - r_, p_.y, p_.x + r_, p_.y, color_);

		// Vertikale Linie
		canvas.drawLine(p_.x, p_.y - r_, p_.x, p_.y + r_, color_);
	}

	void OverlayCross::write(OverlayCanvas& canvas, int zOrder)
	{
		// Horizontale Linie
		canvas.writeLine(zOrder, p_.x - r_, p_.y, p_.x + r_, p_.y, color_);

		// Vertikale Linie
		canvas.writeLine(zOrder, p_.x, p_.y - r_, p_.x, p_.y + r_, color_);
	}



	OverlayRectangle::OverlayRectangle(system::message::MessageBuffer const&buffer) { deserialize(buffer); }
	OverlayRectangle::OverlayRectangle(geo2d::Rect rect, Color const& c) : rectangle_(rect), color_(c) {}
	OverlayRectangle::OverlayRectangle(int x, int y, int width, int height, Color const& c) : rectangle_(x, y, width, height), color_(c) {}

	int OverlayRectangle::type() const { return eRectangle; }

	void OverlayRectangle::serialize ( system::message::MessageBuffer &buffer ) const
	{
		marshal(buffer, rectangle_);
		marshal(buffer, color_);
	}

	void OverlayRectangle::deserialize( system::message::MessageBuffer const&buffer )
	{
		deMarshal(buffer, rectangle_);
		deMarshal(buffer, color_);
	}

	void OverlayRectangle::draw(OverlayCanvas& canvas) { canvas.drawRect( rectangle_, color_); }

	void OverlayRectangle::write(OverlayCanvas& canvas, int zOrder) { canvas.writeRect(zOrder, rectangle_, color_); }


	OverlayText::OverlayText() : index_(0) { }
	OverlayText::OverlayText(system::message::MessageBuffer const&buffer) { deserialize(buffer); }
	OverlayText::OverlayText(std::string const& text, Font f, geo2d::Rect bounds, Color const& c, int index) : 
		text_(text), font_(f), bounds_(bounds), color_(c), index_(index) { }
	OverlayText::OverlayText(std::string  const& text, Font f, geo2d::Point point, Color const& c, int index) : 
		text_(text), font_(f), bounds_(point.x, point.y, Max<int>::Value, Max<int>::Value), color_(c), index_(index) { }

	int OverlayText::type() const { return eText; }

	void OverlayText::serialize ( system::message::MessageBuffer &buffer ) const
	{
		marshal(buffer, text_);
		marshal(buffer, font_);
		marshal(buffer, bounds_);
		marshal(buffer, color_);
		marshal(buffer, index_);
	}

	void OverlayText::deserialize( system::message::MessageBuffer const&buffer )
	{
		deMarshal(buffer, text_);
		deMarshal(buffer, font_);
		deMarshal(buffer, bounds_);
		deMarshal(buffer, color_);
		deMarshal(buffer, index_);
	}

	void OverlayText::draw(OverlayCanvas& canvas) { canvas.drawText( text_ , font_, bounds_, color_); }

	void OverlayText::write(OverlayCanvas& canvas, int zOrder){ canvas.writeText(zOrder, text_, font_, bounds_, color_, index_); }



	OverlayCircle::OverlayCircle(system::message::MessageBuffer const&buffer) { deserialize(buffer); }
	OverlayCircle::OverlayCircle(geo2d::Point p, int r, Color const& c) : p_(p), r_(r), color_(c) {}
	OverlayCircle::OverlayCircle(int x, int y, int r, Color const& c) : p_(x,y), r_(r), color_(c) {}

	int OverlayCircle::type() const { return eCircle; }

	void OverlayCircle::serialize ( system::message::MessageBuffer &buffer ) const
	{
		marshal(buffer, p_);
		marshal(buffer, r_);
		marshal(buffer, color_);
	}

	void OverlayCircle::deserialize( system::message::MessageBuffer const&buffer )
	{
		deMarshal(buffer, p_);
		deMarshal(buffer, r_);
		deMarshal(buffer, color_);
	}

	void OverlayCircle::draw(OverlayCanvas& canvas)	{	canvas.drawCircle(p_.x, p_.y, r_, color_); }
	void OverlayCircle::write(OverlayCanvas& canvas, int zOrder){ canvas.writeCircle(zOrder, p_.x, p_.y, r_, color_); }



	OverlayInfoBox::OverlayInfoBox(const system::message::MessageBuffer &rBuffer) { deserialize(rBuffer); }
	OverlayInfoBox::OverlayInfoBox(ContentType	p_oContentType, int	p_oId, const std::vector<OverlayText>&	p_rLines, const geo2d::Rect& p_rBoundingBox) 
		: 
		m_oContentType	( p_oContentType ), 
		m_oId			( p_oId ), 
		m_oLines		( p_rLines ), 
		m_oBoundingBox	( p_rBoundingBox )
	{}

    OverlayInfoBox::OverlayInfoBox(ContentType p_oContentType, int p_oId, std::vector<OverlayText> &&p_rLines, const geo2d::Rect &p_rBoundingBox)
        : m_oContentType(p_oContentType)
        , m_oId(p_oId)
        , m_oLines(std::move(p_rLines))
        , m_oBoundingBox(p_rBoundingBox)
    {
    }

	/*virtual*/ int OverlayInfoBox::type() const				{ return eInfoBox; }
	
	int OverlayInfoBox::getId() const							{ return m_oId; }
	ContentType OverlayInfoBox::getContentType() const			{ return m_oContentType; }
    geo2d::Rect OverlayInfoBox::getBoundingBox() const          { return m_oBoundingBox; }
	std::vector<OverlayText> OverlayInfoBox::getLines() const	{ return m_oLines; }


	/*virtual*/ void OverlayInfoBox::serialize (system::message::MessageBuffer &p_rBuffer) const {
		marshal(p_rBuffer, m_oContentType);
		marshal(p_rBuffer, m_oId);
		marshal(p_rBuffer, m_oLines);
		marshal(p_rBuffer, m_oBoundingBox);
	} // serialize

	/*virtual*/ void OverlayInfoBox::deserialize(const system::message::MessageBuffer& p_rBuffer) {
		deMarshal(p_rBuffer, m_oContentType);
		deMarshal(p_rBuffer, m_oId);
		deMarshal(p_rBuffer, m_oLines);
		deMarshal(p_rBuffer, m_oBoundingBox);
	} // deserialize

	/*virtual*/ void OverlayInfoBox::draw(OverlayCanvas& p_rCanvas) { p_rCanvas.drawInfoBox(m_oContentType, m_oId, m_oLines, m_oBoundingBox); }
	/*virtual*/ void OverlayInfoBox::write(OverlayCanvas& p_rCanvas, int zOrder){ p_rCanvas.writeInfoBox(zOrder, m_oContentType, m_oId, m_oLines, m_oBoundingBox); }


	OverlayImage::OverlayImage(const system::message::MessageBuffer &rBuffer) { deserialize(rBuffer); }
	OverlayImage::OverlayImage(geo2d::Point p_oPosition, const image::BImage& p_rImage, const OverlayText& p_rTitle) 
		: 
		m_oPosition	( p_oPosition ), 
		m_oImage	( p_rImage ), 
		m_oTitle	( p_rTitle ) 
	{}

	/*virtual*/ int OverlayImage::type() const { return eImage; }


	/*virtual*/ void OverlayImage::serialize (system::message::MessageBuffer &p_rBuffer) const {
		marshal(p_rBuffer, m_oPosition);
		marshal(p_rBuffer, m_oImage);
		marshal(p_rBuffer, m_oTitle);
	}

	/*virtual*/ void OverlayImage::deserialize(const system::message::MessageBuffer& p_rBuffer) {
		deMarshal(p_rBuffer, m_oPosition);
		deMarshal(p_rBuffer, m_oImage);
		deMarshal(p_rBuffer, m_oTitle);
	}

	/*virtual*/ void OverlayImage::draw(OverlayCanvas& canvas) { canvas.drawImage(m_oPosition, m_oImage, m_oTitle); }
	/*virtual*/ void OverlayImage::write(OverlayCanvas& canvas, int zOrder){ canvas.writeImage(zOrder, m_oPosition, m_oImage, m_oTitle); }

	
	
	
	
	OverlayPointList::OverlayPointList(system::message::MessageBuffer const&buffer) 
    { 
        deserialize(buffer); 
    }

    OverlayPointList::OverlayPointList(geo2d::Point p_oPosition, const std::vector<double>&  p_vec_y, Color const& c, bool connected)
    : m_oPosition(p_oPosition),
    m_yList (p_vec_y.size()), 
    color_(c),
      connected_(connected)
    {
        for (int  i = 0, iMax = p_vec_y.size(); i < iMax; ++i)
        {
            m_yList[i] = int(std::rint(p_vec_y[i]));
        }
    }
    
    OverlayPointList::OverlayPointList(geo2d::Point p_oPosition, const std::vector<int> p_vec_y, Color const& c, bool connected)
    : m_oPosition(p_oPosition), 
    m_yList (std::move(p_vec_y)), 
    color_(c),
      connected_(connected)
    {}

	//OverlayPointList::OverlayPointList(int x, int y, Color const& c) : p_(x,y), color_(c) {}

	int OverlayPointList::type() const { return ePointList; }

	void OverlayPointList::serialize ( system::message::MessageBuffer &buffer ) const
	{
        marshal(buffer, m_oPosition);
		marshal(buffer, m_yList );
		marshal(buffer, color_);
        marshal(buffer, connected_);
	}

	void OverlayPointList::deserialize( system::message::MessageBuffer const&buffer )
	{
		deMarshal(buffer, m_oPosition );
        deMarshal(buffer, m_yList );
        deMarshal(buffer, color_);
        deMarshal(buffer, connected_);
	}

    void OverlayPointList::draw(OverlayCanvas& canvas) 
    { 
        if (connected_)
        {
            canvas.drawConnectedPixelList(m_oPosition, m_yList, color_);
        }
        else
        {
            canvas.drawPixelList(m_oPosition, m_yList, color_);
        }
    }
    void OverlayPointList::write(OverlayCanvas& canvas, int zOrder) 
    { 
        if (connected_)
        {
            canvas.writeConnectedPixelList(zOrder, m_oPosition, m_yList, color_);
        }
        else
        {
            canvas.writePixelList(zOrder, m_oPosition, m_yList, color_);
        }
    }


	

	} // namespace image
} // namespace precitec
