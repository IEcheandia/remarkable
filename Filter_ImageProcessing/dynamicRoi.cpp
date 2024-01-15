/**
 *  Filter_PoreAnalysis::dynamicRoi.cpp
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR), A. Beschorner (BA) [analysis Result]
 *	@date           19.10.2012
 *	@brief					<What's the purpose of the code in this file>
 */

#include "dynamicRoi.h"

#include "common/frame.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "common/geoContext.h"
#include "event/results.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>

#include <cstdint>
#include <cmath>
#include <vector>

void rotatedROI(const uint8_t* image, int height, int width, std::ptrdiff_t stride, uint8_t* dst, int roiTop, int roiLeft, int roiHeight, int roiWidth, double angle)
{
    const auto sine = std::sin(angle * M_PI / 180);
    const auto cosine = std::cos(angle * M_PI / 180);

    std::vector<double> ic(roiWidth);
    std::vector<double> is(roiWidth);

    for (int i = 0; i < roiWidth; ++i)
    {
        ic[i] = i * cosine;
        is[i] = i * sine;
    }

    for (int j = 0; j < roiHeight; ++j)
    {
        const auto jc = j * cosine;
        const auto js = j * sine;
        for (int i = 0; i < roiWidth; ++i)
        {
            const auto x = roiLeft + ic[i] - js;
            const auto y = roiTop + is[i] + jc;

            const auto x0 = static_cast<int>(x);
            const auto y0 = static_cast<int>(y);

            if (x0 < 0 || y0 < 0 || x0 >= width - 1 || y0 >= height - 1)
            {
                dst[j * roiWidth + i] = 0;
            }
            else
            {
                const int p = y0 * stride + x0;
                const auto p1 = image[p];
                const auto p2 = image[p + 1];
                const auto p3 = image[p + stride];
                const auto p4 = image[p + stride + 1];

                const auto tx = x - x0;
                const auto ty = y - y0;

                const auto a = p1 + tx * (p2 - p1);
                const auto b = p3 + tx * (p4 - p3);

                dst[j * roiWidth + i] = a + ty * (b - a);
            }
        }
    }
}

using fliplib::Parameter;

namespace precitec {
	using namespace interface;
	using namespace geo2d;
	using namespace image;
namespace filter {

	DynamicRoi::DynamicRoi()
	: TransformFilter("DynamicRoi", Poco::UUID{"6c49de27-bbeb-407a-bede-8b386d209383"}),
		m_pPipeImage(NULL),
		m_pPipeX(NULL),
		m_pPipeY(NULL),
		m_pPipeDx(NULL),
		m_pPipeDy(NULL),
        m_pipeAngle(nullptr),
		m_oColor(Color::Green()),
		m_oPipeSubImage(this, "SubFrame"),	///< tag-name for identifying output (not used for single outputs),
        m_angle(0.0),
		m_oSupportNestedROI (false) // false for back-compatibility
  {
		// add parameters to parameter list
		parameters_.add("rgbRed", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.red));
		parameters_.add("rgbGreen", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.green));
		parameters_.add("rgbBlue", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.blue));
		parameters_.add("rgbAlpha", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.alpha));
        parameters_.add("SupportNestedROI", Parameter::TYPE_bool, m_oSupportNestedROI );

        setInPipeConnectors({{Poco::UUID("68837aaa-72f5-4200-b34d-f7dbe75d3f3c"), m_pPipeImage, "ImageFrame", 1, "image"},
        {Poco::UUID("fe63d9b1-1d8e-4023-a36a-c29411b5f0e5"), m_pPipeX, "RoiX", 1, "roiX"},
        {Poco::UUID("3171bc6c-b583-44d8-9bea-9b3f95caece8"), m_pPipeY, "RoiY", 1, "roiY"},
        {Poco::UUID("1d45bcce-6608-4f23-9397-b84a729dfa34"), m_pPipeDx, "RoiDx", 1, "roiDx"},
        {Poco::UUID("0b597c6e-8f63-400b-acae-c4af2a2f9aa9"), m_pPipeDy, "RoiDy", 1, "roiDy"},
        {Poco::UUID("1a04e922-4d9e-4f4b-8f86-79e11b207b6e"), m_pipeAngle, "OptionalAngle", 1, "OptionalAngle", fliplib::PipeConnector::ConnectionType::Optional}
        });
        setOutPipeConnectors({{Poco::UUID("e747d355-e45a-4dd4-9efa-59a443e36a85"), &m_oPipeSubImage, "SubFrame", 0, "subFrame"}});
        setVariantID(Poco::UUID("78ab07b0-913e-4c3a-ab5f-ca7cef722ef9"));
	}

	void DynamicRoi::setParameter() {
		// here BaseFilter sets the common parameters (currently verbosity)
		TransformFilter::setParameter();

		m_oColor.red = parameters_.getParameter("rgbRed").convert<byte>();
		m_oColor.green = parameters_.getParameter("rgbGreen").convert<byte>();
		m_oColor.blue = parameters_.getParameter("rgbBlue").convert<byte>();
		m_oColor.alpha = parameters_.getParameter("rgbAlpha").convert<byte>();
        m_oSupportNestedROI = parameters_.getParameter("SupportNestedROI").convert<bool>();
	} // setParameter


	bool DynamicRoi::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {

		if (p_rPipe.tag() == "image") {
			m_pPipeImage = dynamic_cast<fliplib::SynchronePipe<ImageFrame> * >(&p_rPipe);
		}

		if (p_rPipe.tag() == "roiX") {
			m_pPipeX = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
		}
		if (p_rPipe.tag() == "roiY") {
			m_pPipeY = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
		}
		if (p_rPipe.tag() == "roiDx") {
			m_pPipeDx = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
		}
		if (p_rPipe.tag() == "roiDy") {
			m_pPipeDy = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
		}
        if (p_rPipe.tag() == "OptionalAngle")
        {
            m_pipeAngle = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(&p_rPipe);
        }
		// herewith we set the proceed as callback (return is always true)
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	} // subscribe

	void DynamicRoi::paint() {

		if ( m_pTrafo.isNull() )
		{
			return;
		}

		if(m_oVerbosity >= eLow) {
			if (!m_oRoi.isEmpty())  {
				Trafo						&rTrafo					( *m_pTrafo );
				OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
				OverlayLayer				&rLayerLine				( rCanvas.getLayerLine());

				// draw final roi
                rLayerLine.add(new OverlayRectangle(rTrafo(m_oRoi), m_oColor));

                if (m_pipeAngle != nullptr)
                {
                    const auto cosine = std::cos(m_angle * M_PI / 180);
                    const auto sine = std::sin(m_angle * M_PI / 180);
                    const auto topLeft = rTrafo(m_oRoi.offset());
                    const auto topRight = (geo2d::Point(topLeft.x + m_oRoi.width() * cosine, topLeft.y + m_oRoi.width() * sine));
                    const auto bottomLeft = (geo2d::Point(topLeft.x - m_oRoi.height() * sine, topLeft.y + m_oRoi.height() * cosine));
                    const auto bottomRight = (geo2d::Point(topLeft.x + m_oRoi.width() * cosine - m_oRoi.height() * sine, topLeft.y + m_oRoi.width() * sine + m_oRoi.height() * cosine));

                    rLayerLine.add<image::OverlayImage>(topLeft, m_image, image::OverlayText());
                    rLayerLine.add<image::OverlayLine>(topLeft, topRight, m_oColor);
                    rLayerLine.add<image::OverlayLine>(topRight, bottomRight, m_oColor);
                    rLayerLine.add<image::OverlayLine>(bottomRight, bottomLeft, m_oColor);
                    rLayerLine.add<image::OverlayLine>(bottomLeft, topLeft, m_oColor);
                    rLayerLine.add<image::OverlayCircle>(topLeft, 5, m_oColor);
                }
			}
		}
	}


	void DynamicRoi::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
	{
		m_badInput = false;
		// needs to be at the very top to make sure paint will not be called when errors occur!
		m_oRoi = Rect();

		// the arguments of this function are unclear??
		// here we check only for existence of the inputs, not their well-formedness
		if (   (m_pPipeImage==nullptr)
				|| (m_pPipeX==nullptr)
				|| (m_pPipeY==nullptr)
				|| (m_pPipeDx==nullptr)
				|| (m_pPipeDy==nullptr)
				)
		{
			// empty rect for drawing

			// signal error output == empty image
			ImageFrame oIf;
			oIf.setAnalysisResult(interface::AnalysisErrBadImage);
			preSignalAction(); m_oPipeSubImage.signal(oIf);
		}
		else
		{
			// connect input variable to pipe content
			/// scalar input value(s)
			ImageFrame const& oIn(m_pPipeImage->read(m_oCounter));
			// the 4 int values are packaged as SynchronePipe<GeoT<TArray<double>>>
			// so we
			//     'read()' to get at the geo-object
			//     'ref()' to get at the Arraydouble
			//     'getData()' to get at the std::array<double>
			//     '[0]' to get at the first (and only interesting value)
			//     'uInt()' to cast from double to uInt
			// then we combine them (left top, with, height) into a rectangle

			// check all incoming results for errors (AnalysisOK)
			interface::ResultType oRes = interface::AnalysisOK;

			if (m_pPipeX->read(m_oCounter).analysisResult() != interface::AnalysisOK)
			{
				oRes = m_pPipeX->read(m_oCounter).analysisResult();
			}
			else if (m_pPipeY->read(m_oCounter).analysisResult() != interface::AnalysisOK)
			{
				oRes = m_pPipeY->read(m_oCounter).analysisResult();
			}
			else if (m_pPipeDx->read(m_oCounter).analysisResult() != interface::AnalysisOK)
			{
				oRes = m_pPipeDx->read(m_oCounter).analysisResult();
			}
			else if (m_pPipeDy->read(m_oCounter).analysisResult() != interface::AnalysisOK)
			{
				oRes = m_pPipeDy->read(m_oCounter).analysisResult();
			}
			if (oRes != interface::AnalysisOK) // incorrect ROI -> forward image
			{
				ImageFrame oIf(m_pPipeImage->read(m_oCounter));
				oIf.setAnalysisResult(oRes);
				preSignalAction(); m_oPipeSubImage.signal(oIf);
				return;
			}

			bool oInputRankOkay = m_pPipeX->read(m_oCounter).ref().getRank()[0] > eRankMin &&
				m_pPipeDx->read(m_oCounter).ref().getRank()[0] > eRankMin &&
				m_pPipeY->read(m_oCounter).ref().getRank()[0] > eRankMin &&
				m_pPipeDy->read(m_oCounter).ref().getRank()[0] > eRankMin;

			// ROI und Transformation nur dann aktualisieren, wenn Inputdaten okay sind:
			if (oInputRankOkay)
			{
                m_pTrafo = oIn.context().trafo();
				// all ROI data valid, so proceed
				int oXStart =   int(m_pPipeX->read(m_oCounter).ref().getData()[0] + m_pPipeX->read(m_oCounter).context().trafo()->dx() + 0.5);   // Transformation beruecksichtigen, wichtig
                if ( m_oSupportNestedROI )
                {
                    oXStart -= m_pTrafo->dx();
                }
				if (oXStart < 0)
				{
					oXStart = 0;
				}

				int oWidth =   int(m_pPipeDx->read(m_oCounter).ref().getData()[0] + 0.5);  // Die Transformation wird bezueglich der Breite nicht beruecksichtigt - Breite ist eine relative Groesse.
				if (oWidth < 0)
				{
					oWidth = 0;
				}
				int oXEnd = oWidth + oXStart;

				int oYStart =   int(m_pPipeY->read(m_oCounter).ref().getData()[0] + m_pPipeY->read(m_oCounter).context().trafo()->dy() + 0.5);     // Transformation beruecksichtigen und zwar die Y-Komponente, wichtig
                if ( m_oSupportNestedROI )
                {
                    oYStart -= m_pTrafo->dy();
                }
				if (oYStart < 0)
				{
					oYStart = 0;
				}

				int oHeight =   int(m_pPipeDy->read(m_oCounter).ref().getData()[0] + 0.5);
				if (oHeight < 0)
				{
					oHeight = 0;
				}
				int oYEnd = oHeight + oYStart;

				m_oRoi = Rect(Range(oXStart, oXEnd), Range(oYStart, oYEnd));
                m_angle = 0.0;

                //handle optional angleStart pipe if it is connected
                if (m_pipeAngle != nullptr)
                {
                    const auto &geoAngle = m_pipeAngle->read(m_oCounter);
                    m_angle = geoAngle.ref().getData().front();
                }
			}

			// do the actual processing
			// signal output -> allow other filters to process result
			auto outRoi = createSubImage(oIn, m_oRoi, m_angle);
			preSignalAction();
			m_oPipeSubImage.signal(outRoi);
		}
	} // proceed


	/**
	 * extract subImage
	 * @param p_rIn source image
	 * @param p_rRoi rect at which to extract subimage
	 * @return frame with subImage of image at rect
	 */
	ImageFrame	DynamicRoi::createSubImage(ImageFrame const& p_rIn, Rect & p_rRoi, double angle) {
		using namespace geo2d;

        const auto imageIn = p_rIn.data();

		// transformation from offset of curent image to offset of roi
		LinearTrafo oSubTrafo(offset(p_rRoi));
		// the new context is the old one with the modified(=concated) transformation
		ImageContext oNewContext( p_rIn.context(), oSubTrafo(p_rIn.context().trafo()) );

        if (m_pipeAngle == nullptr)
        {
            // roi must fit into original image
            const Rect oInRoi ( Range( 0, p_rIn.data().size().width ),  Range( 0, p_rIn.data().size().height ) );
            p_rRoi = intersect(oInRoi, p_rRoi);

            if (p_rRoi.isEmpty() || !p_rIn.data().isValid()) {return ImageFrame(p_rIn.context(), BImage());} // if roi is invald return empty image

            m_image = BImage(imageIn, p_rRoi);
        }
        else
        {
            m_image.resize(image::Size2d{p_rRoi.width(), p_rRoi.height()});

            rotatedROI(imageIn.begin(), imageIn.height(), imageIn.width(), imageIn.stride(), m_image.begin(), p_rRoi.y().start(), p_rRoi.x().start(), p_rRoi.height(), p_rRoi.width(), angle);
        }

		return ImageFrame(oNewContext, m_image, p_rIn.analysisResult());
	} // scale


} // namespace filter
} // namespace precitec
