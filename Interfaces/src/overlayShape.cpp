/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, HS
 * 	@date		2010
 *	@brief		Defines overlay shape.
 */

#include "overlay/overlayShape.h"
#include "overlay/overlayPrimitive.h"
#include "message/messageBuffer.h"

namespace precitec
{
	using namespace system::message;

	namespace image
	{
		/// Dies ist die Factoryklasse zum erzeugen der OverlayPrimitive
		// enum OverlayPrimitive { ePoint, eLine, eCross, eRectangle, eText, eCircle, eNumOverlayPrimitive };
		template <>
		OverlayShape* CreateShape<ePoint>(MessageBuffer const&buffer) {	return new OverlayPoint( buffer ); }
		template <>
		OverlayShape* CreateShape<eLine>(MessageBuffer const&buffer) 	{	return new OverlayLine( buffer );	}
		template <>
		OverlayShape* CreateShape<eCross>(MessageBuffer const&buffer) 	{	return new OverlayCross( buffer );	}
		template <>
		OverlayShape* CreateShape<eRectangle>(MessageBuffer const&buffer) {	return new OverlayRectangle( buffer );	}
		template <>
		OverlayShape* CreateShape<eText>(MessageBuffer const&buffer) 	{	return new OverlayText( buffer );	}
		template <>
		OverlayShape* CreateShape<eCircle>(MessageBuffer const&buffer) 	{	return new OverlayCircle( buffer );	}
		template <>
		OverlayShape* CreateShape<eInfoBox>(MessageBuffer const&buffer) 	{	return new OverlayInfoBox( buffer );	}
		template <>
		OverlayShape* CreateShape<eImage>(MessageBuffer const&buffer) 	{	return new OverlayImage( buffer );	}
		template <>
		OverlayShape* CreateShape<ePointList>(MessageBuffer const&buffer) 	{	return new OverlayPointList( buffer );	}


		typedef OverlayShape* (*OverlayShapeFactory)(MessageBuffer const&buffer);

		OverlayShapeFactory overlayShapeFactoryList[ePointList+1] = {
			CreateShape<ePoint>,
			CreateShape<eLine>,
			CreateShape<eCross>,
			CreateShape<eRectangle>,
			CreateShape<eText>,
			CreateShape<eCircle>,
			CreateShape<eInfoBox>,
			CreateShape<eImage>,
            CreateShape<ePointList>
		};

		OverlayShape* OverlayShape::create(int type, MessageBuffer const&buffer )
		{
			static_assert(sizeof(overlayShapeFactoryList) / sizeof(overlayShapeFactoryList[0]) == eNumOverlayPrimitive, "Factory not complete.");
			return overlayShapeFactoryList[type](buffer);
		}



	} // namespace image
} // namespace precitec
