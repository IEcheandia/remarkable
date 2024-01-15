/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			Provides and holds reference curves.
 */

#include "analyzer/referenceCurves.h"
#include "module/moduleLogger.h"
#include "system/tools.h"

using Poco::UUID;
namespace precitec {
	using namespace interface;
	using namespace system;
namespace analyzer {

/*static*/ const std::string	ReferenceCurves::g_oNameRefCurveFilterWithNs	=	"precitec::filter::RefCurve";	// see refCurve.cpp
/*static*/ const std::string	ReferenceCurves::g_oNameRefCurveIdParameter		=	"ID";							// see refCurve.cpp

ReferenceCurves::ReferenceCurves(if_tdb_t* p_pDbProxy) 
	:
	m_pDbProxy				{ p_pDbProxy }
{} // ReferenceCurves



void ReferenceCurves::requestAndStoreRefCurves(const ParameterList& p_rParameterList) {		
	const auto oIds			= getRefCurveIds(p_rParameterList);
	requestAndStoreRefCurves(oIds);
} // requestAndStoreRefCurves



const id_refcurve_map_t* ReferenceCurves::getRefCurveMap() const {
	return &m_oIdRefCurveMap;
} // getRefCurveMap



std::vector<UUID> ReferenceCurves::getRefCurveIds(const ParameterList& p_rParameterList) const {
	auto	oRefCurveIds	=	std::vector<UUID>{};

	for(const auto& rParameter : p_rParameterList) {
		if (rParameter->name() == g_oNameRefCurveIdParameter) {
			oRefCurveIds.emplace_back(string2Uuid(rParameter->value<std::string>()));
		} // if
	} // for

	return oRefCurveIds;
} // getRefCurveIds



void ReferenceCurves::requestAndStoreRefCurves(const std::vector<UUID>& p_rRefCurveIds) {
	for(const auto& rRefCurveId : p_rRefCurveIds) {
		if (rRefCurveId.isNull()) {
			continue;
		} // if

		m_oIdRefCurveMap.erase(rRefCurveId);
		m_oIdRefCurveMap.emplace(std::make_pair(rRefCurveId, m_pDbProxy->getEinsDParameter(rRefCurveId)));

#if !defined(NDEBUG)
		wmLog(eDebug, "%s - stored 1d-parameter with id '%s'.\n", __FUNCTION__, rRefCurveId.toString().c_str());
		//std::cout << oProduct1dParam << "\n";
#endif // !defined(NDEBUG)
	} // for
} // requestAndStoreRefCurves


} // namespace analyzer
} // namespace precitec

