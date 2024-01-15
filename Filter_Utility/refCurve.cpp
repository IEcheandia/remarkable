/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2014
 * 	@brief		Provides refernce curve data depending on position.
 */

#include "refCurve.h"
#include "filter/armStates.h"
#include "filter/productData.h"
#include "module/moduleLogger.h"
#include "system/tools.h"
#include <cmath>
#include <fliplib/TypeToDataTypeImpl.h>

#include "Poco/UUID.h"

using namespace fliplib;
using Poco::UUID;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace system;
namespace filter {


RefCurve::RefCurve()
	:
	TransformFilter		{ "RefCurve", Poco::UUID("66926CED-A4A6-43E9-81FA-84EAF795C896") },
	m_pPipeInImageFrame	{ nullptr },
	m_oPipeOutRefValue	{ this, "ReferenceValue" },
	m_oCurveId			( UUID::null().toString() ), // bug in VS 2013 Update 2 and later, cannot compile  { } initializer, see http://connect.microsoft.com/VisualStudio/feedback/details/917150/compiler-error-c2797-on-code-that-previously-worked
	m_oPositionOffset			( 0 ),						 // positionOffset
	m_oValueOffset(0.0),						 // valueOffset
	m_pIdRefCurveMap{ nullptr },
	m_pProdRefCurve		{ nullptr },
	m_pSeamRefCurve		{ nullptr },
	m_pPosValVec		{ nullptr },
	m_oSeamSeries		{ 0 },
	m_oSeam				{ 0 }
{
	// add parameters to parameter list
	parameters_.add("ID", Parameter::TYPE_string, m_oCurveId);
	parameters_.add("Offset", Parameter::TYPE_Int32, m_oPositionOffset);
	parameters_.add("ValueOffset", Parameter::TYPE_double, m_oValueOffset);

    setInPipeConnectors({{Poco::UUID("7619C3FA-5105-48DB-A800-338D93199DB7"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("00F40B4E-2958-42C1-AE25-55394296D7F2"), &m_oPipeOutRefValue, "ReferenceValue", 0, ""}});
    setVariantID(Poco::UUID("D9ECB6E1-49CB-4107-ABDB-9EEA6AACCBBA"));
} // RefCurve



/*virtual*/ void RefCurve::arm (const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamIntervalChange) {
		m_pProdRefCurve		=	nullptr;
		m_pSeamRefCurve		=	nullptr;
		m_pPosValVec		=	nullptr;

		// get product information
		m_pIdRefCurveMap	=	externalData<analyzer::ProductData>()->m_pIdRefCurveMap;
		m_oSeamSeries		=	externalData<analyzer::ProductData>()->m_oSeamSeries;
		m_oSeam				=	externalData<analyzer::ProductData>()->m_oSeam;

		if (m_pIdRefCurveMap == nullptr) {
			wmLog(eWarning, "Filter '%s': No ref curve data available.\n", name().c_str());

			return;
		} // if
		auto	oCurveFound	=	m_pIdRefCurveMap->find(string2Uuid(m_oCurveId));

		if(oCurveFound == std::end(*m_pIdRefCurveMap)) {
			wmLog(eWarning, "Filter '%s': No product reference curve(s) found for id '%s'.\n", name().c_str(),  m_oCurveId.c_str());

			return;
		} // if

		m_pProdRefCurve	=	&oCurveFound->second;
		wmLog(eDebug, "Filter '%s': Product reference curve(s) for id '%s' set.\n", name().c_str(),  m_oCurveId.c_str());

		if (m_pProdRefCurve->m_oSeamCurves.empty()) {
			wmLog(eWarning, "Filter '%s': no reference curves available.\n", name().c_str());
			return;
		} // if

		auto oSeamFound	=	false;
		for(auto& rSeamCurve : m_pProdRefCurve->m_oSeamCurves) {
			if (rSeamCurve.m_oSeamSeries == m_oSeamSeries && rSeamCurve.m_oSeam == m_oSeam) {
				oSeamFound		=	true;
				m_pSeamRefCurve	=	&rSeamCurve;
				m_oItCurrent	=	std::end(m_pSeamRefCurve->m_oSeamCurve);
				m_oItPrev		=	m_oItCurrent;
				break;
			} // if
		} // for

		if (oSeamFound == false) {
			wmLog(eWarning, "Filter '%s': No curve avaialble for seam  series %i, seam %i.\n", name().c_str(), m_oSeamSeries, m_oSeam);

			return;
		} // if

		if (m_pSeamRefCurve->m_oSeamCurve.empty()) {
			wmLog(eWarning, "Filter '%s': Seam curve contains no values.\n", name().c_str());

			return;
		} // if
		m_pPosValVec	=	&m_pSeamRefCurve->m_oSeamCurve;
		m_oItCurrent	=	std::begin(*m_pPosValVec);
		m_oItPrev		=	m_oItCurrent;

		if(m_oVerbosity >= eHigh) {
			wmLog(eDebug, "Filter '%s': Nb seam curves: '%i'.\n", name().c_str(),  m_pProdRefCurve->m_oSeamCurves.size());
			wmLog(eDebug, "Filter '%s':	Seam series: '%i', seam '%i'.\n", name().c_str(),  m_pSeamRefCurve->m_oSeamSeries, m_pSeamRefCurve->m_oSeam);
			wmLog(eDebug, "Filter '%s': Nb pos-value-pairs: '%i'.\n", name().c_str(),  m_pPosValVec->size());
			wmLog(eDebug, "Filter '%s': 1st element: '%i mm', '%f'.\n", name().c_str(),  m_pPosValVec->front().m_oPosition, m_pPosValVec->front().m_oValue);
		} // if

	} // if
	else if (state.getStateID() == eSeamEnd) {
		m_pProdRefCurve		=	nullptr;
		m_pSeamRefCurve		=	nullptr;
		m_pPosValVec		=	nullptr;
	} // if
} // arm



void RefCurve::setParameter() {
	TransformFilter::setParameter();
	m_oCurveId = parameters_.getParameter("ID").convert<std::string>();
	m_oPositionOffset = parameters_.getParameter("Offset").convert<int>();
	m_oValueOffset = parameters_.getParameter("ValueOffset").convert<double>();
} // setParameter



bool RefCurve::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void RefCurve::proceed(const void* sender, fliplib::PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr);

	const auto&		rFrameIn	=	m_pPipeInImageFrame->read(m_oCounter);
	int		oPos		=	rFrameIn.context().position();
	oPos += m_oPositionOffset;

	if (m_pSeamRefCurve == nullptr || m_pSeamRefCurve->m_oSeamCurve.empty()) {
		const auto	oRefValueArray		=	Doublearray		{ 1, 0, filter::eRankMin };	// put parmeter into an array of length 1 with min rank
		const auto	oGeoRefValueArray	=	GeoDoublearray	{ rFrameIn.context(), oRefValueArray, rFrameIn.analysisResult() , Limit };
		preSignalAction(); m_oPipeOutRefValue.signal(oGeoRefValueArray);

		return;
	} // if

	if (oPos < m_oItPrev->m_oPosition) { // simulation case: random position jumps
		m_oItCurrent	=	std::begin(*m_pPosValVec);
		m_oItPrev		=	m_oItCurrent;
	} // if

	auto	oVal	=	m_pPosValVec->back().m_oValue;
	while (m_oItCurrent != std::end(*m_pPosValVec)) {
		if (m_oItCurrent->m_oPosition > oPos) {
			const auto	oDistToPrev	=	std::abs(oPos - m_oItPrev->m_oPosition);
			const auto	oDistToCurr	=	std::abs(oPos - m_oItCurrent->m_oPosition);
			oVal	=	oDistToPrev < oDistToCurr ? m_oItPrev->m_oValue : m_oItCurrent->m_oValue;

			break;
		} // if

		m_oItPrev	=	m_oItCurrent;
		++m_oItCurrent;
	} // while

	// apply value offset:
	oVal = oVal + m_oValueOffset;

	const auto	oRefValueArray		=	Doublearray		{ 1, oVal, filter::eRankMax };	// put parmeter into an array of length 1 with max rank
	const auto	oGeoRefValueArray	=	GeoDoublearray	{ rFrameIn.context(), oRefValueArray, rFrameIn.analysisResult() , Limit };
	preSignalAction(); m_oPipeOutRefValue.signal(oGeoRefValueArray);
} // proceed


} // namespace filter
} // namespace precitec
