/*
 * selectLayerRoi.cpp
 *
 *  Created on: Jun 13, 2013
 *      Author: abeschorner
 */

#include "selectLayerRoi.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

using namespace precitec::interface;
using namespace precitec::geo2d;

namespace filter {

const std::string SelectLayerRoi::m_oFilterName = std::string("SelectLayerRoi");
const std::string SelectLayerRoi::m_oNamePipeXTop = std::string("RoiX");
const std::string SelectLayerRoi::m_oNamePipeYTop = std::string("RoiY");
const std::string SelectLayerRoi::m_oNamePipeXBot = std::string("RoiDx");
const std::string SelectLayerRoi::m_oNamePipeYBot = std::string("RoiDy");

SelectLayerRoi::SelectLayerRoi() : TransformFilter( SelectLayerRoi::m_oFilterName, Poco::UUID{"814D3787-327B-433D-939E-349E2EE1D44E"} ), m_pPipeInImageFrame( nullptr ), m_pPipeROIsIn(nullptr),
		m_oPipeOutXTop(this, SelectLayerRoi::m_oNamePipeXTop ), m_oPipeOutYTop(this, SelectLayerRoi::m_oNamePipeYTop ),
		m_oPipeOutXBot(this, SelectLayerRoi::m_oNamePipeXBot ), m_oPipeOutYBot(this, SelectLayerRoi::m_oNamePipeYBot ),
		m_oTopLayer(true), m_oYTop(0), m_oYBottom(0), m_oRes(interface::AnalysisOK)
{
	parameters_.add("Layer", Parameter::TYPE_int, m_oTopLayer);

    setInPipeConnectors({{Poco::UUID("0742D5C9-3547-4483-97B0-BD9E60466630"), m_pPipeInImageFrame, "ImageFrame", 1, "Image"},
    {Poco::UUID("366D2956-FDE4-49A5-A2D9-1834F33DF6E8"), m_pPipeROIsIn, "RoiLayers", 1, "ROIs"}});
    setOutPipeConnectors({{Poco::UUID("2BD76A65-2979-403C-BA97-1777C67E80DB"), &m_oPipeOutXTop, "RoiX", 0, ""},
    {Poco::UUID("5D0C48D8-1C2B-45B9-80C9-53C2A588530F"), &m_oPipeOutYTop, "RoiY", 0, ""},
    {Poco::UUID("F9A7AE8F-904B-4DDC-A931-A3508A3E19E6"), &m_oPipeOutXBot, "RoiDx", 0, ""},
    {Poco::UUID("2EF758CC-1262-4371-8679-8D20463CFC4F"), &m_oPipeOutYBot, "RoidDy", 0,""}});
    setVariantID(Poco::UUID{"8F4630E5-B430-450D-AE16-E90DFBC36EFF"});
}

SelectLayerRoi::~SelectLayerRoi()
{
}

/// Set filter parameters as defined in database / xml file.
void SelectLayerRoi::setParameter()
{
	m_oTopLayer = parameters_.getParameter("Layer").convert<int>();
}

bool SelectLayerRoi::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "Image")
	{
		m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "ROIs")
	{
		m_pPipeROIsIn = dynamic_cast< fliplib::SynchronePipe< interface::GeoVecDoublearray >* >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void SelectLayerRoi::signalSend(const ImageContext &p_rImgContext, const double p_oWidth, const int p_oIO)
{
	geo2d::Doublearray oTopX, oTopY, oBotX, oBotY;
	oTopX.getData().resize(1); oTopX.getRank().resize(1);
	oTopY.getData().resize(1); oTopY.getRank().resize(1);
	oBotX.getData().resize(1); oBotX.getRank().resize(1);
	oBotY.getData().resize(1); oBotY.getRank().resize(1);

	if (p_oIO == 1)
	{
		ResultType oRT = interface::AnalysisOK; //(m_oTopLayer > 0) ? interface::CalibrationBottomLayer : interface::CalibrationTopLayer;
		oTopX.getData()[0] = 0; oTopY.getData()[0] = m_oYTop;
		oBotX.getData()[0] = p_oWidth; oBotY.getData()[0] = m_oYBottom-m_oYTop;
		oTopX.getRank()[0] = eRankMax; oTopY.getRank()[0] = eRankMax;
		oBotX.getRank()[0] = eRankMax; oBotY.getRank()[0] = eRankMax;

        preSignalAction();
		m_oPipeOutXTop.signal( GeoDoublearray(p_rImgContext, oTopX, oRT, 1.0) ); // set analysis result to either top or bottom layer
		m_oPipeOutYTop.signal( GeoDoublearray(p_rImgContext, oTopY, oRT, 1.0) );
		m_oPipeOutXBot.signal( GeoDoublearray(p_rImgContext, oBotX, oRT, 1.0) );
		m_oPipeOutYBot.signal( GeoDoublearray(p_rImgContext, oBotY, oRT, 1.0) );
	} else
	{
		oTopX.getData()[0] = 0; oTopY.getData()[0] = 0;
		oBotX.getData()[0] = 0; oBotY.getData()[0] = 0;
		oTopX.getRank()[0] = eRankMin; oTopY.getRank()[0] = eRankMin;
		oBotX.getRank()[0] = eRankMin; oBotY.getRank()[0] = eRankMin;
        const auto oRes = m_oRes; // do not access members after preSignalAction()

		preSignalAction();
		m_oPipeOutXTop.signal( GeoDoublearray(p_rImgContext, oTopX, oRes, eRankMin) );
		m_oPipeOutYTop.signal( GeoDoublearray(p_rImgContext, oTopY, oRes, eRankMin) );
		m_oPipeOutXBot.signal( GeoDoublearray(p_rImgContext, oBotX, oRes, eRankMin) );
		m_oPipeOutYBot.signal( GeoDoublearray(p_rImgContext, oBotY, oRes, eRankMin) );
	}
}

void SelectLayerRoi::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)
{
	poco_assert_dbg(m_pPipeROIsIn != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr);

	const GeoVecDoublearray &rROIs( m_pPipeROIsIn->read(m_oCounter) );
	const VecDoublearray &myROIs = rROIs.ref();
	const ImageContext &rContext(rROIs.context());
	m_oSpTrafo = rROIs.context().trafo();
	const ImageFrame &rFrame (m_pPipeInImageFrame->read(m_oCounter));

	// todo test contexts equal
	auto oImg = rFrame.data();
	int oSignalIO = 0;
	double oWidth = oImg.width()*1.0;
	m_oRes = rROIs.analysisResult();

	if ( m_oRes == interface::AnalysisOK )
	{
		auto oRef = myROIs[0];
		if ( (rROIs.rank() != eRankMin) && (oRef.getData().size() >= 4) )
		{
			int oOffset = 2*(int)(m_oTopLayer <= 0); // >0 = toplayer, so bottom layer choice gets offset as data comes int with top layer first
			m_oYTop = oRef.getData()[oOffset]; m_oYBottom = oRef.getData()[1+oOffset];
			oSignalIO = 1;
			signalSend( rContext, oWidth, oSignalIO );
			return;
		}
	} else
	{
		signalSend( rContext, oWidth, oSignalIO );
	}
}

} // namespaces
}

