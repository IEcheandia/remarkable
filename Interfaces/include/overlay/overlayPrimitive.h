/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, HS
 * 	@date		2010
 *	@brief		Einfache Zeichungsprimitve fuer die Overlaytechnik. Die Primitiven sind in erster Linie serialisierbar.
 */

#ifndef OVERLAYPRIMITIVE_H_
#define OVERLAYPRIMITIVE_H_

#include "overlay/overlayCanvas.h"
#include "overlay/overlayShape.h"
#include "overlay/color.h"
#include "overlay/font.h"

#include "geo/point.h"
#include "geo/rect.h"

#include "system/typeTraits.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"

#include "InterfacesManifest.h"


namespace precitec
{
namespace image
{
	enum OverlayPrimitive { ePoint, eLine, eCross, eRectangle, eText, eCircle, eInfoBox, eImage, ePointList, eNumOverlayPrimitive };

	// Zeichnet einen Punkt
	class INTERFACES_API OverlayPoint : public OverlayShape
	{
	public:
		OverlayPoint(system::message::MessageBuffer const&buffer);
		OverlayPoint(geo2d::Point p, Color const& c);
		OverlayPoint(int x, int y, Color const& c);
		virtual ~OverlayPoint() {}

		int type() const;

		virtual void serialize ( system::message::MessageBuffer &buffer ) const;

		virtual void deserialize( system::message::MessageBuffer const&buffer );

		protected:
			void draw(OverlayCanvas& canvas);
			void write(OverlayCanvas& canvas, int zOrder);

		private:
			geo2d::Point p_;
			Color color_;

	};

	/**
	 * Primitive fuer eine gerade Linie. Der Startpunkt wird durch p0 und der Endpunkt durch p1 bstimmt
	 */
	class INTERFACES_API OverlayLine : public OverlayShape
	{
	public:
		OverlayLine(system::message::MessageBuffer const&buffer);
		OverlayLine(geo2d::Point from, geo2d::Point to, Color const& c);
		OverlayLine(int x0, int y0, int x1, int y1, Color const& c);
		virtual ~OverlayLine() {}

		int type() const;

		virtual void serialize ( system::message::MessageBuffer &buffer ) const;

		virtual void deserialize( system::message::MessageBuffer const&buffer );


	protected:
		void draw(OverlayCanvas& canvas);
		void write(OverlayCanvas& canvas, int zOrder);

	private:
		geo2d::Point p0_;
		geo2d::Point p1_;
		Color color_;

	};


	/**
	 * Primitive fuer ein gezeichnets Kreuz. Der Mittelpunkt wird durch xy bestimmt,
	 * die Laengen der Seiten durch r
	 */
	class INTERFACES_API OverlayCross : public OverlayShape
	{
		enum { CROSS_SIZE = 10 };

	public:
		OverlayCross(system::message::MessageBuffer const&buffer);
		OverlayCross(geo2d::Point p, int r, Color const& c);
		OverlayCross(geo2d::Point p, Color const& c);
		OverlayCross(int x, int y, int r, Color const& c);
		OverlayCross(int x, int y, Color const& c);

		int type() const;

		virtual void serialize ( system::message::MessageBuffer &buffer ) const;

		virtual void deserialize( system::message::MessageBuffer const&buffer );


	protected:
		void draw(OverlayCanvas& canvas);

		void write(OverlayCanvas& canvas, int zOrder);

	private:
		geo2d::Point p_;
		int r_;
		Color color_;
	};

	/**
	 * Diese Klasse zeichnet ein Rechteck
	 **/
	class INTERFACES_API OverlayRectangle : public OverlayShape
	{

	public:
		OverlayRectangle(system::message::MessageBuffer const&buffer);
		OverlayRectangle(geo2d::Rect rect, Color const& c);
		OverlayRectangle(int x, int y, int width, int height, Color const& c);
		virtual ~OverlayRectangle() {}

		int type() const;

		virtual void serialize ( system::message::MessageBuffer &buffer ) const;

		virtual void deserialize( system::message::MessageBuffer const&buffer );


	protected:
		void draw(OverlayCanvas& canvas);
		void write(OverlayCanvas& canvas, int zOrder);


	private:
		geo2d::Rect rectangle_;
		Color color_;
	};


	// Text auf Canvas
	class INTERFACES_API OverlayText : public OverlayShape
	{

	public:
		OverlayText();
		OverlayText(system::message::MessageBuffer const&buffer);
		OverlayText(std::string const& text, Font f, geo2d::Rect bounds, Color const& c, int index = 0);
		OverlayText(std::string  const& text, Font f, geo2d::Point point, Color const& c, int index = 0);

		int type() const;

		virtual void serialize ( system::message::MessageBuffer &buffer ) const;

		virtual void deserialize( system::message::MessageBuffer const&buffer );

	protected:
		void draw(OverlayCanvas& canvas);
		void write(OverlayCanvas& canvas, int zOrder);

	public:
		std::string		text_;
		Font			font_;
		geo2d::Rect		bounds_;
		Color			color_;
		int				index_;	// 0 by default, needed as sort criterium for lists of OverlayText on C# side
	};

	/**
	 * Primitive fuer ein gezeichnets Kreuz. Der Mittelpunkt wird durch xy bestimmt,
	 * die Laengen der Seiten durch r
	 */
	class INTERFACES_API OverlayCircle: public OverlayShape
	{

	public:
		OverlayCircle(system::message::MessageBuffer const&buffer);
		OverlayCircle(geo2d::Point p, int r, Color const& c);
		OverlayCircle(int x, int y, int r, Color const& c);
		virtual ~OverlayCircle() {}

		int type() const;

		virtual void serialize ( system::message::MessageBuffer &buffer ) const;

		virtual void deserialize( system::message::MessageBuffer const&buffer );


	protected:
		void draw(OverlayCanvas& canvas);
		void write(OverlayCanvas& canvas, int zOrder);

	private:
		geo2d::Point p_;
		int r_;
		Color color_;
	};


/**
 *	@brief		Displays information for a certain location in the main image.
 *	@detail		The information consists of strings. An ID shows the relation to the image location, 
 *				which is further defined by a bounding box to enable on-click actions.
 */
class INTERFACES_API OverlayInfoBox : public OverlayShape {
public:

	OverlayInfoBox(const system::message::MessageBuffer &rBuffer);
	OverlayInfoBox(ContentType	p_oContentType, int	p_oId, const std::vector<OverlayText>&	p_rLines, const geo2d::Rect& p_rBoundingBox);
    /**
     * Constructor which allows to move the vector of OverlayText.
     * Use this variant if a temporary vector is used to prevent a copy of the vector.
     * Example:
     * @code
     * std::vector<OverlayText> textLines;
     * textLines.emplace_back(...);
     * textLines.emplace_back(...);
     * OverlayInfoBox(image::eSurface, 1, std::move(textLines), Rect(0, 0, 10, 20));
     * @endcode
     **/
    OverlayInfoBox(ContentType p_oContentType, int p_oId, std::vector<OverlayText> &&p_rLines, const geo2d::Rect &p_rBoundingBox);

	/*virtual*/ int type() const;
	
	int getId() const;
	ContentType getContentType() const;
    geo2d::Rect getBoundingBox() const;
	
	std::vector<OverlayText> getLines() const;

protected:

	/*virtual*/ void serialize (system::message::MessageBuffer &p_rBuffer) const;

	/*virtual*/ void deserialize(const system::message::MessageBuffer& p_rBuffer);

	/*virtual*/ void draw(OverlayCanvas& p_rCanvas);
	/*virtual*/ void write(OverlayCanvas& p_rCanvas, int zOrder);

private:
	
	ContentType					m_oContentType;		///< The info box wil be displayed depending on the content.
	int							m_oId;				///< An ID shows the relation to the image location.
	std::vector<OverlayText>	m_oLines;			///< Content, eg feature values.
	geo2d::Rect					m_oBoundingBox;		///< A bounding box defines the image location to enable on-click actions
}; // class OverlayInfoBox



/**
 *	@brief	Displays an image with a title.
 */
class INTERFACES_API OverlayImage : public OverlayShape {
public:

	OverlayImage(const system::message::MessageBuffer &rBuffer);
	OverlayImage(geo2d::Point p_oPosition, const image::BImage& p_rImage, const OverlayText& p_rTitle);

	/*virtual*/ int type() const;

protected:

	/*virtual*/ void serialize (system::message::MessageBuffer &p_rBuffer) const;

	/*virtual*/ void deserialize(const system::message::MessageBuffer& p_rBuffer);

	/*virtual*/ void draw(OverlayCanvas& canvas);
	/*virtual*/ void write(OverlayCanvas& canvas, int zOrder);

private:

	geo2d::Point	m_oPosition;	///< Position on canvas
	image::BImage	m_oImage;		///< Image to be displayed.
	OverlayText		m_oTitle;		///< Image title
}; // class OverlayImage

// Displays a list of points
class INTERFACES_API OverlayPointList : public OverlayShape
	{
	public:
		OverlayPointList(system::message::MessageBuffer const&buffer);
        OverlayPointList(geo2d::Point p_oPosition, const std::vector<int>  p_vec_y, Color const& c, bool connected=false); //laser line+ linear trafo
        OverlayPointList(geo2d::Point p_oPosition, const std::vector<double> & p_vec_y, Color const& c, bool connected = false); //laser line+ linear trafo
        
		virtual ~OverlayPointList() {}

		int type() const;

		virtual void serialize ( system::message::MessageBuffer &buffer ) const;

		virtual void deserialize( system::message::MessageBuffer const&buffer );

		protected:
			void draw(OverlayCanvas& canvas);
			void write(OverlayCanvas& canvas, int zOrder);

		private:
            geo2d::Point	m_oPosition;	///< Position on canvas
			std::vector<int> m_yList;
			Color color_;
            bool connected_;

	};

} // namespace interface 
} // namespace precitec

#endif /*OVERLAYPRIMITIVE_H_*/
