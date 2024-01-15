/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, AB, HS
 * 	@date		2010
 * 	@brief		Product class.
 */

#ifndef PRODUCT_H_
#define PRODUCT_H_

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
#include "event/results.h"

namespace precitec
{
namespace interface
{
	// sorry for the bad amount of typedefs, but clear types are good at this point, as you will see right now.

	enum TriggerSource 	{ SoftTrigger, ExternalTrigger, GrabControlledTrigger, NumTriggerSource };
	enum TriggerMode	{ SingleTrigger, BurstTrigger, ContinueTrigger, NoneTrigger, NumTriggerMode };

	// Produktdaten
	class INTERFACES_API Product : public Serializable
	{
		public:
			Product () :
				productID_( Poco::UUID::null() ),
				stationID_( Poco::UUID::null() ),
				hwParametersatzID_(Poco::UUID::null()),
				productType_(0),
				endless_(true),
				triggerSource_(SoftTrigger),
				triggerMode_(BurstTrigger),
				name_(""),
				default_(true),
				startPosYAxis_(0),
				m_oRankCounter(0),
				m_lwmTriggerSignal(-1)

			{
			}
			// CTOR, DTOR
			Product (Poco::UUID const& productID, Poco::UUID const& stationID, Poco::UUID const& hwParametersatzID
					, uint32_t productType ,bool endless,  int triggerSource, int triggerMode, std::string name, int startPosYAxis, int lwmTriggerSignal
					,bool defaultProduct=false) :
				productID_(productID),
				stationID_(stationID),
				hwParametersatzID_(hwParametersatzID),
				productType_(productType),
				endless_(endless),
				triggerSource_(triggerSource),
				triggerMode_(triggerMode),
				name_(name),
				default_(defaultProduct),
				startPosYAxis_(startPosYAxis),
				m_oRankCounter(0),
				m_lwmTriggerSignal(lwmTriggerSignal)
			{
			}

			virtual ~Product() {}
			static Product createNil(Poco::UUID const& stationID)    { return Product(Poco::UUID(), stationID, Poco::UUID::null(), 0, false,  SoftTrigger, SingleTrigger, "", -1, 0); }

			/// Sets nio proxy. Necessary to send SumErrors and forward single NIOs. Returns false on error.

			/*
			 * To make things easier, each kind of global error has its own add method here.
			 * ALL errors need the seam-triplet and return a GUID for referencing. The GUID is needed for win to reference
			 * errors (changes to parameters, deletion), whereas the triplet is the scope of a Sum Error.
			 */



		public:
			const Poco::UUID& 	productID() 			const 	{ return productID_; 		}	//<- DB ID Produkt. Entspricht einem Produkt Type.
			const Poco::UUID& 	stationId() 			const 	{ return stationID_; 		}	//<- DB ID Station
			const Poco::UUID& 	hwParameterSatzID() 	const 	{ return hwParametersatzID_;}	//<- DB ID HWParametersatzID
			uint32_t 			productType() 			const 	{ return productType_; 		}	//<- GUID zur produkt ID.
			bool 				endless() 				const 	{ return endless_; 			}	//<- Endlossmessung (Kreis etc)
			int					triggerSource()			const 	{ return triggerSource_;	}	//<- Triggerquelle (Soft Trigger, Externer Trigger)
			int 				triggerMode()			const 	{ return triggerMode_;		}	//<- TriggerModus (Single, Continue, None )
			PvString			name()					const	{ return name_;				}	//<- Name des Produktes
			bool				defaultProduct()		const 	{ return default_;			} 	//<- DefaultProdukt
			bool				isEmpty()       		const   { return productID_.isNull(); } //<- Liefert True, wenn es ein NULL- Produkt ist
			int					startPosYAxis() 		const   { return startPosYAxis_;} 		//<- Liefert die Startposition der Y Achse
            int lwmTriggerSignal() const { return m_lwmTriggerSignal; }                         //<- Returns the signal type, used to trigger the lwm signal inspection


		private:
			Poco::UUID			productID_;
			Poco::UUID			stationID_;
			Poco::UUID			hwParametersatzID_;
			uint32_t			productType_;
			bool				endless_;
			int					triggerSource_;
			int					triggerMode_;
			PvString			name_;
			bool				default_;
			int					startPosYAxis_;
			unsigned int		m_oRankCounter;
            int m_lwmTriggerSignal;

		public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, productID_);
				marshal(buffer, stationID_);
				marshal(buffer, productType_);
				marshal(buffer, endless_);
				marshal(buffer, triggerSource_);
				marshal(buffer, triggerMode_);
				marshal(buffer, name_);
				marshal(buffer, startPosYAxis_);
				marshal(buffer, default_);
				marshal(buffer, hwParametersatzID_);
                marshal(buffer, m_lwmTriggerSignal);
			}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, productID_);
				deMarshal(buffer, stationID_);
				deMarshal(buffer, productType_);
				deMarshal(buffer, endless_);
				deMarshal(buffer, triggerSource_);
				deMarshal(buffer, triggerMode_);
				deMarshal(buffer, name_);
				deMarshal(buffer, startPosYAxis_);
				deMarshal(buffer, default_);
				deMarshal(buffer, hwParametersatzID_);
                deMarshal(buffer, m_lwmTriggerSignal);
			}
	};

	typedef std::vector<Product> ProductList;
	INTERFACES_API bool operator<(const Product& p_rFirst, const Product& p_rSecond);

}	// precitec
}	// interface


#endif /*PRODUCT_H_*/
