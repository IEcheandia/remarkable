/**
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         LB
 *	@date           2020
 */

#include "parameterFilterLine.h"
#include "geo/geo.h"
#include "module/moduleLogger.h"
#include "fliplib/TypeToDataTypeImpl.h"

namespace precitec {
namespace filter {

using interface::GeoDoublearray;
using fliplib::Parameter;


ParameterFilterLine::ParameterFilterLine()
    : fliplib::TransformFilter("ParameterFilterLine", Poco::UUID("964662F3-0A27-4FFA-BA6F-9EA24E534E88"))
	, m_pPipeIn(NULL)
	, m_oPipeOut(this, "Scalar")
	, m_oParam(1.0)
{
    // add parameters to parameter list
	parameters_.add( "scalar",	Parameter::TYPE_double,	static_cast<double>(m_oParam) );
    setInPipeConnectors({{Poco::UUID("AF23076D-0991-4E4B-A4A2-D7EB6BCF7542"), m_pPipeIn, "Data", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("D7F7B20D-B04B-496B-884B-9E0449126225"), &m_oPipeOut, "Scalar", 0, ""}});
    setVariantID(Poco::UUID("572208FE-08B9-4AE0-88A8-53C3FAAC9822"));
}



void ParameterFilterLine::setParameter()
{
    TransformFilter::setParameter();
	m_oParam = parameters_.getParameter("scalar").convert<double>();
} // setParameter



bool ParameterFilterLine::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeIn = dynamic_cast< fliplib::SynchronePipe < interface::GeoVecDoublearray > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void ParameterFilterLine::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
   	poco_assert_dbg( m_pPipeIn != nullptr); // to be asserted by graph editor

    const interface::GeoVecDoublearray &rGeoVecDoubleArrayIn = m_pPipeIn->read(m_oCounter);
    // put parameter into an array of the same length as the input, with max rank
    geo2d::Doublearray output(rGeoVecDoubleArrayIn.ref().size(), m_oParam, filter::eRankMax);
    const auto oGeoOut = interface::GeoDoublearray(rGeoVecDoubleArrayIn.context(), output, rGeoVecDoubleArrayIn.analysisResult(), interface::Perfect);
    preSignalAction();
    m_oPipeOut.signal(oGeoOut);

} // proceed


} // namespace inspect
} // namespace precitec
