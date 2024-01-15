/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			Reference curves.
 */

#ifndef SEAM1DPARAMETER_H_20140522_INCLUDED
#define SEAM1DPARAMETER_H_20140522_INCLUDED

#include "InterfacesManifest.h"

#include "message/serializer.h"

#include "Poco/UUID.h"

// std lib
#include <ostream>
#include <vector>
#include <cstdint>

namespace precitec {
namespace interface {

/**
 *	@brief		Positon value aggregate. Used instead of tuple due to padding control.
**/
#pragma pack(1)
class PosVal { 
public:
	PosVal(int32_t p_oPosition = 0, double p_oValue = 0.)
	: m_oPosition{ p_oPosition }, m_oValue{ p_oValue } 
	{}

	int32_t		m_oPosition;	// position [um]
	double		m_oValue;		// value
}; // class PosVal
#pragma pack()

inline bool operator==(const PosVal& p_rLhs, const PosVal& p_rRhs) { 
	return p_rLhs.m_oPosition == p_rRhs.m_oPosition && p_rLhs.m_oValue == p_rRhs.m_oValue; 
} // operator==

typedef std::vector<PosVal>				vec_pos_val_t;
typedef vec_pos_val_t::const_iterator	cit_pos_val_t;

/**
 * @brief Reference curves.
 */
class INTERFACES_API Seam1dParameter : public system::message::Serializable
{
public:

	Seam1dParameter(const Poco::UUID& p_rSeamCurveId = Poco::UUID::null());
	Seam1dParameter(const int32_t& p_rType, const int32_t& p_rSeam, const int32_t& p_rSeamSeries, const Poco::UUID& p_rSeamCurveId = Poco::UUID::null());


	/**
     * The method is used for serializing an instance.
     **/
	void serialize (system::message::MessageBuffer& p_rBuffer) const;

	/**
	 * The method is used for deserializing an instance.
	 **/
	void deserialize (const system::message::MessageBuffer& p_rBuffer);

    Poco::UUID			m_oSeamCurveId;
	int32_t				m_oType;		// type of curve data
	uint32_t			m_oSeam;		// seam number
	uint32_t			m_oSeamSeries;	// seam series number
	vec_pos_val_t		m_oSeamCurve;	// data array for a seam curve
}; // Seam1dParameter


/**
 *	@brief		Inserts data into output stream.
 *	@param		p_rOStream	Out stream to be modified.
 *	@param		p_rData		Input data.
 *	@return		ostream&	Modified stream.
*/
INTERFACES_API std::ostream& operator<<(std::ostream& p_rOStream, const Seam1dParameter &p_rData);


} // interface
} // precitec

#endif /* SEAM1DPARAMETER_H_20140522_INCLUDED */
