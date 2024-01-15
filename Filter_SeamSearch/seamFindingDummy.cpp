/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2015
* 	@brief 		This filter extracs the width of the laserline, so seams can be found in thin line areas
*/

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>

// local includes
#include "seamFindingDummy.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string SeamFindingDummy::m_oFilterName = std::string("SeamFindingDummy");
		const std::string SeamFindingDummy::PIPENAME_OUT = std::string("SeamFindingOut");

		SeamFindingDummy::SeamFindingDummy() :
			TransformFilter(SeamFindingDummy::m_oFilterName, Poco::UUID{"D02FE58E-F5C9-4E32-9EE4-6460D2156053"}),
			m_pPipeInImageFrame(NULL),
			m_pPipeOutSeamFinding(NULL),
			m_oSeamLeft(0),
			m_oSeamRight(0),
			m_oAlgoType(0)
		{
			m_pPipeOutSeamFinding = new SynchronePipe< GeoSeamFindingarray >(this, SeamFindingDummy::PIPENAME_OUT);

			// Set default values of the parameters of the filter
			parameters_.add("SeamLeft", Parameter::TYPE_int, m_oSeamLeft);
			parameters_.add("SeamRight", Parameter::TYPE_int, m_oSeamRight);
			parameters_.add("AlgoTyp", Parameter::TYPE_int, m_oAlgoType);
			parameters_.add("Quality", Parameter::TYPE_int, m_oQuality);

            setInPipeConnectors({{Poco::UUID("566CAAA8-4913-40B8-AC11-AAC41746DE16"), m_pPipeInImageFrame, "ImageFrameIn", 0, "ImageFrame"}});
            setOutPipeConnectors({{Poco::UUID("8B7CCFD0-8461-4AFD-B85D-5E21511AA826"), m_pPipeOutSeamFinding, "SeamFindingOut", 0, ""}});
            setVariantID(Poco::UUID("C2726C3F-7C52-460B-B805-F8D78AAFCA3B"));
		}

		SeamFindingDummy::~SeamFindingDummy()
		{
			delete m_pPipeOutSeamFinding;
		} // ~LineProfile

		void SeamFindingDummy::setParameter()
		{
			TransformFilter::setParameter();
			m_oSeamLeft = parameters_.getParameter("SeamLeft").convert<int>();
			m_oSeamRight = parameters_.getParameter("SeamRight").convert<int>();
			m_oAlgoType = parameters_.getParameter("AlgoTyp").convert<int>();
			m_oQuality = parameters_.getParameter("Quality").convert<int>();
		} // setParameter

		bool SeamFindingDummy::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void SeamFindingDummy::paint()
		{
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
			{
				return;
			} // if

		} // paint


		void SeamFindingDummy::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

			geo2d::SeamFindingarray outArray;
			//outArray[0].getData().

			SeamFinding seamFinding;
			seamFinding.m_oSeamLeft = m_oSeamLeft;
			seamFinding.m_oSeamRight = m_oSeamRight;
			seamFinding.m_oAlgoType = m_oAlgoType;
			seamFinding.m_oQuality = m_oQuality;

			outArray.getData().push_back(seamFinding);
			outArray.getRank().push_back(255);

			// Read out image frame from pipe
			const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
			m_oSpTrafo = rFrameIn.context().trafo();
			// Extract actual image and size
			//const BImage &rImageIn = rFrameIn.data();
			m_oSpTrafo = rFrameIn.context().trafo();

			// Create a new byte array, and put the global context into the resulting profile
			const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
			const GeoSeamFindingarray &rGeoSFA = GeoSeamFindingarray(rFrameIn.context(), outArray, oAnalysisResult, filter::eRankMax);
			preSignalAction();
			m_pPipeOutSeamFinding->signal(rGeoSFA);

		} // proceedGroup
	} // namespace precitec
} // namespace filter
