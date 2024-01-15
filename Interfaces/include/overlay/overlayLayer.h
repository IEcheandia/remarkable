/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR
 * 	@date		2010
 *	@brief		Defines overlay layer.
 */

#ifndef OVERLAYLAYER_H_20140113_INCLUDED
#define OVERLAYLAYER_H_20140113_INCLUDED

#include "InterfacesManifest.h"

#include "message/serializer.h"
#include "overlay/overlayShape.h"

#include <vector>

namespace precitec {
namespace image {

	enum ContentType { ePore, ePoreSet1, ePoreSet2, ePoreSet3, ePoorPenetration, eSurface, ePoorPenetrationHough, eContentTypeMin = ePore, eContentTypeMax = ePoorPenetrationHough };

	// Ein Layer besitzt eine Liste mit Zeichunungsobjekten (Shape)
	class INTERFACES_API OverlayLayer : public system::message::Serializable
	{

	public:
		void add(SpOverlayShape shape);
        template <typename T>
        void add(T* shape)
        {
            add(SpOverlayShape(shape));
        }
        /**
         * Overload which adds with zero temporary copies if the overlay needs to be constructed as a new object.
         * Instead of passing the pointer to an object, the arguments for the constructor are passed in.
         * So instead of
         * @code
         * layer.add(new OverlayPoint(Point(1, 2), Color::Red()));
         * @endcode
         *
         * Use:
         * @code
         * layer.add<OverlayPoint>(Point(1, 2), Color::Red());
         * @endcode
         **/
        template <typename T, typename... Args>
        void add(Args&&... args)
        {
            // arguments are forwarded to make_shared so that the shared pointer does not
            // need to be copied in. By using emplace_back also the insert into the vector is without copy
            shapes_.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
        }

		void serialize ( system::message::MessageBuffer &buffer ) const;
		void deserialize( system::message::MessageBuffer const&buffer );

        /**
         * @returns whether there are any shapes in this layer
         **/
        bool hasShapes() const;

        ShapeList info() const;

	private:
		friend class OverlayCanvas;

		void draw(OverlayCanvas& canvas);

		void write(OverlayCanvas& canvas, int zOrder);

		void clearShapeList();

	protected:
		ShapeList shapes_;
	};

	typedef std::vector<OverlayLayer> LayerList;

} // namespace image
} // namespace precitec

#endif /*OVERLAYLAYER_H_20140113_INCLUDED*/
