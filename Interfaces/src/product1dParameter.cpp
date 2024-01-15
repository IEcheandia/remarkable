/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			Reference curves.
 */

#include "common/product1dParameter.h"


namespace precitec {
	using namespace system::message;
namespace interface {
Product1dParameter::Product1dParameter(const Poco::UUID& p_rProductCurveId )
	:
	m_oProductCurveId	( p_rProductCurveId )
{}



/*virtual*/ void Product1dParameter::serialize (MessageBuffer& p_rBuffer) const {
	marshal(p_rBuffer, m_oProductCurveId);
	marshal(p_rBuffer, m_oSeamCurves);
} // serialize



/*virtual*/ void Product1dParameter::deserialize (const MessageBuffer& p_rBuffer) {
	deMarshal(p_rBuffer, m_oProductCurveId);
	deMarshal(p_rBuffer, m_oSeamCurves);
} // deserialize



std::ostream& operator<<(std::ostream& p_rOStream, const Product1dParameter &p_rData) {
	p_rOStream << "<Product1dParameter=\n";

	p_rOStream << "\tmProductCurveId:\t\t" 	<< p_rData.m_oProductCurveId.toString() << '\n';
	p_rOStream << "\tSeam curves size:\t" 	<< p_rData.m_oSeamCurves.size() << '\n';
	for (auto& rSeamCurve : p_rData.m_oSeamCurves) {
		p_rOStream << rSeamCurve << '\n';
	} // for
	p_rOStream << "Product1dParameter>";
	return p_rOStream;
} // operator<<

} // namespace interface
} // namespace precitec


