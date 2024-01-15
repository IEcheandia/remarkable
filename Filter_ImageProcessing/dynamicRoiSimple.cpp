/**
 *  Filter_ImageProcessing::dynamicRoiSimple.cpp
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         JS
 *	@date           06.2015
 *	@brief			dynamic ROI with two inputs: x0 and dx
 */

#include "dynamicRoiSimple.h"
#include "common/frame.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "common/geoContext.h"
#include "event/results.h"
#include "module/moduleLogger.h"
#include <filter/armStates.h>

#include <fliplib/TypeToDataTypeImpl.h>

using fliplib::Parameter;

namespace precitec {
	using namespace interface;
	using namespace geo2d;
	using namespace image;
namespace filter {

const std::string DynamicRoiSimple::m_filterName {std::string("DynamicRoiSimple")};

	DynamicRoiSimple::DynamicRoiSimple()
		: TransformFilter(m_filterName, Poco::UUID{"91739228-7B13-4E73-8F38-E61FCA153388"}),
		m_pPipeImage(NULL),
		m_pPipeX(NULL),
		m_pPipeDx(NULL),
		m_oY0(0),
		m_oDY(0),
		m_xIsStart(false),
		m_oPipeSubImage(this, "SubFrame")	///< tag-name for identifying output (not used for single outputs)
  {
		// add parameters to parameter list
		// Defaultwerte der Parameter setzen
		parameters_.add("roiY", Parameter::TYPE_int, static_cast<int>(m_oY0));
		parameters_.add("roiDy", Parameter::TYPE_int, static_cast<int>(m_oDY));
        parameters_.add("xIsStart", Parameter::TYPE_bool, static_cast<bool>(m_xIsStart));

        setInPipeConnectors({{Poco::UUID("79278FC4-D10C-48EF-B2B3-BC52B49CF39F"), m_pPipeImage, "ImageFrame", 1, "image"},
        {Poco::UUID("33EF78EE-2EC1-42CA-8ECA-3B335F9A7824"), m_pPipeX, "roiX", 1, "roiX"},
        {Poco::UUID("763478A5-78E2-4269-9DF4-92B9E9A3718A"), m_pPipeDx, "RoiDx", 1, "roiDx"}});
        setOutPipeConnectors({{Poco::UUID("168FF59E-B64A-4D64-B268-C4C6D44D6530"), &m_oPipeSubImage, "SubFrame", 0, "subFrame"}});
        setVariantID(Poco::UUID("91D5509D-7E55-4BC7-A418-FFC26699B7F9"));
	}

	void DynamicRoiSimple::setParameter() {
		// here BaseFilter sets the common parameters (currently verbosity)
		TransformFilter::setParameter();

		TransformFilter::setParameter();
		m_oY0 = static_cast<int>(parameters_.getParameter("roiY").convert<int>());
		m_oDY = static_cast<int>(parameters_.getParameter("roiDy").convert<int>());
        m_xIsStart = static_cast<bool>(parameters_.getParameter("xIsStart").convert<bool>());
	}


	bool DynamicRoiSimple::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {

		if (p_rPipe.tag() == "image") { // die tag Namen im sql SCript unter tag unter Connectir IDs Values eintragen
			m_pPipeImage = dynamic_cast<fliplib::SynchronePipe<ImageFrame> * >(&p_rPipe);
		}

		if (p_rPipe.tag() == "roiX") {
			m_pPipeX = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
		}


		if (p_rPipe.tag() == "roiDx") {
			m_pPipeDx = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
		}

		// herewith we set the proceed as callback (return is always true)
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	} // subscribe

	void DynamicRoiSimple::paint() {

		if(m_oVerbosity >= eLow) {

			if (!m_oRoi.isEmpty())
			{
				OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
				OverlayLayer				&rLayerLine				( rCanvas.getLayerLine());

                const interface::Trafo& trafo{*m_smpTrafo};
                Color color{m_isRoiValid ? Color::Green() : Color::Red()};
                if (m_oVerbosity > eLow || !m_isRoiValid)
                {
                    rLayerLine.add<OverlayRectangle>(trafo(m_oRoi), color);
                }
			}
		}
	}


	void	DynamicRoiSimple::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
    {
        m_isRoiValid = false;

		// the arguments of this function are unclear??
		// here we check only for existence of the inputs, not their well-formedness
		if ((m_pPipeImage == nullptr) || (m_pPipeX == nullptr) || (m_pPipeDx == nullptr))
        {
			// empty rect for drawing

			// signal error output == empty image
			ImageFrame oIf;
			oIf.setAnalysisResult(interface::AnalysisErrBadImage);
			preSignalAction();
            m_oPipeSubImage.signal(oIf);
            return;
		}

        // connect input variable to pipe content
        /// scalar input value(s)
        ImageFrame const& imageIn( m_pPipeImage->read(m_oCounter) );
        m_smpTrafo = imageIn.context().trafo();

        // check all incoming results for errors (AnalysisOK)
        interface::ResultType oRes = interface::AnalysisOK;

        if (m_pPipeX->read(m_oCounter).analysisResult() != interface::AnalysisOK)
        {
            oRes = m_pPipeX->read(m_oCounter).analysisResult();
        } else if (m_pPipeDx->read(m_oCounter).analysisResult() != interface::AnalysisOK)
        {
            oRes = m_pPipeDx->read(m_oCounter).analysisResult();
        }
        if (oRes != interface::AnalysisOK) // incorrect ROI -> forward image
        {
            ImageFrame imageFrameIn(m_pPipeImage->read(m_oCounter));
            imageFrameIn.setAnalysisResult(oRes);
            preSignalAction(); m_oPipeSubImage.signal(imageFrameIn);
            return;
        }

        // trafo of image
        Rect roi{0, 0, 0, 0};
        if (m_pPipeX->read(m_oCounter).ref().getRank()[0] > eRankMin)
        {
            // Wenn die Informationen ueber die X-Koordinate belastbar sind, dann berechne aus diesen Informationen
            // das ROI neu. Fuer den Fall ist auch die Transformation belastbar. Ferner wird die Transformation ausserhalb dieser
            // Zeilen nicht benutzt, also ist die lokal.
            //
            // Fuer den Fall, dass die Informationen nicht belastbar sind, bleibt das ROI so, wie es war.
            SmpTrafo pTrafo = m_pPipeX->read(m_oCounter).context().trafo();

            int oTemp = static_cast<int>(m_pPipeX->read(m_oCounter).ref().getData()[0] + pTrafo->dx() + 0.5);
            uInt xStart;
            if (oTemp > 0)
            {
                xStart = oTemp;
            }
            else{
                xStart = 0;
            }

            uInt xWidth(uInt(m_pPipeDx->read(m_oCounter).ref().getData()[0]));

            //Der Filter(pipe) Eingang liefert den x Mittelpunkt des ROis:
            if (!m_xIsStart)
            {
                if (xStart > (xWidth / 2))
                {
                    xStart = xStart - xWidth / 2;
                }
                else
                {
                    // xStart ist so klein, dass es negativ werden wuerde, wenn wir davon jetzt noch was abziehen. Der Typ ist aber unsigned, also wollen wir das nicht machen.
                    xStart = 0;
                }
            }

            uInt xEnd = xStart + xWidth;
            uInt yStart(m_oY0);
            uInt yEnd(m_oDY + yStart);


            roi = Rect(Range(xStart, xEnd), Range(yStart, yEnd));
        }

        // Die Verarbeitung des Bildes muss in jedem Fall erfolgen, auch und insbesondere dann, wenn die Information ueber die
        // X-Koordinate nicht belastbar war - das ist der Punkt.
        m_oRoi = roi;
        auto imageOut = createSubImage(imageIn);
        if (!imageOut.data().isValid())
        {
            m_oRoi = roi;
        }
        preSignalAction();
        m_oPipeSubImage.signal(imageOut);
	}


	/**
	 * extract subImage
	 * @param p_rIn source image
	 * @param p_rRoi rect at which to extract subimage
	 * @return frame with subImage of image at rect
	 */
	ImageFrame	DynamicRoiSimple::createSubImage(ImageFrame const& imageIn) {
		using namespace geo2d;

		// roi must fit into original image
		const Rect inRoi {Range(0, imageIn.data().size().width),  Range(0, imageIn.data().size().height)};

        const bool roiTooBig = inRoi.width() < m_oRoi.width() || inRoi.height() < m_oRoi.height();
        const bool roiNotInImage = intersect(inRoi, m_oRoi).isEmpty();
		if (roiTooBig || roiNotInImage || !imageIn.data().isValid())
		{
            if (roiTooBig)
            {
                wmLog(eWarning, "Filter %s: roi is too big. Actual width: %f, height: %f, maximum allowed width: %f, height %f.\n", m_filterName, m_oRoi.width(), m_oRoi.height(), inRoi.width(), inRoi.height());
            }
            else if (roiNotInImage)
            {
                wmLog(eWarning, "Filter %s: roi is completely outside the image.\n", m_filterName);
            }
			return ImageFrame(imageIn.context(), BImage());
		} // if roi is invald return empty image

        translateIntoInputRoi(inRoi);

		// transformation from offset of current image to offset of roi
		LinearTrafo oSubTrafo(offset(m_oRoi));

		// the new context is the old one with the modified(=concated) transformation
		ImageContext oNewContext( imageIn.context(), oSubTrafo(imageIn.context().trafo()) );

		// here the new image is created
		const BImage oNewImage (imageIn.data(), m_oRoi);
        m_isRoiValid = true;
		return ImageFrame(oNewContext, oNewImage, imageIn.analysisResult());
	}

    void DynamicRoiSimple::translateIntoInputRoi(const Rect& inRoi)
    {
        if (m_oRoi.x().end() > inRoi.x().end())
        {
            m_oRoi.x().start() = inRoi.x().end() - m_oRoi.width();
            m_oRoi.x().end() = inRoi.x().end();
        }
        if (m_oRoi.y().end() > inRoi.y().end())
        {
            m_oRoi.y().start() = inRoi.y().end() - m_oRoi.height();
            m_oRoi.y().end() = inRoi.y().end();
        }
    }

	/*virtual*/ void
	DynamicRoiSimple::arm(const fliplib::ArmStateBase& state) {
		if (state.getStateID() == eSeamStart)
		{
			// Fuer dieses Filter steht die ausgegebene Information direkt im ROI drin, d.h. das muss initialisiert werden.
			// Die Information zu ueberschreiben, ist in Ordnung, weil das am Anfang von proceedGroup() sowieso neu gesetzt wird.
			m_oRoi = Rect();
		}
	} // arm

} // namespace filter
} // namespace precitec
