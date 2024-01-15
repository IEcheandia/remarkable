/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			OS
*  @date			2016
*  @file
*  @brief			Analog to Roi-Selector. Clips Roi to Begin/End-Detection result
*/

#include "startEndRoiSelector.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< Overlay
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>


using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string StartEndROISelector::m_oFilterName = std::string("StartEndROISelector");
		const std::string StartEndROISelector::PIPENAME1 = std::string("ImageFrame");
		const std::string StartEndROISelector::PIPENAME2 = std::string("SetROI");


		StartEndROISelector::StartEndROISelector() :
			TransformFilter(StartEndROISelector::m_oFilterName, Poco::UUID{"2E112FF8-7FFE-4093-BA77-DC5EAD9F3112"}),
			m_pPipeInImageFrame(nullptr),
			m_pPipeInStartEndInfo(nullptr),
			m_oPipeOutImageFrame(this, StartEndROISelector::PIPENAME1),
			m_oPipeOutSetROI(this, StartEndROISelector::PIPENAME2),
			m_oRoi(50, 50, 200, 200),
			m_oColor(Color::Green())
		{
			// Defaultwerte der Parameter setzen

			parameters_.add("X", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oRoi.x().start()));
			parameters_.add("Y", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oRoi.y().start()));
			parameters_.add("Width", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oRoi.width()));
			parameters_.add("Height", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oRoi.height()));
			parameters_.add("rgbRed", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.red));
			parameters_.add("rgbGreen", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.green));
			parameters_.add("rgbBlue", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.blue));
			parameters_.add("rgbAlpha", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.alpha));

            setInPipeConnectors({{Poco::UUID("E778358C-FBAD-4700-AACE-10D4E863DC9A"), m_pPipeInImageFrame, "ImageFrame", 1, ""},
            {Poco::UUID("93FC1A6E-B279-4DD8-86E8-8736656C922D"), m_pPipeInStartEndInfo, "StartEndInfo", 1, ""}});
            setOutPipeConnectors({{Poco::UUID("9943E40D-49DA-4ECD-9BFF-5AD815FD22B3"), &m_oPipeOutImageFrame, PIPENAME1, 0, PIPENAME1}, {Poco::UUID("F52988AE-DACB-44E7-B905-DE72930EACD2"), &m_oPipeOutSetROI, PIPENAME2, 0, PIPENAME2}});
            setVariantID({Poco::UUID("066399F7-23CF-441E-BD96-36CAF2A47E43"), Poco::UUID("C95F9D22-3DB6-4F43-956C-6EDE2753C3C8"), Poco::UUID("62DC1255-421A-4AA1-9992-3A79D0EBC305")}); //Three IDs
		}



		void StartEndROISelector::setParameter() {
			TransformFilter::setParameter();
			m_oRoi.x().start() = parameters_.getParameter("X").convert<int>();
			m_oRoi.y().start() = parameters_.getParameter("Y").convert<int>();
			m_oRoi.x().end() = m_oRoi.x().start() + parameters_.getParameter("Width").convert<int>();
			m_oRoi.y().end() = m_oRoi.y().start() + parameters_.getParameter("Height").convert<int>();

			m_oRoi.x().start() = std::abs(m_oRoi.x().start());
			m_oRoi.y().start() = std::abs(m_oRoi.y().start());
			m_oRoi.x().end() = std::abs(m_oRoi.x().end());
			m_oRoi.y().end() = std::abs(m_oRoi.y().end());

			m_oColor.red = parameters_.getParameter("rgbRed").convert<byte>();
			m_oColor.green = parameters_.getParameter("rgbGreen").convert<byte>();
			m_oColor.blue = parameters_.getParameter("rgbBlue").convert<byte>();
			m_oColor.alpha = parameters_.getParameter("rgbAlpha").convert<byte>();
		} // setParameter

		void StartEndROISelector::arm(const fliplib::ArmStateBase& state)
		{
				//std::cout << "\nFilter '" << m_oFilterName << "' armed at armstate " << state.getStateID() << std::endl;
		} // arm

		void StartEndROISelector::paint() {
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull()){
				return;
			} // if

			const Trafo		&rTrafo(*m_oSpTrafo);
			OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
			OverlayLayer	&rLayerLine(rCanvas.getLayerLine());
			// Zeichne Rechteck fuer ROI
			if (m_isCropped)
			{
				rLayerLine.add(new OverlayRectangle(rTrafo(m_croppedRoi), m_oColor));
			}
			else
			{
				rLayerLine.add(new OverlayRectangle(rTrafo(m_oRoi), m_oColor));
			}
		} // paint



		using fliplib::SynchronePipe;

		bool StartEndROISelector::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);
			if (p_rPipe.type() == typeid(GeoStartEndInfoarray))
				m_pPipeInStartEndInfo = dynamic_cast< fliplib::SynchronePipe < GeoStartEndInfoarray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		using geo2d::Rect;
		using geo2d::Size;

		/// die eigentliche Filterfunktion
		SmpBImage StartEndROISelector::selectRoi(BImage const& p_rImgIn, Rect &p_rRoi)
		{
			p_rRoi = intersect(Rect(Range(0, p_rImgIn.size().width), Range(0, p_rImgIn.size().height)), p_rRoi);

			if (p_rRoi.isValid() == false) {
				wmLogTr(eWarning, "QnxMsg.Filter.RoiReset", "ROI origin (%i, %i) lies out of image size (%i X %i). ROI origin reset to (0, 0).\n",
					p_rRoi.x().start(), p_rRoi.y().start(), p_rImgIn.size().width, p_rImgIn.size().height);
				p_rRoi.x().start() = 0;
				p_rRoi.y().start() = 0;
			} // if

			return new BImage(p_rImgIn, p_rRoi); // altes Bild mit neuem ROI - shallow copy
		} // selectRoi

		using fliplib::PipeEventArgs;

		void StartEndROISelector::proceedGroup(const void* sender, PipeGroupEventArgs& e)
		{
			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
			m_isCropped = false;

			// hole const referenz aus pipe
			const ImageFrame &rFrame = m_pPipeInImageFrame->read(m_oCounter);
			m_oSpTrafo = rFrame.context().trafo();

			// data
			const interface::GeoStartEndInfoarray &rGeoStartEndInfoArrayIn = m_pPipeInStartEndInfo->read(m_oCounter);
			unsigned int oSizeOfArray = rGeoStartEndInfoArrayIn.ref().size();
            int startValidRangeY = 0;
            int endValidRangeY = 0;
            if (oSizeOfArray <= 0)
			{
				m_isCropped = false;
			}
			else
			{
				geo2d::StartEndInfo oInInfo = std::get<eData>(rGeoStartEndInfoArrayIn.ref()[0]);
				m_isCropped = oInInfo.isCropped;
				//int oInRank = std::get<eRank>(rGeoStartEndInfoArrayIn.ref()[0]);
                SmpTrafo oStartEndTrafo = rGeoStartEndInfoArrayIn.context().trafo();
                int offsetY = 0;
                if ( !oStartEndTrafo.isNull() )
                {
                    offsetY = oStartEndTrafo->dy() - m_oSpTrafo->dy();
                }
                startValidRangeY = oInInfo.m_oStartValidRangeY  + offsetY;
                endValidRangeY = oInInfo.m_oEndValidRangeY + offsetY;
			}

			if (rFrame.data().isValid())
			{
				// die eigentiche Filterfunktion
				//SmpBImage

                // Trafo von globalImage zu subImage
                LinearTrafo subTrafo(offset(m_oRoi));

                // Set for ImageFrame with ROI setting (Filter parameter)
                oSmpImgROIParaSet = selectRoi(rFrame.data(), m_oRoi);
                
				if (m_isCropped)
				{
					int x1 = m_oRoi.x().start();
					int x2 = m_oRoi.x().end();
					int y1 = m_oRoi.y().start();
					int y2 = m_oRoi.y().end();
					int newY1 = (y1 > startValidRangeY) ? y1 : startValidRangeY;
					int newY2 = (y2 < endValidRangeY) ? y2 : endValidRangeY;
					if (newY1 >= newY2) // Roi quasi Null
					{
						newY1 = (m_oRoi.y().end() - m_oRoi.y().start()) / 2 - 1;
						newY2 = (m_oRoi.y().end() - m_oRoi.y().start()) / 2 + 1;
					}
					m_croppedRoi = Rect(x1, newY1, x2 - x1 + 1, newY2 - newY1 + 1);
					oSmpNewImage = selectRoi(rFrame.data(), m_croppedRoi);
                    subTrafo = LinearTrafo(offset(m_croppedRoi));
				}
				else
				{
					oSmpNewImage = selectRoi(rFrame.data(), m_oRoi);
				}

				// Frame-/Contextverwaltung

				// das ROI hat einen neuen Kontext = alter Kontext bis auf die Trafo
				ImageContext oNewContext(rFrame.context(), subTrafo(rFrame.context().trafo()));
				const auto oAnalysisResult = rFrame.analysisResult() == AnalysisOK ? AnalysisOK : rFrame.analysisResult(); // replace 2nd AnalysisOK by your result type
				// Neues ImageFrame versenden (Imageframe wird kopiert -> es bleiben keine Referenzen auf lokale Variable)
				preSignalAction();
				m_oPipeOutImageFrame.signal(ImageFrame(oNewContext, *oSmpNewImage, oAnalysisResult));
				m_oPipeOutSetROI.signal(ImageFrame(oNewContext, *oSmpImgROIParaSet, oAnalysisResult));
			}
			else
			{
				preSignalAction();
				m_oPipeOutImageFrame.signal(ImageFrame(rFrame.context(), BImage(), rFrame.analysisResult())); // return zero image
				m_oPipeOutSetROI.signal(ImageFrame(rFrame.context(), BImage(), rFrame.analysisResult())); // return zero image
			}

		} // proceed



	} // namespace filter
} // namespace precitec

