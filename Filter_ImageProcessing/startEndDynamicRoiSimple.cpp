/**
*  Filter_ImageProcessing::startEndDynamicRoiSimple.cpp
*	@file
*	@copyright      Precitec Vision GmbH & Co. KG
*	@author         OS
*	@date           04.2017
*	@brief			Filter to create subRoi-image from image and 2 double inputs denoting ROI
*/

#include "startEndDynamicRoiSimple.h"
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

StartEndDynamicRoiSimple::StartEndDynamicRoiSimple() : TransformFilter("StartEndDynamicRoiSimple", Poco::UUID{"D4F29731-E14D-4860-988A-2F82798BE487"}),
	m_pPipeImage(NULL),
	m_pPipeX(NULL),
	m_pPipeDx(NULL),
	m_pPipeStartEndInfo(NULL),
	m_oY0(0),
	m_oDY(0),
	m_oPipeSubImage(this, "SubFrame")	///< tag-name for identifying output (not used for single outputs)
{
	// add parameters to parameter list
	// Defaultwerte der Parameter setzen
	parameters_.add("roiY", Parameter::TYPE_int, static_cast<int>(m_oY0));
	parameters_.add("roiDy", Parameter::TYPE_int, static_cast<int>(m_oDY));

    setInPipeConnectors({{Poco::UUID("14A6C97B-1948-4296-96A1-AF3A9F790384"), m_pPipeImage, "ImageFrame", 1, "image"},
    {Poco::UUID("8B214A0A-36B8-4143-A5BF-D891C7D1ED34"), m_pPipeX, "roiX", 1, "roiX"},
    {Poco::UUID("D4BB9289-6C9C-4447-BD77-7184FC119A39"), m_pPipeDx, "roiDx", 1, "roiDx"},
    {Poco::UUID("5F09453D-EC5C-4A76-A296-8DB7035CF8A9"), m_pPipeStartEndInfo, "StartEndInfo", 1, "startEndInfo"}});
    setOutPipeConnectors({{Poco::UUID("89051C94-7091-4FE5-A954-C98C2EDBC54E"), &m_oPipeSubImage, "SubFrame", 0, "subFrame"}});
    setVariantID(Poco::UUID("D4BC503D-0EAB-401B-8FDD-E4890963D8E8"));
}

void StartEndDynamicRoiSimple::setParameter()
{
	// here BaseFilter sets the common parameters (currently verbosity)
	TransformFilter::setParameter();

	TransformFilter::setParameter();
	m_oY0 = static_cast<int>(parameters_.getParameter("roiY").convert<int>());
	m_oDY = static_cast<int>(parameters_.getParameter("roiDy").convert<int>());


} // setParameter


bool StartEndDynamicRoiSimple::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{

	if (p_rPipe.tag() == "image")
	{ // die tag Namen im sql SCript unter tag unter Connectir IDs Values eintragen
		m_pPipeImage = dynamic_cast<fliplib::SynchronePipe<ImageFrame> * >(&p_rPipe);
	}

	if (p_rPipe.tag() == "roiX")
	{
		m_pPipeX = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
	}

	if (p_rPipe.tag() == "roiDx")
	{
		m_pPipeDx = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
	}

	if (p_rPipe.tag() == "startEndInfo")
	{
		m_pPipeStartEndInfo = dynamic_cast<fliplib::SynchronePipe<GeoStartEndInfoarray> * >(&p_rPipe);
	}

	// herewith we set the proceed as callback (return is always true)
	return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe

void StartEndDynamicRoiSimple::paint()
{
	if (m_oVerbosity >= eLow)
	{
		if ((m_oRoi.width()>0) && (m_oRoi.height()>0))
		{
			OverlayCanvas				&rCanvas(canvas<OverlayCanvas>(m_oCounter));
			OverlayLayer				&rLayerLine(rCanvas.getLayerLine());

			rLayerLine.add(new OverlayRectangle(m_oRoi, Color::Green()));
		}
	}
}


void StartEndDynamicRoiSimple::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	// the arguments of this function are unclear??
	// here we check only for existence of the inputs, not their well-formedness
	if ((m_pPipeImage == nullptr)
		|| (m_pPipeX == nullptr)
		|| (m_pPipeDx == nullptr)
		|| (m_pPipeStartEndInfo == nullptr)
		)
	{
		// empty rect for drawing
        m_oRoi = Rect{0, 0, 0, 0};
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


		// check all incoming results for errors (AnalysisOK)
		interface::ResultType oRes = interface::AnalysisOK;

		if (m_pPipeX->read(m_oCounter).analysisResult() != interface::AnalysisOK)
		{
			oRes = m_pPipeX->read(m_oCounter).analysisResult();
		}
		else if (m_pPipeDx->read(m_oCounter).analysisResult() != interface::AnalysisOK)
		{
			oRes = m_pPipeDx->read(m_oCounter).analysisResult();
		}
		else if (m_pPipeStartEndInfo->read(m_oCounter).analysisResult() != interface::AnalysisOK)
		{
			oRes = m_pPipeDx->read(m_oCounter).analysisResult();
		}

		if (oRes != interface::AnalysisOK) // incorrect ROI -> forward image
		{
			ImageFrame oIf(m_pPipeImage->read(m_oCounter));
			oIf.setAnalysisResult(oRes);
			preSignalAction(); m_oPipeSubImage.signal(oIf);
			return;
		}
        auto oSpTrafoImageIn = oIn.context().trafo();

		const interface::GeoStartEndInfoarray &rGeoStartEndInfoArrayIn = m_pPipeStartEndInfo->read(m_oCounter);
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
                offsetY = oStartEndTrafo->dy() - oSpTrafoImageIn->dy();
            }
			startValidRangeY = oInInfo.m_oStartValidRangeY + offsetY;
            endValidRangeY = oInInfo.m_oEndValidRangeY + offsetY;
		}

		// trafo of image
		if (m_pPipeX->read(m_oCounter).ref().getRank()[0] > eRankMin)
		{
			// Wenn die Informationen ueber die X-Koordinate belastbar sind, dann berechne aus diesen Informationen
			// das ROI neu. Fuer den Fall ist auch die Transformation belastbar. Ferner wird die Transformation ausserhalb dieser
			// Zeilen nicht benutzt, also ist die lokal.
			//
			// Fuer den Fall, dass die Informationen nicht belastbar sind, bleibt das ROI so, wie es war.
			SmpTrafo pTrafo = m_pPipeX->read(m_oCounter).context().trafo();

			int oTemp = static_cast<int>(m_pPipeX->read(m_oCounter).ref().getData()[0] + pTrafo->dx() - oSpTrafoImageIn->dx() + 0.5);
			uInt xStart;
			if (oTemp > 0)
			{
				xStart = oTemp;
			}
			else{
				xStart = 0;
			}

			uInt xWidth(uInt(m_pPipeDx->read(m_oCounter).ref().getData()[0]));  //xE iat hiee ROI Breite

			//Der Filter(pipe) Eingang liefert den x Mittelpunkt des ROis:
			if (xStart > (xWidth / 2))
			{
				xStart = xStart - xWidth / 2;
			}
			else {
				// xStart ist so klein, dass es negativ werden wuerde, wenn wir davon jetzt noch was abziehen. Der Typ ist aber unsigned, also wollen wir das nicht machen.
				xStart = 0;
			}

			uInt xEnd = xStart + xWidth;

			int yStart = m_oY0;
			int yEnd = m_oDY + yStart;

			int newY1 = (yStart > startValidRangeY) ? yStart : startValidRangeY;
			int newY2 = (yEnd < endValidRangeY) ? yEnd : endValidRangeY;

			m_oRoi = Rect(Range(xStart, xEnd), Range(newY1, newY2));
		}

		// Die Verarbeitung des Bildes muss in jedem Fall erfolgen, auch und insbesondere dann, wenn die Information ueber die
		// X-Koordinate nicht belastbar war - das ist der Punkt.
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
ImageFrame	StartEndDynamicRoiSimple::createSubImage(ImageFrame const& p_rIn, Rect & p_rRoi)
{
	using namespace geo2d;

	interface::SmpTrafo oInTrafo = p_rIn.context().trafo();

	uInt x1 = oInTrafo->dx();
	uInt x2 = p_rIn.data().size().width;
	uInt y1 = oInTrafo->dy();
	uInt y2 = p_rIn.data().size().height;
	// std::cout<<"oInTrafo: "<<x1<<" "<<x2<<" "<<y1<<" "<<y2<<std::endl;


	// roi must fit into original image
	//const Rect oInRoi ( Range( 0, p_rIn.data().size().width ),  Range( 0, p_rIn.data().size().height ) );
	const Rect oInRoi(Range(x1, (x1 + x2)), Range(y1, (y1 + y2)));

	//std::cout<<"alter frame in createSubImg(inFrame) "<<oInRoi.width()<<" "<<oInRoi.height()<<std::endl;

	//liegt neuer ROi im alten ?
	p_rRoi = intersect(oInRoi, p_rRoi);

	if (!p_rRoi.isValid() || !p_rIn.data().isValid())
	{
#ifndef NDEBUG
		std::cout << " neuer Roi iegt nicht im alten..." << std::endl;
#endif
		return ImageFrame(p_rIn.context(), BImage());
	} // if roi is invald return empty image

	// transformation from offset of current image to offset of roi
	LinearTrafo oSubTrafo(offset(p_rRoi)); // jetzt gehts in die Hose...

	/*std::cout<<"create image roi(p_rRoi):  x:"<<p_rRoi.x().start()<<" xEnd: "<<p_rRoi.x().end()
	<<" y: "<<p_rRoi.y().start()<<" yEnd: "<<p_rRoi.y().end()
	<<" width: "<<p_rRoi.width()<<" height: "<<p_rRoi.height()<<std::endl;*/

	// the new context is the old one with the modified(=concated) transformation
	ImageContext oNewContext(p_rIn.context(), oSubTrafo(p_rIn.context().trafo()));


	// here the new image is created
	const BImage oNewImage(p_rIn.data(), p_rRoi);
	return ImageFrame(oNewContext, oNewImage, p_rIn.analysisResult());
} // scale

/*virtual*/ void StartEndDynamicRoiSimple::arm(const fliplib::ArmStateBase& state)
{
		if (state.getStateID() == eSeamStart)
		{
			// Fuer dieses Filter steht die ausgegebene Information direkt im ROI drin, d.h. das muss initialisiert werden.
			// Die Information zu ueberschreiben, ist in Ordnung, weil das am Anfang von proceedGroup() sowieso neu gesetzt wird.
			m_oRoi = Rect();
		}
} // arm


} // namespace filter
} // namespace precitec
