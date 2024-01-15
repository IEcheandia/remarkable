/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2015
* 	@brief 		This filter is able to shift some lines in an image
*/

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "lineShifter.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string LineShifter::m_oFilterName = std::string("LineShifter");
		const std::string LineShifter::PIPENAME_IMAGE_OUT = std::string("ImageFrameOut");


		LineShifter::LineShifter() :
			TransformFilter(LineShifter::m_oFilterName, Poco::UUID{"ae4cb6f9-7d0b-40d4-ada2-a7f3b6899f50"}),
			m_pPipeInImageFrame(NULL),
			m_oMode(0),
			m_oNoShiftLinePercent(50),
		    m_oShiftTop(5),
		    m_oShiftBottom(5),
		    m_oFillGreyValue(128),

		    m_oX1(0),
			m_oY1(0),
			m_oX2(0),
			m_oY2(0),
		    m_oX3(0),
			m_oY3(0)

		{
			m_pPipeOutImageFrame = new SynchronePipe< interface::ImageFrame >(this, LineShifter::PIPENAME_IMAGE_OUT);

			// Set default values of the parameters of the filter
			parameters_.add("Mode", Parameter::TYPE_int, m_oMode);
			parameters_.add("NoShiftLinePercent", Parameter::TYPE_int, m_oNoShiftLinePercent);
			parameters_.add("ShiftTop", Parameter::TYPE_int, m_oShiftTop);
			parameters_.add("ShiftBottom", Parameter::TYPE_int, m_oShiftBottom);
			parameters_.add("FillValue", Parameter::TYPE_int, m_oFillGreyValue);
			parameters_.add("X1", Parameter::TYPE_int, m_oX1);
			parameters_.add("Y1", Parameter::TYPE_int, m_oY1);
			parameters_.add("X2", Parameter::TYPE_int, m_oX2);
			parameters_.add("Y2", Parameter::TYPE_int, m_oY2);
			parameters_.add("X3", Parameter::TYPE_int, m_oX3);
			parameters_.add("Y3", Parameter::TYPE_int, m_oY3);

            setInPipeConnectors({{Poco::UUID("E8B5F4EF-236B-4588-A78A-3CCCEE2868F5"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("46778099-C8D2-45FA-B744-F236B3DCC9F3"), m_pPipeOutImageFrame, "ImageFrameOut", 0, ""}});
            setVariantID(Poco::UUID("5dc60249-32b6-4cba-b525-61ca5dcf2ee7"));
		}

		LineShifter::~LineShifter()
		{
			delete m_pPipeOutImageFrame;

		}

		void LineShifter::setParameter()
		{
			TransformFilter::setParameter();
			m_oMode = parameters_.getParameter("Mode").convert<int>();
			m_oNoShiftLinePercent = parameters_.getParameter("NoShiftLinePercent").convert<int>();
			m_oShiftTop = parameters_.getParameter("ShiftTop").convert<int>();
			m_oShiftBottom = parameters_.getParameter("ShiftBottom").convert<int>();
			m_oFillGreyValue = parameters_.getParameter("FillValue").convert<int>();
			m_oX1 = parameters_.getParameter("X1").convert<int>();
			m_oY1 = parameters_.getParameter("Y1").convert<int>();
			m_oX2 = parameters_.getParameter("X2").convert<int>();
			m_oY2 = parameters_.getParameter("Y2").convert<int>();
			m_oX3 = parameters_.getParameter("X3").convert<int>();
			m_oY3 = parameters_.getParameter("Y3").convert<int>();

		} // setParameter

		bool LineShifter::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);


			return BaseFilter::subscribe(p_rPipe, p_oGroup);

		} // subscribe

		void LineShifter::paint()
		{
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
			{
				return;
			} // if

			if (!m_hasPainting) return;

			try
			{

				const Trafo		&rTrafo(*m_oSpTrafo);
				OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
				//OverlayLayer	&rLayerContour(rCanvas.getLayerContour());
				OverlayLayer	&rLayerImage(rCanvas.getLayerImage());

				// paint here
				const auto		oPosition = rTrafo(Point(0, 0));
				const auto		oTitle = OverlayText("shifted image", Font(), Rect(150, 18), Color::Black());

				rLayerImage.add(new OverlayImage(oPosition, m_imageOut, oTitle));

			}
			catch (...)
			{
				return;
			}
		} // paint


		void LineShifter::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
		{

			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

			geo2d::Doublearray oOutImageFrame;


			// Read out image frame from pipe
			const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
			ImageFrame frameOut(rFrameIn);
			const BImage &rImageIn = rFrameIn.data();
			//BImage imageOut;
			m_imageOut.resize(rImageIn.size());

			int imageWidth = rImageIn.width();
			int imageHeight = rImageIn.height();


			try
			{
				m_oSpTrafo = rFrameIn.context().trafo();


				if (false) // bei Problemen
				{

					preSignalAction(); //logTiming();

					m_pPipeOutImageFrame->signal(rFrameIn);

					return; // RETURN
				}

				// Here: Do some calculation
				// Fill up oOutDoubleX and LaserLineOut
				m_hasPainting = true;

				// Parameter zuweisen
				int noShiftLinePercent = m_oNoShiftLinePercent;
				if (noShiftLinePercent < 0) noShiftLinePercent = 0;
				if (noShiftLinePercent > 99) noShiftLinePercent = 99;
				double shiftTop = m_oShiftTop;
				double shiftBottom = m_oShiftBottom;
				int fillGreyVal = m_oFillGreyValue;

				// weitere Varaiblen berechnen
				int noShiftLine = (int)(0.5 + (noShiftLinePercent / 100.00) * imageHeight);
                if ( noShiftLine >= imageHeight ) { noShiftLine = imageHeight - 1; }
				int linesTop = noShiftLine;
				int linesBottom = imageHeight - linesTop - 1;

				//int lineCounterTop = 0;
				//int lineCounterBottom = 0;
				int currentLine = 0;
				int currentShift = 0;

				for (int i = 0; i < imageHeight; i++) // Ziel fuellen
				{
					for (int j = 0; j<imageWidth; j++)
					{
						m_imageOut[i][j] = fillGreyVal;
					}
				}

				if (noShiftLine > 0) // es gibt einen Bereich ueber der NoShiftLine
				{
					for (int i = 0; i < linesTop; i++)
					{
						currentLine = noShiftLine - i;
						currentShift = (int)(0.5 + (i + 1) * (shiftTop / linesTop));

						for (int j = 0; j < imageWidth; j++)
						{
							if (j + currentShift >= imageWidth) continue;
							if (j + currentShift < 0) continue;
							m_imageOut[currentLine][j + currentShift] = rImageIn[currentLine][j];
						}
					}
				}

				if (noShiftLine < imageHeight - 1) // es gibt einen Bereich unter der NoShiftLine
				{
					for (int i = 0; i < linesBottom; i++)
					{
						currentLine = noShiftLine + i;
						currentShift = (int)(0.5 + (i + 1) * (shiftBottom / linesBottom));

						for (int j = 0; j < imageWidth; j++)
						{
							if (j + currentShift >= imageWidth) continue;
							if (j + currentShift < 0) continue;
							m_imageOut[currentLine][j + currentShift] = rImageIn[currentLine][j];
						}
					}
				}

				// noShiftLine kopieren
				for (int j = 0; j < imageWidth; j++) m_imageOut[noShiftLine][j] = rImageIn[noShiftLine][j];

				////////

				const ImageFrame frameOut(rFrameIn.context(), m_imageOut, rFrameIn.analysisResult()); // put image into frame


				preSignalAction();

				m_pPipeOutImageFrame->signal(frameOut);


			}
			catch (...)
			{

				preSignalAction();

				m_pPipeOutImageFrame->signal(rFrameIn);

				return;
			}
		} // proceedGroup



	} // namespace precitec
} // namespace filter
