/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			Reference curves.
 */

#include "common/seam1dParameter.h"

#include <iostream>


namespace precitec {
	using namespace system::message;
namespace interface {
	
Seam1dParameter::Seam1dParameter(const Poco::UUID& p_rSeamCurveId)
	:	m_oSeamCurveId (p_rSeamCurveId)
    ,	m_oType{0}
    ,	m_oSeam{0}
    ,	m_oSeamSeries{0}
{} // Seam1dParameter

Seam1dParameter::Seam1dParameter(const int32_t& p_rType, const int32_t& p_rSeam, const int32_t& p_rSeamSeries, const Poco::UUID& p_rSeamCurveId)
    :	m_oSeamCurveId (p_rSeamCurveId)
    ,   m_oType(p_rType)
    ,   m_oSeam(p_rSeam)
    ,   m_oSeamSeries(p_rSeamSeries)
{
}

void Seam1dParameter::serialize (MessageBuffer& p_rBuffer) const {
    marshal(p_rBuffer, m_oSeamCurveId);
	marshal(p_rBuffer, m_oType);
	marshal(p_rBuffer, m_oSeam);
	marshal(p_rBuffer, m_oSeamSeries);
	marshal(p_rBuffer, m_oSeamCurve);
} // serialize



void Seam1dParameter::deserialize (const MessageBuffer& p_rBuffer) {
    deMarshal(p_rBuffer, m_oSeamCurveId);
	deMarshal(p_rBuffer, m_oType);
	deMarshal(p_rBuffer, m_oSeam);
	deMarshal(p_rBuffer, m_oSeamSeries);
	deMarshal(p_rBuffer, m_oSeamCurve);
} // deserialize



std::ostream& operator<<(std::ostream& p_rOStream, const Seam1dParameter &p_rData) {
	p_rOStream << "<Seam1dParameter=\n";

	p_rOStream << "\tType:\t\t" 			<< p_rData.m_oType << '\n';
	p_rOStream << "\tSeam:\t\t" 			<< p_rData.m_oSeam << '\n';
	p_rOStream << "\tSeam series:\t" 		<< p_rData.m_oSeamSeries << '\n';
	p_rOStream << "\tSeam curve size:\t" 	<< p_rData.m_oSeamCurve.size() << '\n';
	for (auto& rPosVal : p_rData.m_oSeamCurve) {
		p_rOStream << "\t\tpos\t" << std::fixed << rPosVal.m_oPosition << "\tval\t" << rPosVal.m_oValue << '\n';
	} // for
	p_rOStream << "Seam1dParameter>";
	return p_rOStream;
} // operator<<

} // namespace interface
} // namespace precitec


