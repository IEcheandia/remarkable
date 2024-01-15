/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2016
 * 	@brief		Line profile result filter.
 */


#include "lineProfileResult.h"

#include "fliplib/Parameter.h"
#include "event/results.h"
#include "filter/algoArray.h"
#include "module/moduleLogger.h"
#include "image/image.h"				///< BImage

#include <algorithm>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	using namespace interface;
	using namespace image;
namespace filter {

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

using namespace precitec::interface;
using namespace precitec::geo2d;

// filter name and out pipes' names
const std::string LineProfileResult::m_oFilterName			= std::string("LineProfileResult");
const std::string LineProfileResult::m_oPipeNameResult		= std::string("LineProfile");
const std::string LineProfileResult::m_oResultTypeParameter			= std::string("ResultType");

// standard ctor
LineProfileResult::LineProfileResult() : ResultFilter(LineProfileResult::m_oFilterName, Poco::UUID{"D7A2CD48-8498-41AA-BDD9-761C8BEEFB8F"}),
		m_pPipeLineIn		( nullptr),                                         // IN Pipe
		m_pPipeInImageFrame		( nullptr ),
		m_oPipeResultDouble	( this, LineProfileResult::m_oPipeNameResult ),			// OUT Pipe
		m_oMinMaxRange		( -100000.0, 100000.0 ),
		m_oResultType		(0)
{
	parameters_.add(m_oResultTypeParameter,	Parameter::TYPE_int,	static_cast<int>(m_oResultType));

    setInPipeConnectors({{Poco::UUID("294F7698-BCF1-439D-BA3F-9751AA45B2AA"), m_pPipeLineIn, "lineProfileValIn", 1, "in_line"},
    {Poco::UUID("BB179CD9-8EB6-464B-9E67-064F3BCD2002"), m_pPipeInImageFrame, "imageProfileValIn", 1, "in_image"}});
    setVariantID(Poco::UUID("B36EA376-AC47-4878-BE47-191F40AFC6CF"));
} //ctor



void LineProfileResult::setParameter()
{
	ResultFilter::setParameter();
    m_oResultType			= static_cast<int>(parameters_.getParameter(LineProfileResult::m_oResultTypeParameter).convert<int>());
} // setParameter



bool LineProfileResult::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "in_image" )
	{
		m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);
	}

	if ( p_rPipe.tag() == "in_line" )
	{
		m_pPipeLineIn  = dynamic_cast< SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void LineProfileResult::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	const ImageFrame& rImageFrame = m_pPipeInImageFrame->read(m_oCounter);
	m_oSpTrafo	= rLineIn.context().trafo();
	const ResultType			oResType = rLineIn.analysisResult();


	const Doublearray& rArrayIn = rLineIn.ref().front(); //only use first laserline
	Doublearray outArray(rArrayIn.size(), 0, eRankMin);

	int imageHeight = rImageFrame.data().height();
	int countData = rArrayIn.size();
	for (int i = 0; i < countData; i++)
	{
		outArray.getData()[i] = std::min(imageHeight - rArrayIn.getData()[i], (double)imageHeight);
		outArray.getRank()[i] = rArrayIn.getRank()[i];
	}

	const auto	oGeoValueOut			= GeoDoublearray{ rLineIn.context(),
																	outArray,
																	oResType,
																	rLineIn.rank() };

	auto		oResultDoubleOut		=	ResultDoubleArray	{	id(),
																	(interface::ResultType)(FirstLineProfile + m_oResultType),
																	ValueOutOfLimits,
																	rLineIn.context(),
																	oGeoValueOut,
																	m_oMinMaxRange,
																	false };

	// signal result
	preSignalAction();
	m_oPipeResultDouble.signal( oResultDoubleOut );
} // proceed


} // namespace filter
} // namespace precitec
