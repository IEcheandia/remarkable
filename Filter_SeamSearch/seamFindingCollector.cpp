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
#include "seamFindingCollector.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string SeamFindingCollector::m_oFilterName = std::string("SeamFindingCollector");
		const std::string SeamFindingCollector::PIPENAME_OUT = std::string("SeamFindingOut");

		SeamFindingCollector::SeamFindingCollector() :
			TransformFilter(SeamFindingCollector::m_oFilterName, Poco::UUID{"26432E03-D171-416F-8981-64B31307BD3B"}),
			m_pPipeInSeamFindings1(NULL),
			m_pPipeInSeamFindings2(NULL),
			m_pPipeOutSeamFindings(NULL),
			m_oMode(0)
		{
			m_pPipeOutSeamFindings = new SynchronePipe< GeoSeamFindingarray >(this, SeamFindingCollector::PIPENAME_OUT);

			// Set default values of the parameters of the filter
			parameters_.add("Mode", Parameter::TYPE_int, m_oMode);

            setInPipeConnectors({{Poco::UUID("0D606F0C-FAAE-409F-AFE4-BED139F49A4C"), m_pPipeInSeamFindings1, "SeamFindings1", 1, "SeamFindings1"},
            {Poco::UUID("9C14DF02-4510-484E-A494-B009581FF554"), m_pPipeInSeamFindings2, "SeamFindings2", 1, "SeamFindings2"}});
            setOutPipeConnectors({{Poco::UUID("C3C32432-7BC5-494A-94EA-EA5F40A41882"), m_pPipeOutSeamFindings, PIPENAME_OUT, 0, ""}});
            setVariantID(Poco::UUID("2B3C26CE-C6DB-4B7D-ABC5-603EA93338D5"));
		}

		SeamFindingCollector::~SeamFindingCollector()
		{
			delete m_pPipeOutSeamFindings;
		} // ~LineProfile

		void SeamFindingCollector::setParameter()
		{
			TransformFilter::setParameter();
			m_oMode = parameters_.getParameter("Mode").convert<int>();
		} // setParameter

		bool SeamFindingCollector::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.tag() == "SeamFindings1")
				m_pPipeInSeamFindings1 = dynamic_cast< fliplib::SynchronePipe < interface::GeoSeamFindingarray > * >(&p_rPipe);
			if (p_rPipe.tag() == "SeamFindings2")
				m_pPipeInSeamFindings2 = dynamic_cast< fliplib::SynchronePipe < interface::GeoSeamFindingarray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void SeamFindingCollector::paint()
		{
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
			{
				return;
			} // if

		} // paint


		void SeamFindingCollector::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInSeamFindings1 != nullptr); // to be asserted by graph editor
			poco_assert_dbg(m_pPipeInSeamFindings2 != nullptr); // to be asserted by graph editor

			geo2d::SeamFindingarray outArray;
			//outArray[0].getData().
			GeoSeamFindingarray geoArray1 = m_pPipeInSeamFindings1->read(m_oCounter);
			GeoSeamFindingarray geoArray2 = m_pPipeInSeamFindings2->read(m_oCounter);
			SeamFindingarray array1 = geoArray1.ref();
			SeamFindingarray array2 = geoArray2.ref();

			for (int i = 0; i < (int)array1.size(); i++)
			{
				outArray.getData().push_back(array1.getData()[i]);
				outArray.getRank().push_back(array1.getRank()[i]);
			}

			for (int i = 0; i < (int)array2.size(); i++)
			{
				outArray.getData().push_back(array2.getData()[i]);
				outArray.getRank().push_back(array2.getRank()[i]);
			}

			// Create a new byte array, and put the global context into the resulting profile
			const auto oAnalysisResult = geoArray1.analysisResult() == AnalysisOK ? AnalysisOK : geoArray1.analysisResult(); // replace 2nd AnalysisOK by your result type
			const GeoSeamFindingarray &rGeoSFA = GeoSeamFindingarray(geoArray1.context(), outArray, oAnalysisResult, filter::eRankMax);
			preSignalAction();
			m_pPipeOutSeamFindings->signal(rGeoSFA);

		} // proceedGroup
	} // namespace precitec
} // namespace filter
