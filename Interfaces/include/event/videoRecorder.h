/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			VideoRecorder Interface
 */

#ifndef EVENT_VIDEORECORDER_H_20120417_INCLUDED
#define EVENT_VIDEORECORDER_H_20120417_INCLUDED


#include "Poco/UUID.h"
#include "message/serializer.h"
#include "common/triggerContext.h"
#include "common/seamData.h"
#include "image/image.h"
#include "system/stdImplementation.h"

#include <cstdint>


namespace precitec {
namespace interface {

typedef Poco::UUID	PocoUUID; ///< for msg macros

/**
 * @ingroup VideoRecorder
 * @brief Aggregates product and seam data for a product instance. Video sequences refer to this data.
 */
class ProductInstData  : public system::message::Serializable {
	public:
		ProductInstData() :
			m_oProductName		("-"),
			m_oProductType		(0),
			m_oProductNr		(0),
			m_oSeamData			(seamDataVector_t(1)) {
		} // ProductInstData

		ProductInstData(
				PvString 					p_oProductName,
				const Poco::UUID& 			p_rProductId,
				const Poco::UUID& 			p_rProductInstId,
				uint32_t 					p_oProductType,
				uint32_t 					p_oProductNr,
                const std::string &extendedProductInfo,
				const seamDataVector_t 		&p_rSeamData) :
			m_oProductName		(p_oProductName),
			m_oProductId		(p_rProductId),
			m_oProductInstId	(p_rProductInstId),
			m_oProductType		(p_oProductType),
			m_oProductNr		(p_oProductNr),
			m_extendedProductInfo{extendedProductInfo},
			m_oSeamData			(p_rSeamData) {
		} // ProductInstData

		PvString					m_oProductName;		///< product name
		Poco::UUID 					m_oProductId;		///< product id
		Poco::UUID 					m_oProductInstId;	///< product instance id
		uint32_t 					m_oProductType;		///< product type
		uint32_t 					m_oProductNr;		///< product number
		std::string m_extendedProductInfo;
		interface::seamDataVector_t m_oSeamData;		///< series and seam data

		/// BASE METHODS

		/**
		 * The method is used for serializing an instance of RequestVideoSequenceResult.
		 */
		virtual void serialize ( MessageBuffer &p_buffer) const {
			marshal(p_buffer, m_oProductName);
			marshal(p_buffer, m_oProductId);
			marshal(p_buffer, m_oProductInstId);
			marshal(p_buffer, m_oProductType);
			marshal(p_buffer, m_oProductNr);
            marshal(p_buffer, m_extendedProductInfo);
            marshal(p_buffer, m_oSeamData.size());
            for (const auto &seamData : m_oSeamData)
            {
                marshal(p_buffer, seamData.m_oSeamSeries);
                marshal(p_buffer, seamData.m_oSeam);
                marshal(p_buffer, seamData.m_oTriggerDelta);
            }
		}
		/**
		 * The method is used for deserializing an instance of RequestVideoSequenceResult.
		 */
		virtual void deserialize (MessageBuffer const &p_buffer) {
			deMarshal(p_buffer, m_oProductName);
			deMarshal(p_buffer, m_oProductId);
			deMarshal(p_buffer, m_oProductInstId);
			deMarshal(p_buffer, m_oProductType);
			deMarshal(p_buffer, m_oProductNr);
            deMarshal(p_buffer, m_extendedProductInfo);
            std::size_t size = 0;
            deMarshal(p_buffer, size);
            m_oSeamData.resize(size);
            for (std::size_t i = 0; i < size; i++)
            {
                auto &seamData = m_oSeamData.at(i);
                deMarshal(p_buffer, seamData.m_oSeamSeries);
                deMarshal(p_buffer, seamData.m_oSeam);
                deMarshal(p_buffer, seamData.m_oTriggerDelta);
            }
		}
}; // ProductInstData


/**
 *	@brief		Inserts data into output stream.
 *	@param		p_rOStream	Out stream to be modified.
 *	@param		p_rData		Input data.
 *	@return		ostream		Modified stream.
*/
inline std::ostream& operator<<( std::ostream& p_rOStream, const ProductInstData &p_rData ) {
	p_rOStream << "<ProductInstData=\n";
	p_rOStream << "\tProductName:\t"	<< p_rData.m_oProductName << '\n';
	p_rOStream << "\tProductId:\t"		<< p_rData.m_oProductId.toString() << '\n';
	p_rOStream << "\tProductInstId:\t"	<< p_rData.m_oProductInstId.toString() << '\n';
	p_rOStream << "\tProductType:\t"	<< p_rData.m_oProductType << '\n';
	p_rOStream << "\tProductNr:\t"		<< p_rData.m_oProductNr << '\n';
	p_rOStream << "\tExtendedProductInfo:\t"		<< p_rData.m_extendedProductInfo << '\n';
	p_rOStream << "\tSeamData:\t"		<< p_rData.m_oSeamData << '\n';
	p_rOStream << "ProductInstData>";
	return p_rOStream;
} // operator<<

} // namespace interface
} // namespace precitec



#endif /*EVENT_VIDEORECORDER_H_20120417_INCLUDED*/
