/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			Reference curves.
 */

#ifndef PRODUCT1DPARAMETER_H_20140522_INCLUDED
#define PRODUCT1DPARAMETER_H_20140522_INCLUDED

#include "common/seam1dParameter.h"

#include "InterfacesManifest.h"
#include "message/serializer.h"

#include "Poco/UUID.h"

#include <map>


namespace precitec {
namespace interface {

/**
 * @brief Reference curves.
 */
	typedef std::vector<Seam1dParameter>		vec_seam1d_parameter_t;
	class INTERFACES_API Product1dParameter : public system::message::Serializable {
public:
	
	/**
	 * CTOR with id.
	 */
	Product1dParameter(const Poco::UUID& p_rProductCurveId = Poco::UUID::null());

	/**
	  * The method is used for serializing an instance.
	  */
	/*virtual*/ void serialize (system::message::MessageBuffer& p_rBuffer) const;

	/**
	  * The method is used for deserializing an instance.
	  */
	/*virtual*/ void deserialize (const system::message::MessageBuffer& p_rBuffer);

	Poco::UUID				m_oProductCurveId;		// id of product curve
	vec_seam1d_parameter_t	m_oSeamCurves;			// vector of seam curves
}; // Product1dParameter


/**
 *	@brief		Inserts data into output stream.
 *	@param		p_rOStream	Out stream to be modified.
 *	@param		p_rData		Input data.
 *	@return		ostream&	Modified stream.
*/
INTERFACES_API std::ostream& operator<<(std::ostream& p_rOStream, const Product1dParameter &p_rData);


typedef std::map<Poco::UUID, interface::Product1dParameter>	id_refcurve_map_t;


} // interface
} // precitec

#endif /* PRODUCT1DPARAMETER_H_20140522_INCLUDED */
