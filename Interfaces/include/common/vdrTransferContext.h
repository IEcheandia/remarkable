/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2017
 * 	@brief		VdrTransferContext class
 */

#ifndef VDR_TRANSFER_CONTEXT_H_
#define VDR_TRANSFER_CONTEXT_H_

#include <utility>
#include <string>
#include <vector>
#include <map>
#include <list>

#include "InterfacesManifest.h"
#include "graph.h"
#include "Poco/UUID.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"

namespace precitec
{
namespace interface
{
	// VDR transfer context
	class VdrTransferContext : public system::message::Serializable
	{
		public:

			// CTOR, DTOR
			VdrTransferContext() :
				productID_(Poco::UUID::null()),
				instanceProductID_(Poco::UUID::null()),
				seamseriesNr_(0),
				seamNr_(0),
				seamIntevalNr_(0)
			{
			}
		
			VdrTransferContext (Poco::UUID const& productID, Poco::UUID const& instanceProductID, int seamseries, int seam, int seamInterval) :
				productID_(productID),
				instanceProductID_(instanceProductID),
				seamseriesNr_(seamseries),
				seamNr_(seam),
				seamIntevalNr_(seamInterval)
			{
			}

			virtual ~VdrTransferContext() {}



		public:
			const Poco::UUID& 	GetProductID() 			const 	{ return productID_; 		}	//<- DB ID Produkt.
			const Poco::UUID& 	GetInstanceProductID() 	const 	{ return instanceProductID_;}	//<- DB ID InstanzProdukt
			int 				GetSeamseriesNr() 		const 	{ return seamseriesNr_; 	}	//<- Nahtfolge
			int 				GetSeamNr() 			const 	{ return seamNr_; 	}			//<- Naht
			int 				GetSeamIntervalNr() 	const 	{ return seamIntevalNr_; 	}	//<- Nahtbereich

		private:
			Poco::UUID			productID_;
			Poco::UUID			instanceProductID_;
			int 				seamseriesNr_;
			int 				seamNr_;
			int 				seamIntevalNr_;

		public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, productID_);
				marshal(buffer, instanceProductID_);
				marshal(buffer, seamseriesNr_);
				marshal(buffer, seamNr_);
				marshal(buffer, seamIntevalNr_);
			}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, productID_);
				deMarshal(buffer, instanceProductID_);
				deMarshal(buffer, seamseriesNr_);
				deMarshal(buffer, seamNr_);
				deMarshal(buffer, seamIntevalNr_);
			}
	};

}	// precitec
}	// interface


#endif /*VDR_TRANSFER_CONTEXT_H_*/
