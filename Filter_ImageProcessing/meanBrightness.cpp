/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter calculates the mean brightness in the whole image.
*/

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "meanBrightness.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string MeanBrightness::m_oFilterName = std::string("MeanBrightness");
		const std::string MeanBrightness::PIPENAME_MEAN_OUT = std::string("MeanOut");

		MeanBrightness::MeanBrightness() :
			TransformFilter(MeanBrightness::m_oFilterName, Poco::UUID{"AD39B6C7-E93F-4D41-B96D-13F211835299"}),
			m_pPipeInImageFrame(NULL),
			m_oResolutionX(1),
			m_oResolutionY(1)
		{
			m_pPipeOutMean = new SynchronePipe< interface::GeoDoublearray >(this, MeanBrightness::PIPENAME_MEAN_OUT);

			// Set default values of the parameters of the filter
			parameters_.add("ResolutionX", Parameter::TYPE_int, m_oResolutionX);
			parameters_.add("ResolutionY", Parameter::TYPE_int, m_oResolutionY);

            setInPipeConnectors({{Poco::UUID("B9781785-7106-44AF-ADA6-4C8840AD7C8C"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("85138719-FC43-40A0-BC33-DBF135F55304"), m_pPipeOutMean, "MeanOut", 0, ""}});
            setVariantID(Poco::UUID("0D3A7608-2BEA-4FE2-9379-9170E0D0F8B9"));
		}

		MeanBrightness::~MeanBrightness()
		{
			delete m_pPipeOutMean;
		}

		void MeanBrightness::setParameter()
		{
			TransformFilter::setParameter();
			m_oResolutionX = parameters_.getParameter("ResolutionX").convert<int>();
			m_oResolutionY = parameters_.getParameter("ResolutionY").convert<int>();
		}

		bool MeanBrightness::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		}

		void MeanBrightness::paint()
		{
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
			{
				return;
			} // if

			if (!m_hasPainting) return;

			try
			{

				// const Trafo		&rTrafo(*m_oSpTrafo);
				// OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
				// OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

				// paint here

			}
			catch (...)
			{
				return;
			}
		} // paint


		void MeanBrightness::proceed(const void* sender, fliplib::PipeEventArgs& e)
		{
			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
			m_hasPainting = false;
			geo2d::Doublearray oOutMean;

			// Read out image frame from pipe
			const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
			const BImage &rImageIn = rFrameIn.data();
			int imageHeight = rImageIn.height();
			int imageWidth = rImageIn.width();

			try
			{
				m_oSpTrafo = rFrameIn.context().trafo();

				if (imageHeight*imageWidth == 0)
				{
					oOutMean.getData().push_back(0);
					oOutMean.getRank().push_back(0);

					const GeoDoublearray &rDouble = GeoDoublearray(rFrameIn.context(), oOutMean, rFrameIn.analysisResult(), interface::NotPresent);

					preSignalAction();

					m_pPipeOutMean->signal(rDouble);
					return;
				}

				long sum = 0;
				int counter = 0;

				for (int y = 0; y < imageHeight; y += m_oResolutionY)
				{
					for (int x = 0; x < imageWidth; x += m_oResolutionX)
					{
						sum += rImageIn[y][x];
						counter++;
					}
				}
				if (counter == 0) counter = 1;
				double mean = ((double)sum) / counter;

				oOutMean.getData().push_back(mean);
				oOutMean.getRank().push_back(255);

				////////

				const GeoDoublearray &rDouble = GeoDoublearray(rFrameIn.context(), oOutMean, rFrameIn.analysisResult(), filter::eRankMax);

				preSignalAction();

				m_pPipeOutMean->signal(rDouble);

			}
			catch (...)
			{
				oOutMean.getData().push_back(0);
				oOutMean.getRank().push_back(0);

				const GeoDoublearray &rDouble = GeoDoublearray(rFrameIn.context(), oOutMean, rFrameIn.analysisResult(), interface::NotPresent);

				preSignalAction();

				m_pPipeOutMean->signal(rDouble);

				return;
			}
		} // proceedGroup
	} // namespace precitec
} // namespace filter
