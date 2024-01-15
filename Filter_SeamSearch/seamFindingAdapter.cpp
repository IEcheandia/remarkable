/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Filter that allows the access to every value coming out of the seam finding struct
*/

// WM includes
#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
#include "util/calibDataSingleton.h"
#include "seamFindingAdapter.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
	namespace filter {

		const std::string SeamFindingAdapter::m_oFilterName("SeamFindingAdapter");
		const std::string SeamFindingAdapter::m_oPipeOutDataName("SeamFindingAdapterDataOut");

		SeamFindingAdapter::SeamFindingAdapter() :
			TransformFilter(m_oFilterName, Poco::UUID{"17E75345-E368-41A9-A342-1DF29107E122"}),
			m_pPipeInSeamFinding(nullptr),
			m_oPipeOutData(this, m_oPipeOutDataName),
			m_oHwRoiY(0),
			m_oSeamFindingAdapter(eSeamPosition)
		{
			parameters_.add("SeamFindingComponent", Parameter::TYPE_int, static_cast<int>(m_oSeamFindingAdapter));

            setInPipeConnectors({{Poco::UUID("91F3D1B1-670B-4087-8FFF-05F70F9AE684"), m_pPipeInSeamFinding, "SeamFindingsIn", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("ADE5C053-AE62-4D94-BFE1-749B8752EB2E"), &m_oPipeOutData, m_oPipeOutDataName, 0, ""}});
            setVariantID(Poco::UUID("9AEF6FC2-F4DF-41D1-BDB8-58EA78F7CE26"));
		} // BlobAdapter

		void SeamFindingAdapter::setParameter()
		{
			TransformFilter::setParameter();
			//m_oCalibGrid = CalibrationSingleton::getCalibration();
			m_oSeamFindingAdapter = static_cast<SeamFindingAdapterType>(parameters_.getParameter("SeamFindingComponent").convert<int>());
		}

		void SeamFindingAdapter::paint()
		{
			if (m_oVerbosity < eLow)
			{
				return;
			}
			return;
		}

		bool SeamFindingAdapter::subscribe(BasePipe& p_rPipe, int p_oGroup)
		{
			m_pPipeInSeamFinding = dynamic_cast<seamfinding_pipe_t*>(&p_rPipe);
			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		}

		void SeamFindingAdapter::proceed(const void* p_pSender, PipeEventArgs&)
		{
			if (m_pPipeInSeamFinding == nullptr)
            {
                preSignalAction();
				return;
            }

			bool isValid = true;

			if (m_pPipeInSeamFinding == nullptr) isValid = false;
			poco_assert_dbg(m_pPipeInSeamFinding != nullptr);

			// Read in pipe
			const GeoSeamFindingarray&			rGeoSeamFindingList(m_pPipeInSeamFinding->read(m_oCounter));
			const SeamFindingarray&			rSeamFindingList(rGeoSeamFindingList.ref());
			const std::size_t			oNbSeamFindingsIn(rSeamFindingList.size());

			if (oNbSeamFindingsIn <= 0) isValid = false; // Keine Blobs, Rank schlecht
			if (rGeoSeamFindingList.rank() <= 0) isValid = false; // Rank schon vorher schlecht

			m_oSpTrafo = rGeoSeamFindingList.context().trafo();
			m_oHwRoiY = rGeoSeamFindingList.context().HW_ROI_y0;

			if (oNbSeamFindingsIn > 0)
			{
				m_oArrayData.assign(oNbSeamFindingsIn, 0, eRankMax);
			}
			else
			{
				m_oArrayData.assign(1, 0, eRankMin);
			}

			auto		oSeamFindingInIt = rSeamFindingList.getData().cbegin();
			const auto	oSeamFindingInEndIt = rSeamFindingList.getData().cend();
			auto		oArrayOutIt = m_oArrayData.getData().begin();
			//double h = 0, w = 0;

			if (isValid)
			{
				while (oSeamFindingInIt != oSeamFindingInEndIt)
				{
					switch (m_oSeamFindingAdapter)
					{
					case eSeamLeft:
						*oArrayOutIt = (double)oSeamFindingInIt->m_oSeamLeft;
						break;
					case eSeamRight:
						*oArrayOutIt = (double)oSeamFindingInIt->m_oSeamRight;
						break;
					case eSeamPosition:
						*oArrayOutIt = 0.5*(oSeamFindingInIt->m_oSeamLeft + oSeamFindingInIt->m_oSeamRight);
						break;
					case eSeamWidth:
						*oArrayOutIt = std::abs((double)(oSeamFindingInIt->m_oSeamRight - oSeamFindingInIt->m_oSeamLeft));
						break;
					case eAlgoType:
						*oArrayOutIt = (double)oSeamFindingInIt->m_oAlgoType;
						break;
					case eQuality:
						*oArrayOutIt = (double)oSeamFindingInIt->m_oQuality;
						break;
					default:
						std::ostringstream oMsg;
						oMsg << "No case for switch argument: " << m_oSeamFindingAdapter;
						wmLog(eWarning, oMsg.str().c_str());
						isValid = false;
					}
					++oSeamFindingInIt;
					++oArrayOutIt;
				} // while
			}

			if (isValid)
			{
				const GeoDoublearray		oGeoArrayOut(rGeoSeamFindingList.context(), m_oArrayData, rGeoSeamFindingList.analysisResult(), rGeoSeamFindingList.rank()); // detailed rank in array
				preSignalAction();
                m_oPipeOutData.signal(oGeoArrayOut);
			}
			else
			{
				const GeoDoublearray		oGeoArrayOut(rGeoSeamFindingList.context(), m_oArrayData, rGeoSeamFindingList.analysisResult(), 0.0); // detailed rank in array
				preSignalAction();
                m_oPipeOutData.signal(oGeoArrayOut);
			}
		} // proceed



	} // namespace filter
} // namespace precitec
