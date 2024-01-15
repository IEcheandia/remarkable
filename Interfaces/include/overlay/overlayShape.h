/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, HS
 * 	@date		2010
 *	@brief		Defines overlay shape.
 */

#ifndef OVERLAYSHAPE_H_20131210_INCLUDED
#define OVERLAYSHAPE_H_20131210_INCLUDED

#include "InterfacesManifest.h"

#include "message/serializer.h"

#include "Poco/SharedPtr.h"

#include <vector>


namespace precitec {
namespace image {
	class OverlayCanvas;

	// Basisobjekt aller OverlayPrimitive
	class INTERFACES_API OverlayShape : public precitec::system::message::Serializable
	{
	public:
		OverlayShape() {};
		virtual ~OverlayShape() {};

		void drawShape(OverlayCanvas& canvas) { draw ( canvas ); }
		void writeShape(OverlayCanvas& canvas, int zOrder) { write(canvas, zOrder); }
		virtual int type() const = 0;

		static OverlayShape* create(int type, system::message::MessageBuffer const& buffer );	// Factoryobjekt in overlay.cpp implementiert

	protected:
		virtual void draw(OverlayCanvas& canvas) = 0;
		virtual void write(OverlayCanvas& canvas, int zOrder) = 0;
	};

	typedef std::shared_ptr<OverlayShape> SpOverlayShape;
	typedef std::vector<SpOverlayShape> ShapeList;

	template <int Type>
	OverlayShape* CreateShape(system::message::MessageBuffer const&buffer);

} // namespace image
} // namespace precitec

#endif /*OVERLAYSHAPE_H_20131210_INCLUDED*/
