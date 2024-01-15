/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR
 * 	@date		2010
 *	@brief		Defines overlay layer.
 */

#include "overlay/overlayLayer.h"

#include "message/messageBuffer.h"

#include "overlay/overlayPrimitive.h"

namespace precitec {
namespace image {

void OverlayLayer::add(SpOverlayShape shape) { shapes_.push_back( shape ); }

void OverlayLayer::serialize ( system::message::MessageBuffer &buffer ) const
{
    marshal(buffer, shapes_, shapes_.size());
}

void OverlayLayer::deserialize( system::message::MessageBuffer const&buffer )
{
    deMarshal(buffer, shapes_, 0);
}

void OverlayLayer::draw(OverlayCanvas& canvas)
{
    for(ShapeList::iterator it= shapes_.begin();it!=shapes_.end();++it)
        (*it)->drawShape(canvas);
}

void OverlayLayer::write(OverlayCanvas& canvas, int zOrder)
{
    for (ShapeList::iterator it = shapes_.begin(); it != shapes_.end(); ++it)
        (*it)->writeShape(canvas, zOrder);
}

void OverlayLayer::clearShapeList() {
    shapes_.clear();
}

ShapeList OverlayLayer::info() const
{
    ShapeList infoShapes;

    std::copy_if(shapes_.begin(), shapes_.end(), std::back_inserter(infoShapes), [](auto s)
    {
        return s->type() == OverlayPrimitive::eInfoBox;
    });

    return infoShapes;
}

bool OverlayLayer::hasShapes() const
{
    return !shapes_.empty();
}

} // namespace image
} // namespace precitec


