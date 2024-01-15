/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		DUW
 * 	@date		2015
 * 	@brief		This filter may discard images or select them for processing given a delay.
 */

// todo: add enum for parameter

// project includes
#include "selectImage.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string SelectImage::m_oFilterName( "SelectImage" );
const std::string SelectImage::m_oPipeOutName("ImageFrame");				///< Pipe: Data out-pipe. Use the standard name instead of any custom name (e.g. SelectedImage)
const std::string SelectImage::m_oParamSelectThresholdName("SelectThreshold");			///< Parameter: Amount of delay [um].
const std::string SelectImage::m_oParamEndThresholdName("EndThreshold");			///< Parameter: Amount of delay [um].


SelectImage::SelectImage() :
	TransformFilter			( SelectImage::m_oFilterName, Poco::UUID{"E27A227E-2214-4EA9-A37F-6F19244719D0"} ),
	m_pPipeInImageFrame(nullptr),
	m_oPipeOutImageFrame(this, SelectImage::m_oPipeOutName),
	m_oSelectThreshold				( 0 ),
	m_oTriggerDelta			( 0 ),
	m_oEndThreshold(10000000)
{
	parameters_.add( m_oParamSelectThresholdName, fliplib::Parameter::TYPE_uint, m_oSelectThreshold );
	parameters_.add(m_oParamEndThresholdName, fliplib::Parameter::TYPE_uint, m_oEndThreshold);

    setInPipeConnectors({{Poco::UUID("AA836A30-E634-4FB3-89C8-83FA5F14B827"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("BD20C248-4AE7-45B1-98E0-820812C47CCF"), &m_oPipeOutImageFrame, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("CBC706BC-CFFA-4D53-8357-3FB6656B6D40"));
} // CTor



/*virtual*/ SelectImage::~SelectImage()
{

} // DTor



void SelectImage::setParameter()
{
	TransformFilter::setParameter();

	m_oSelectThreshold = parameters_.getParameter(SelectImage::m_oParamSelectThresholdName).convert<unsigned int>();
	m_oEndThreshold = parameters_.getParameter(SelectImage::m_oParamEndThresholdName).convert<unsigned int>();
} // setParameter.



/*virtual*/ void SelectImage::arm(const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
		// get product information
		const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
		// get trigger delta
		m_oTriggerDelta = pProductData->m_oTriggerDelta;
	} // if

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());
	} // if

} // arm



bool SelectImage::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < interface::ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void SelectImage::proceed(const void* p_pSender, fliplib::PipeEventArgs& e)
{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// data
	const interface::ImageFrame &rFrame = m_pPipeInImageFrame->read(m_oCounter);
	const interface::ImageContext& rContext = rFrame.context();
	long oPosition = rContext.position();

	// In case this image should be processed, then do that.
	if ((oPosition >= (int)m_oSelectThreshold) && (oPosition <= (int)m_oEndThreshold) )    // "larger or equal" is necessary in case StartThreshold == 0
	{
		preSignalAction(); m_oPipeOutImageFrame.signal(interface::ImageFrame(rFrame.context(), rFrame.data(), rFrame.analysisResult()));
	}
	else
    {
        preSignalAction();
    }

	if(m_oVerbosity >= eHigh)
	{
		wmLog( eDebug, "SelectImage::proceed.\n" );
	}
} // proceed


} // namespace filter
} // namespace precitec
