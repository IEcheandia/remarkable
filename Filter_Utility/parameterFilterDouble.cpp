/**
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           25.10.2012
 */

#include "parameterFilterDouble.h"
#include "geo/geo.h"
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

using interface::GeoDoublearray;
using fliplib::Parameter;


ParameterFilterDouble::ParameterFilterDouble()
    : fliplib::TransformFilter("ParameterFilterDouble", Poco::UUID{"1e926bbc-e386-41fc-89ef-89ef6b09d29d"})
	, m_pPipeIn(NULL)
	, m_oPipeOut(this, "Scalar")
	, m_oParam(1.0)
{
    // add parameters to parameter list
	parameters_.add( "scalar",	Parameter::TYPE_double,	static_cast<double>(m_oParam) );
    setInPipeConnectors({{Poco::UUID("e07e0ff6-d574-418f-9064-ad7d4abeb93a"), m_pPipeIn, "Data", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("ed82eabf-b65c-4691-b7b3-100403d7a143"), &m_oPipeOut, "Scalar", 0, ""}});
    setVariantID(Poco::UUID("ae5e91b1-a593-4193-b9d5-72322e8a1eba"));
}



void ParameterFilterDouble::setParameter()
{
    TransformFilter::setParameter();
	m_oParam = parameters_.getParameter("scalar").convert<double>();
} // setParameter



bool ParameterFilterDouble::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeIn = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void ParameterFilterDouble::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
   	poco_assert_dbg( m_pPipeIn != nullptr); // to be asserted by graph editor

    const interface::GeoDoublearray &rGeoDoubleArrayIn = m_pPipeIn->read(m_oCounter);
    // put parmeter into an array of length 1 with max rank
    geo2d::Doublearray output(1, m_oParam, filter::eRankMax);
    const auto oGeoOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(), output, rGeoDoubleArrayIn.analysisResult(), interface::Perfect);
    preSignalAction(); m_oPipeOut.signal(oGeoOut);

} // proceed


} // namespace inspect
} // namespace precitec
