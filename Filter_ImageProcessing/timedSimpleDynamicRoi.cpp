/**
 *  Filter_ImageProcessing::dynamicRoiSimple.cpp
 *	@file
 *	@copyright      Precitec GmbH & Co. KG
 *	@author         Duw
 *	@date           06.2015
 *	@brief			Filter to create subRoi-image from image and 1 double input denoting ROI. After a given amount of time, the width of the ROI is reduced to avoid false detections.
 */

#include "timedSimpleDynamicRoi.h"
#include "common/frame.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "common/geoContext.h"
#include "event/results.h"
#include "module/moduleLogger.h"
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>

using fliplib::Parameter;

namespace precitec {
	using namespace interface;
	using namespace geo2d;
	using namespace image;
namespace filter {

	const std::string TimedSimpleDynamicRoi::m_oFilterName("TimedSimpleDynamicRoi");
	const std::string TimedSimpleDynamicRoi::m_oPipeOutName("SubFrame");				///< Pipe: Data out-pipe. "SubFrame" is the name that has been used in other ROI contexts.
	const std::string TimedSimpleDynamicRoi::m_oParamY0Name("roiY");			///< Parametername: Y-Position
	const std::string TimedSimpleDynamicRoi::m_oParamX0Name("roiX");			///< Parametername: X-Position
	const std::string TimedSimpleDynamicRoi::m_oParamDYName("roiDy");			///< Parametername: Height
	const std::string TimedSimpleDynamicRoi::m_oParamDXLargeName("roiDxLarge");			///< Parametername: Large width
	const std::string TimedSimpleDynamicRoi::m_oParamDXSmallName("roiDxSmall");			///< Parametername: Small width
	const std::string TimedSimpleDynamicRoi::m_oParamThresholdName("SelectThreshold");			///< Parametername: Amount of delay [um].

	TimedSimpleDynamicRoi::TimedSimpleDynamicRoi()
		: TransformFilter(TimedSimpleDynamicRoi::m_oFilterName, Poco::UUID{"74703BE3-C6C7-46C8-A0E8-60A65EEC1AD4"}),
		m_pPipeInImageFrame(NULL),
		m_pPipeInX(NULL),
		m_oY0(0),
		m_oX0(0),
		m_oDY(0),
		m_oDXLarge(0),
		m_oDXSmall(0),
		m_oThreshold(0),
		m_oTriggerDelta(0),
		m_oPipeSubImage(this, m_oPipeOutName)	///< tag-name for identifying output (not used for single outputs)
  {
		// add parameters to parameter list
		// Defaultwerte der Parameter setzen
		parameters_.add(m_oParamX0Name, Parameter::TYPE_int, m_oX0);
		parameters_.add(m_oParamY0Name, Parameter::TYPE_int, m_oY0);
		parameters_.add(m_oParamDYName, Parameter::TYPE_int, m_oDY);
		parameters_.add(m_oParamDXLargeName, Parameter::TYPE_int, m_oDXLarge);
		parameters_.add(m_oParamDXSmallName, Parameter::TYPE_int, m_oDXSmall);
		parameters_.add(m_oParamThresholdName, Parameter::TYPE_uint, m_oThreshold);

        setInPipeConnectors({{Poco::UUID("D8E0AC43-AAB6-42BD-93D1-ADCCDEE319A3"), m_pPipeInImageFrame, "ImageFrame", 1, "image"},
        {Poco::UUID("944D994D-92EF-43CE-9094-2E28813AE08B"), m_pPipeInX, "roiX", 1, "roiX"}});
        setOutPipeConnectors({{Poco::UUID("6B5ECC10-F9D2-4D2C-858D-2FFE97AFBF39"), &m_oPipeSubImage, m_oPipeOutName, 0, "subFrame"}});
        setVariantID(Poco::UUID("A8D05F1B-30BA-4ED6-856A-A40EBA36A183"));
	}

	void TimedSimpleDynamicRoi::setParameter() {
		// here BaseFilter sets the common parameters (currently verbosity)
		TransformFilter::setParameter();

		m_oX0 = parameters_.getParameter(m_oParamX0Name).convert<int>();
		m_oY0 = parameters_.getParameter(m_oParamY0Name).convert<int>();
		m_oDY = parameters_.getParameter(m_oParamDYName).convert<int>();
		m_oDXLarge = parameters_.getParameter(m_oParamDXLargeName).convert<int>();
		m_oDXSmall = parameters_.getParameter(m_oParamDXSmallName).convert<int>();
		m_oThreshold = parameters_.getParameter(m_oParamThresholdName).convert<unsigned int>();
	} // setParameter


	bool TimedSimpleDynamicRoi::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {

		if (p_rPipe.tag() == "image") { // die tag Namen im sql SCript unter tag unter Connectir IDs Values eintragen
			m_pPipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<ImageFrame> * >(&p_rPipe);
		}

		if (p_rPipe.tag() == "roiX") {
			m_pPipeInX = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
		}

		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	} // subscribe

	void TimedSimpleDynamicRoi::paint() {

		if (m_oVerbosity >= eLow ) {
			if (!m_oRoi.isEmpty())  {
				OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
				OverlayLayer				&rLayerLine				( rCanvas.getLayerLine());
				// draw final roi
				rLayerLine.add( new OverlayRectangle(m_oRoi, Color::Green()) );
			}
		}
	}


	void	TimedSimpleDynamicRoi::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) {
		// the arguments of this function are unclear??
		// here we check only for existence of the inputs, not their well-formedness
		if ((m_pPipeInImageFrame == nullptr)
			|| (m_pPipeInX == nullptr)
				){
			// empty rect for drawing

			// signal error output == empty image
			ImageFrame oIf;
			oIf.setAnalysisResult(interface::AnalysisErrBadImage);
			preSignalAction(); m_oPipeSubImage.signal(oIf);
		} else {
			// connect input variable to pipe content
			/// scalar input value(s)
			ImageFrame const& oIn(m_pPipeInImageFrame->read(m_oCounter));
			const interface::ImageContext& rContext = oIn.context();
			unsigned long oPosition = rContext.position();

			// all ROI data valid, so proceed
			uInt xWidth = 0;
			uInt xStart = 0;

			bool bConsiderInput = (oPosition >= m_oThreshold);    // "larger or equal" is necessary in case Threshold == 0
			if (bConsiderInput)
			{
				// check all incoming results for errors (AnalysisOK)
				interface::ResultType oRes = m_pPipeInX->read(m_oCounter).analysisResult();

				if (oRes == interface::AnalysisOK)
				{
					if (m_pPipeInX->read(m_oCounter).ref().getRank()[0] > eRankMin)
					{
						// Wenn die Informationen ueber die X-Koordinate belastbar sind, dann berechne aus diesen Informationen
						// das ROI neu. Fuer den Fall ist auch die Transformation belastbar.
						//
						// Fuer den Fall, dass die Informationen nicht belastbar sind, bleibt das ROI so, wie es war.

						int oTemp = static_cast<int>(m_pPipeInX->read(m_oCounter).ref().getData()[0] + m_pPipeInX->read(m_oCounter).context().trafo()->dx() + 0.5);

						if (oTemp > 0)
						{
							xStart = oTemp;
						}
						else {
							xStart = 0;
						}
						xWidth = m_oDXSmall;

						//Der Filter(pipe) Eingang liefert den x Mittelpunkt des ROis:
						//also - wichtig: Variable xWidth erst verwenden, nachdem sie zum korrekten Wert gesetzt wurde !!
						if (xStart > (xWidth / 2))
						{
							xStart = xStart - xWidth / 2;
						}
						else{
							xStart = 0;
						}

						uInt xEnd = xStart + xWidth;
						uInt yStart(m_oY0);
						uInt yEnd(m_oDY + yStart);
						m_oRoi = Rect(Range(xStart, xEnd), Range(yStart, yEnd));
					}
				}
			}
			else {
				// vom Benutzer vorgegebene Werte verwenden
				xStart = m_oX0;
				xWidth = m_oDXLarge;

				uInt xEnd = xStart + xWidth;
				uInt yStart(m_oY0);
				uInt yEnd(m_oDY + yStart);
				m_oRoi = Rect(Range(xStart, xEnd), Range(yStart, yEnd));
			}

			// Operation des ROIs auf das Bild anwenden:
			auto oRoiCopy = m_oRoi;
			preSignalAction();
			m_oPipeSubImage.signal(createSubImage(oIn, oRoiCopy));
		}
	} // proceed


	/**
	 * extract subImage
	 * @param p_rIn source image
	 * @param p_rRoi rect at which to extract subimage
	 * @return frame with subImage of image at rect
	 */
	ImageFrame	TimedSimpleDynamicRoi::createSubImage(ImageFrame const& p_rIn, Rect & p_rRoi) {
		using namespace geo2d;

		// roi must fit into original image

		const Rect oInRoi ( Range( 0, p_rIn.data().size().width ),  Range( 0, p_rIn.data().size().height ) );
		p_rRoi = intersect(oInRoi, p_rRoi);
		if (!p_rRoi.isValid() || !p_rIn.data().isValid()) { return ImageFrame(p_rIn.context(), BImage());	} // if roi is invald return empty image

		// transformation from offset of curent image to offset of roi
		LinearTrafo oSubTrafo(offset(p_rRoi));
		// the new context is the old one with the modified(=concated) transformation
		ImageContext oNewContext( p_rIn.context(), oSubTrafo(p_rIn.context().trafo()) );
		// here the new image is created
		const BImage oNewImage (p_rIn.data(), p_rRoi);
		return ImageFrame(oNewContext, oNewImage, p_rIn.analysisResult());
	} // scale

	/*virtual*/ void TimedSimpleDynamicRoi::arm(const fliplib::ArmStateBase& state)
	{
		if (state.getStateID() == eSeamStart)
		{
			// get product information
			const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
			// get trigger delta
			m_oTriggerDelta = pProductData->m_oTriggerDelta;

			m_oRoi = Rect();  // Information initialisieren. Das ist notwendig, wenn wir im Zusammenhang mit dem Verwerf-Filter die Ausgabe unterdruecken wollen, und das ist der Fall.
		} // if

		if (m_oVerbosity >= eHigh)
		{
			wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(), state.getStateID());

		} // if

	} // arm

} // namespace filter
} // namespace precitec
