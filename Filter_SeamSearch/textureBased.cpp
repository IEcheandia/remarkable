/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter finds the seam texture based
*/

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "textureBased.h"
#include "textureBasedMeritFct.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {


		SFTB_SeamFinderTextureBasedT SFTB_SeamFinderTextureBased;
		SFTB_SeamQualityT SFTB_SeamQuality;
		SFTB_RF_CalculatorT  SFTB_RF_Calculator;
		SFTB_RF_CalculatorLx1M2_Ly1M1_Lx2P2_Ly2M1T  SFTB_RF_CalculatorLx1M2_Ly1M1_Lx2P2_Ly2M1;
		SFTB_RF_CalculatorLx1M4_Ly1M4_Lx2P4_Ly2M4T  SFTB_RF_CalculatorLx1M4_Ly1M4_Lx2P4_Ly2M4;
		SFTB_RF_CalculatorLx1M4_Ly1P4_Lx2P4_Ly2P4T  SFTB_RF_CalculatorLx1M4_Ly1P4_Lx2P4_Ly2P4;
		SFTB_ParametersT param;

		const std::string SeamFindTextureBased::m_oFilterName = std::string("SeamFindTextureBased");
		const std::string SeamFindTextureBased::PIPENAME_SEAMPOS_OUT = std::string("SeamPositionOut");
		const std::string SeamFindTextureBased::PIPENAME_SEAMLEFT_OUT = std::string("SeamLeftOut");
		const std::string SeamFindTextureBased::PIPENAME_SEAMRIGHT_OUT = std::string("SeamRightOut");
		const std::string SeamFindTextureBased::PIPENAME_SEAMFINDINGS_OUT = std::string("SeamFindingOut");

		SeamFindTextureBased::SeamFindTextureBased() :
			TransformFilter(SeamFindTextureBased::m_oFilterName, Poco::UUID{"C411A449-373B-4B74-8A91-A8011A32A4E6"}),
			m_pPipeInImageFrame(NULL), m_pPipeInSeamFindings(NULL)
		{
			m_pPipeOutSeamPos = new SynchronePipe< interface::GeoDoublearray >(this, SeamFindTextureBased::PIPENAME_SEAMPOS_OUT);
			m_pPipeOutSeamLeft = new SynchronePipe< interface::GeoDoublearray >(this, SeamFindTextureBased::PIPENAME_SEAMLEFT_OUT);
			m_pPipeOutSeamRight = new SynchronePipe< interface::GeoDoublearray >(this, SeamFindTextureBased::PIPENAME_SEAMRIGHT_OUT);
			m_pPipeOutSeamFindings = new SynchronePipe< interface::GeoSeamFindingarray >(this, SeamFindTextureBased::PIPENAME_SEAMFINDINGS_OUT);

			// Set default values of the parameters of the filter
			parameters_.add("ThreshProj", Parameter::TYPE_int, m_oThreshProj);
			parameters_.add("ThreshSingleVal", Parameter::TYPE_int, m_oThreshSingleVal);
			parameters_.add("SubX", Parameter::TYPE_int, m_oSubX);
			parameters_.add("SubY", Parameter::TYPE_int, m_oSubY);
			parameters_.add("MaxSeamCenterWidth", Parameter::TYPE_int, m_oMaxSeamCenterWidth);
			parameters_.add("MinSeamWidth", Parameter::TYPE_int, m_oMinSeamWidth);
			parameters_.add("Lx1", Parameter::TYPE_int, m_oLx1);
			parameters_.add("Ly1", Parameter::TYPE_int, m_oLy1);
			parameters_.add("Lx2", Parameter::TYPE_int, m_oLx2);
			parameters_.add("Ly2", Parameter::TYPE_int, m_oLy2);

            setInPipeConnectors({{Poco::UUID("38731F11-A220-4048-97AC-5D3EA1818CEC"), m_pPipeInImageFrame, "ImageFrame", 1, "ImageFrame"},
            {Poco::UUID("F34E6821-27D9-40F4-A7DB-3C1CCE91608E"), m_pPipeInSeamFindings, "SeamFinding", 1, "SeamFinding"}});
            setOutPipeConnectors({{Poco::UUID("DE472260-418E-4E46-A8FC-83843F3A6CFB"), m_pPipeOutSeamPos, PIPENAME_SEAMPOS_OUT, 0, ""},
            {Poco::UUID("D8068018-B4A6-45A9-BB56-17A0B4CDDC54"), m_pPipeOutSeamLeft, PIPENAME_SEAMLEFT_OUT, 0, ""},
            {Poco::UUID("624D9F5A-C128-4288-8841-B3DF4287B69C"), m_pPipeOutSeamRight, PIPENAME_SEAMRIGHT_OUT, 0, ""},
            {Poco::UUID("7EACE747-B265-473C-A5DB-FE34A1CB2A61"), m_pPipeOutSeamFindings, PIPENAME_SEAMFINDINGS_OUT, 0, ""}});
		} // LineProfile

		SeamFindTextureBased::~SeamFindTextureBased()
		{
			delete m_pPipeOutSeamPos;
			delete m_pPipeOutSeamLeft;
			delete m_pPipeOutSeamRight;
			delete m_pPipeOutSeamFindings;
		} // ~LineProfile

		void SeamFindTextureBased::setParameter()
		{
			TransformFilter::setParameter();
			param.ImfWantedThreshold				= parameters_.getParameter("ThreshProj").convert<int>();
			param.ImfWantedSinglevalueThreshold		= parameters_.getParameter("ThreshSingleVal").convert<int>();
			param.GridStepsizeX						= parameters_.getParameter("SubX").convert<int>();
			param.GridStepsizeY						= parameters_.getParameter("SubY").convert<int>();
			param.MaxGapBridge						= parameters_.getParameter("MaxSeamCenterWidth").convert<int>();
			param.MinSeamWidth						= parameters_.getParameter("MinSeamWidth").convert<int>();
			param.RF_Lx1							= parameters_.getParameter("Lx1").convert<int>();
			param.RF_Ly1							= parameters_.getParameter("Ly1").convert<int>();
			param.RF_Lx2							= parameters_.getParameter("Lx2").convert<int>();
			param.RF_Ly2							= parameters_.getParameter("Ly2").convert<int>();

			setRF_CalcSelect(param);
		} // setParameter

		bool SeamFindTextureBased::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			m_pPipeInSeamFindings = nullptr;
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);
			if (p_rPipe.type() == typeid(GeoSeamFindingarray))
				m_pPipeInSeamFindings = dynamic_cast< SynchronePipe < GeoSeamFindingarray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void SeamFindTextureBased::paint()
		{
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
			{
				return;
			} // if

			try
			{
				const Trafo		&rTrafo(*m_oSpTrafo);
				OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
				OverlayLayer	&rLayerContour(rCanvas.getLayerContour());
				//OverlayLayer	&rLayerImage(rCanvas.getLayerImage());


				if ((m_resultSeamLeft != 0) && (m_resultSeamRight != 0) && (m_resultSeamPosY != 0))
				{
					rLayerContour.add(new OverlayCross(rTrafo(Point(m_resultSeamLeft, m_resultSeamPosY)), 20, Color::Red()));
					rLayerContour.add(new OverlayCross(rTrafo(Point(m_resultSeamLeft - 1, m_resultSeamPosY)), 20, Color::Red()));
					rLayerContour.add(new OverlayCross(rTrafo(Point(m_resultSeamRight, m_resultSeamPosY)), 20, Color::Green()));
					rLayerContour.add(new OverlayCross(rTrafo(Point(m_resultSeamRight + 1, m_resultSeamPosY)), 20, Color::Green()));
				}

			}
			catch (...)
			{
				return;
			}
		} // paint

		void SeamFindTextureBased::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
			//poco_assert_dbg(m_pPipeInSeamFindings != nullptr);

			m_resultSeamLeft = m_resultSeamRight = m_resultSeamPosX = m_resultSeamPosY = 0;

			// Read out image frame from pipe
			const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);

			m_oSpTrafo = rFrameIn.context().trafo();

			const BImage	&rImageIn = rFrameIn.data();
			const Size2d	oSizeImgIn = rImageIn.size();

			// input validity check
			if (rImageIn.isValid() == false) {
				//ImageFrame oNewFrame(rFrameIn.context(), BImage(), oAnalysisResult);	// signal null image
				preSignalAction();

				return; // RETURN
			}

			// for image pointer work new image array (Souvis style)
			const byte *row;
			byte *bImageNew = new byte[oSizeImgIn.width * oSizeImgIn.height];
			byte * bImgOrg = bImageNew;

			for (int i = 0; i < oSizeImgIn.height; i++)
			{
				row = rImageIn[i];
				memcpy(bImageNew, row, oSizeImgIn.width);
				bImageNew += oSizeImgIn.width;
			}

			SeamFindingarray seamFindingArray;
			if (m_pPipeInSeamFindings != nullptr)
			{
				const GeoSeamFindingarray& rSeamFindingsIn = m_pPipeInSeamFindings->read(m_oCounter);
				seamFindingArray = rSeamFindingsIn.ref();
			}
			else
			{
				seamFindingArray.getData().push_back(SeamFinding(0, 0, 0, 0));
				seamFindingArray.getRank().push_back(0);
			}

			geo2d::Doublearray oOutSeamPos;
			geo2d::Doublearray oOutSeamLeft;
			geo2d::Doublearray oOutSeamRight;
			geo2d::SeamFindingarray oOutSeamFindings = seamFindingArray;

			if (0)
			{
				const GeoDoublearray &rSeamPos = GeoDoublearray(rFrameIn.context(), m_oSeamPosOut, rFrameIn.analysisResult(), interface::NotPresent);
				const GeoDoublearray &rSeamLeft = GeoDoublearray(rFrameIn.context(), m_oSeamLeftOut, rFrameIn.analysisResult(), interface::NotPresent);
				const GeoDoublearray &rSeamRight = GeoDoublearray(rFrameIn.context(), m_oSeamRightOut, rFrameIn.analysisResult(), interface::NotPresent);
				const GeoSeamFindingarray &rSeamFindings = GeoSeamFindingarray(rFrameIn.context(), oOutSeamFindings, rFrameIn.analysisResult(), interface::NotPresent);
				preSignalAction();
				m_pPipeOutSeamPos->signal(rSeamPos);
				m_pPipeOutSeamLeft->signal(rSeamLeft);
				m_pPipeOutSeamRight->signal(rSeamRight);
				m_pPipeOutSeamFindings->signal(rSeamFindings);

				delete[] bImgOrg;
				return; // RETURN
			}

			// Now do the actual image processing
			searchSeamTextureBased(bImgOrg, oSizeImgIn, oOutSeamPos, oOutSeamLeft, oOutSeamRight, oOutSeamFindings);

			// Create a new byte array, and put the global context into the resulting profile
			const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
			const GeoDoublearray &rGeoSeamPos = GeoDoublearray(rFrameIn.context(), oOutSeamPos, oAnalysisResult, filter::eRankMax);
			const GeoDoublearray &rGeoSeamLeft = GeoDoublearray(rFrameIn.context(), oOutSeamLeft, oAnalysisResult, filter::eRankMax);
			const GeoDoublearray &rGeoSeamRight = GeoDoublearray(rFrameIn.context(), oOutSeamRight, oAnalysisResult, filter::eRankMax);
			const GeoSeamFindingarray &rSeamFindings = GeoSeamFindingarray(rFrameIn.context(), oOutSeamFindings, rFrameIn.analysisResult(), interface::NotPresent);

			preSignalAction();

			m_pPipeOutSeamPos->signal(rGeoSeamPos);
			m_pPipeOutSeamLeft->signal(rGeoSeamLeft);
			m_pPipeOutSeamRight->signal(rGeoSeamRight);
			m_pPipeOutSeamFindings->signal(rSeamFindings);

			bImageNew = 0;
			delete[] bImgOrg;
		} // proceedGroup

		void SeamFindTextureBased::searchSeamTextureBased(const byte *bImageNew, const Size2d size,
			Doublearray & p_rSeamPosOut, Doublearray & p_rSeamLeftOut, Doublearray & p_rSeamRightOut, SeamFindingarray & seamFindingArrayOut)
		{
			try
			{
				SFTB_roiT SFTB_roi;

				SFTB_roi.start = bImageNew;
				SFTB_roi.pitch = size.width;
				SFTB_roi.dx = size.width;
				SFTB_roi.dy = size.height;

//				if (pimageShMem.DebugLevelS2Insp>8)
//				{
//					SFTB_SeamFinderTextureBased.printparams();
//				}

				SFTB_SeamFinderTextureBased.GetSeamPositions(SFTB_roi);

				m_resultSeamLeft = SFTB_SeamFinderTextureBased.MeasurementArray[0].xL + SFTB_SeamFinderTextureBased.RF_Calc->templatesizeX / 2;
				m_resultSeamRight = SFTB_SeamFinderTextureBased.MeasurementArray[0].xR + SFTB_SeamFinderTextureBased.RF_Calc->templatesizeX / 2;//
				m_resultSeamPosY = SFTB_SeamFinderTextureBased.MeasurementArray[0].y + SFTB_SeamFinderTextureBased.RF_Calc->templatesizeY / 2;

				m_resultSeamPosX = m_resultSeamLeft + (m_resultSeamRight - m_resultSeamLeft) / 2;
				int rank = SFTB_SeamFinderTextureBased.MeasurementArray[0].rank;

				p_rSeamPosOut.getData().push_back(m_resultSeamPosX);
				p_rSeamPosOut.getRank().push_back(rank);
				p_rSeamLeftOut.getData().push_back(m_resultSeamLeft);
				p_rSeamLeftOut.getRank().push_back(rank);
				p_rSeamRightOut.getData().push_back(m_resultSeamRight);
				p_rSeamRightOut.getRank().push_back(rank);

//				if (pimageShMem.DebugLevelS2Insp > 8)
//				{
//					printf("untersuchtes ROI: x0=%d  y0=%d   dx=%d  dy=%d \n", seamFindRoi.x_.start_, seamFindRoi.y_.start_, SFTB_roi.dx, SFTB_roi.dy);
//					printf("Naht gefunden bei: %d,  %d\n", xL, xR);
//					printf("y0t=%d  rank=%d \n", y0t, (int)seamSinglePos.rank_);
//					printf("error=%d \n", SFTB_SeamFinderTextureBased.MeasurementArray[0].errorflag);
//				}

			}
			catch (...)
			{
				p_rSeamPosOut.getData().push_back(0);
				p_rSeamPosOut.getRank().push_back(0);
				p_rSeamLeftOut.getData().push_back(0);
				p_rSeamLeftOut.getRank().push_back(0);
				p_rSeamRightOut.getData().push_back(0);
				p_rSeamRightOut.getRank().push_back(0);
			}
		} // searchSeamTextureBased


		int SeamFindTextureBased::setRF_CalcSelect(SFTB_ParametersT & param)
		{
			// Auswahl f() abhaengig nach Parameter-Einstellungen
			// Muss jetzt erst bestimmen, welche Parameter wie gesetzt sind

			// Pruefe: Lx1=-4 Ly1= 4 Lx2=4 Ly2= 4  =>  Mubea Kamera verdreht (y gespiegelt)  =>  Index 2
			if ((param.RF_Lx1 == -4)
				&& (param.RF_Ly1 == 4)
				&& (param.RF_Lx2 == 4)
				&& (param.RF_Ly2 == 4)
				)
			{
				// Funktion updaten
				param.RF_CalculatorArrayWantedSlot = 2;
			}

			// Pruefe: Lx1=-2 Ly1= 1 Lx2=2 Ly2= 1  =>  Yudigar y gespiegelt  =>  Index 3
			else if ((param.RF_Lx1 == -2)
				&& (param.RF_Ly1 == 1)
				&& (param.RF_Lx2 == 2)
				&& (param.RF_Ly2 == 1)
				)
			{
				// Funktion updaten
				param.RF_CalculatorArrayWantedSlot = 3;
			}

			// Nicht passende Param!! oder Lx1=-4 Ly1=-4 Lx2=4 Ly2=-4
			// Setze: Lx1=-4 Ly1=-4 Lx2=4 Ly2=-4  =>  Mubea normal  =>  Index 1
			else if ((param.RF_Lx1 == -4)
				&& (param.RF_Ly1 == -4)
				&& (param.RF_Lx2 == 4)
				&& (param.RF_Ly2 == 4)
				)
			{
				// Funktion updaten
				param.RF_CalculatorArrayWantedSlot = 1;
			}
			else
			{
				// Funktion updaten
				param.RF_CalculatorArrayWantedSlot = 0;
			}

			SFTB_SeamFinderTextureBased.RF_Calc->setCoefficients(param.RF_Lx1, param.RF_Ly1, param.RF_Lx2, param.RF_Ly2);

			return 0;
		}

		//*******************************************************************************************************************************************************
		//*******************************************************************************************************************************************************

		SFTB_SeamFinderTextureBasedT::SFTB_SeamFinderTextureBasedT()
		{

			NumberOfMeasurements = 0;

			RF_Calc = &SFTB_RF_Calculator; //wird durch setRF_Calc spaeter ueberschrieben: sicherheitshalber vorbelegen

			RF_CalculatorArrayCurrentSlot = -1; //kein sinnvoller Wert: setRF_Calc(param.RF_CalculatorArrayWantedSlot) muss auf jeden Fall updaten

			RF_CalculatorArray[0] = &SFTB_RF_Calculator;
			RF_CalculatorArray[1] = &SFTB_RF_CalculatorLx1M4_Ly1M4_Lx2P4_Ly2M4;//Lx1=-4 Ly1=-4 Lx2=4 Ly2=-4;//normal
			RF_CalculatorArray[2] = &SFTB_RF_CalculatorLx1M4_Ly1P4_Lx2P4_Ly2P4;//Lx1=-4 Ly1= 4 Lx2=4 Ly2= 4;//Kamera y gespiegelt
			RF_CalculatorArray[3] = &SFTB_RF_CalculatorLx1M2_Ly1M1_Lx2P2_Ly2M1;//Lx1=-2 Ly1= 1 Lx2=2 Ly2= 1;// Versuch Yudigar

		}


		void SFTB_SeamFinderTextureBasedT::GetSeamPositions(SFTB_roiT & roi)
		{

			signed short ycenter;
			int ret, linenumber, nSpots1, nSpots2;
			int xLraw, xRraw, xL, xR;
			unsigned char rank, quality2;

			linenumber = 0;

#ifdef  SFTBCONSOLEDEBUG
	int i;
	printparams();
#endif


			MeasurementArray[linenumber].xL = 0; //Vorbelegung=> definierter Rueckgabewert
			MeasurementArray[linenumber].xR = 0; //Vorbelegung=> definierter Rueckgabewert

			//Thresholds aus Parametern int thresholds berechnen
			CalculateThresholds(param.ImfWantedSinglevalueThreshold, param.ImfWantedThreshold);

			//Alle Meritfunction Werte auf Grid berechnen, y0t: Mitte des Auswertebereichs zurueckgeben
			CalculateGridValues(roi, ycenter);

			MeasurementArray[linenumber].y = ycenter;

			//Werte auf Grid zu Lines zusammenfassen und jede Line auswerten
			linenumber = 0;

#ifdef  SFTBCONSOLEDEBUG
	SFTBWINCONENABLE(mcon.cyan();)
	printf("********************************************************************\n");
	printf("Line no.:%d  y=%d\n", linenumber + 1, ycenter);
	SFTBWINCONENABLE(mcon.white();)
#endif

			CalculateLineValues(roi); //alle Werte des Grids zu einer Zeile zusammenfuegen

			ret = CalculateLineSections(linearray1, roi.pitch, MF1LineDistributionParameter, spots1, nSpots1); //State machine fuer die Zeile anwenden
			if (ret != 0 || nSpots1 == 0)
			{
#ifdef SFTBCONSOLEDEBUG
	SFTBWINCONENABLE(mcon.red();)
	printf("Line evaluation 1 did not succeed\n");
	SFTBWINCONENABLE(mcon.white();)
#endif
				MeasurementArray[linenumber].errorflag = 1;
				MeasurementArray[linenumber].rank = 0;
				return;  //next line
			}

			ret = CalculateLineSections(linearray2, roi.pitch, MF2LineDistributionParameter, spots2, nSpots2); //State machine fuer die Zeile anwenden
			if (ret != 0 || nSpots2 == 0)
			{
#ifdef SFTBCONSOLEDEBUG
	SFTBWINCONENABLE(mcon.red();)
	printf("Line evaluation 2 did not succeed\n");
	SFTBWINCONENABLE(mcon.white();)
#endif
				MeasurementArray[linenumber].errorflag = 1;
				MeasurementArray[linenumber].rank = 0;
				return; //next line
			}

#ifdef SFTBCONSOLEDEBUG
	for (i = 0; i<nSpots1; ++i) printf("1 spot:%d xL=%d  xR=%d sLen=%d  onspotlength=%d  spotMaxW=%g  spotSumW=%g\n", i, spots1[i].xL, spots1[i].xR, spots1[i].spotlength, spots1[i].onspotlength, spots1[i].spotMaxW, spots1[i].spotSumW);
	for (i = 0; i<nSpots2; ++i) printf("2 spot:%d xL=%d  xR=%d sLen=%d  onspotlength=%d  spotMaxW=%g  spotSumW=%g\n", i, spots2[i].xL, spots2[i].xR, spots2[i].spotlength, spots2[i].onspotlength, spots2[i].spotMaxW, spots2[i].spotSumW);
#endif

			LineSectionEvaluation(spots1, nSpots1, spots2, nSpots2, xLraw, xRraw, quality2); //Zeilenabschitte zu Nahtrandpositionen verarbeiten

			//Zeile erfolgreich ausgewertet
#ifdef SFTBCONSOLEDEBUG
	printf("y=%d  xLraw=%d  xRraw=%d quality2=%d\n", ycenter, xLraw, xRraw, quality2);
#endif
			//Transform subsamling coordinates into roi coordinates
			xL = xLraw*param.GridStepsizeX;
			xR = xRraw*param.GridStepsizeX;

			//Calculate rank from qualities:
			rank = quality2;

#ifdef SFTBCONSOLEDEBUG
	printf("y=%d  xr=%d  xl=%d \n", ycenter, xL, xR);
#endif

			MeasurementArray[linenumber].errorflag = 0;
			MeasurementArray[linenumber].xL = xL;
			MeasurementArray[linenumber].xR = xR;
			MeasurementArray[linenumber].y = ycenter;
			MeasurementArray[linenumber].rank = rank;

			NumberOfMeasurements = 1;

		}

		//float thresholds aus parametern int thresholds berechnen
		void  SFTB_SeamFinderTextureBasedT::CalculateThresholds(int iWantedmfSinglevalueThreshold, int iWantedmfThreshold)
		{

			// Threshold fuer Meritfunction (Einzel-)Wert:  threshold = average * fUsedmfSinglevalueThreshold
			//aus Parametern:  int iWantedmfSinglevalueThreshold;
			fUsedmfSinglevalueThreshold = 1.0e-2f *iWantedmfSinglevalueThreshold;

			// Threshold fuer summierte Meritfunction:  threshold = maximalwert * fUsedmfThreshold + average *(1-fUsedmfThreshold)
			//aus Parametern:   int iWantedmfThreshold;
			fUsedmfThreshold = 1.0e-4f *iWantedmfSinglevalueThreshold;
		}


		void SFTB_SeamFinderTextureBasedT::printparams(void)
		{
			printf("ImfWantedThreshold=%d  %g\n", param.ImfWantedThreshold, 1e-4*param.ImfWantedThreshold);
			printf("ImfWantedSinglevalueThreshold=%d   %g\n", param.ImfWantedSinglevalueThreshold, 1e-2*param.ImfWantedSinglevalueThreshold);
			printf("GridStepsizeX=%d\n", param.GridStepsizeX);
			printf("GridStepsizeY=%d\n", param.GridStepsizeY);
			printf("MaxGapBridge=%d\n", param.MaxGapBridge);
			printf("MinSeamWidth=%d\n", param.MinSeamWidth);

			printf("RF_CalculatorArrayWantedSlot=%d\n", param.RF_CalculatorArrayWantedSlot);
			printf("RF_Lx1=%d\n", param.RF_Lx1);
			printf("RF_Ly1=%d\n", param.RF_Ly1);
			printf("RF_Lx2=%d\n", param.RF_Lx2);
			printf("RF_Ly2=%d\n", param.RF_Ly2);

		}


		void  SFTB_SeamFinderTextureBasedT::CalculateGridValues(SFTB_roiT & roi, signed short & ycenter)
		{
			unsigned long ngrid;
			signed short y0t, x0t, ymax;
			float rf1, rf2;
			SFTB_roiT roitemplate;

			//Alle Werte auf Grid berechnen
			ngrid = 0;

			roitemplate.dx = roi.dx;
			roitemplate.dy = roi.dy;
			roitemplate.pitch = roi.pitch;

			MF1LineDistributionParameter.GridMaxW = MF2LineDistributionParameter.GridMaxW = 0.0;
			MF1LineDistributionParameter.GridAveW = MF2LineDistributionParameter.GridAveW = 0.0;

			ymax = roi.dy - RF_Calc->templatesizeY; //sloppy berechnet: (keine Mittelung der y-Koordinaten)
			ycenter = ymax / 2;

			for (x0t = 0; x0t<roi.dx - RF_Calc->templatesizeX; x0t += param.GridStepsizeX)
			{
				for (y0t = 0; y0t<ymax; y0t += param.GridStepsizeY)
				{
					roitemplate.start = roi.start + x0t + y0t*roi.pitch;
					RF_CalculatorArray[param.RF_CalculatorArrayWantedSlot]->f(roitemplate, rf1, rf2);

					MeritValueArray1[x0t + y0t*roi.pitch] = rf1;
					MeritValueArray2[x0t + y0t*roi.pitch] = rf2;

					MF1LineDistributionParameter.GridAveW += rf1;
					MF2LineDistributionParameter.GridAveW += rf2;
					ngrid++;

					if (rf1>MF1LineDistributionParameter.GridMaxW) MF1LineDistributionParameter.GridMaxW = rf1;
					if (rf2>MF2LineDistributionParameter.GridMaxW) MF2LineDistributionParameter.GridMaxW = rf2;
				}
			}

			MF1LineDistributionParameter.GridAveW /= float(ngrid);
			MF2LineDistributionParameter.GridAveW /= float(ngrid);
		}


		void SFTB_SeamFinderTextureBasedT::CalculateLineValues(SFTB_roiT & roi)
		{
			// Summiere die drueber und drunter liegenden Gridwerte
			int x0t, y0t, ix;
			float rf1sum, rf2sum, rf1, rf2, threshold1, threshold2;

			SFTB_SeamQuality.SumMeritFunctionNvaluesYdir = 1 + (roi.dy - RF_Calc->templatesizeY - 1) / param.GridStepsizeY;
			SFTB_SeamQuality.SumMeritFunctionXSubsampling = param.GridStepsizeX;
			threshold1 = MF1LineDistributionParameter.GridAveW*fUsedmfSinglevalueThreshold;
			threshold2 = MF2LineDistributionParameter.GridAveW*fUsedmfSinglevalueThreshold;

			ix = 0;

			MF1LineDistributionParameter.LineMaxW = MF2LineDistributionParameter.LineMaxW = 0.0;
			MF1LineDistributionParameter.LineAveW = MF2LineDistributionParameter.LineAveW = 0.0;

			for (x0t = 0; x0t<roi.dx - RF_Calc->templatesizeX; x0t += param.GridStepsizeX, ++ix)
			{
				rf1sum = rf2sum = 0.0;

				for (y0t = 0; y0t<roi.dy - RF_Calc->templatesizeY; y0t += param.GridStepsizeY)
				{

					rf1 = MeritValueArray1[x0t + y0t*roi.pitch];
					rf2 = MeritValueArray2[x0t + y0t*roi.pitch];
					if (rf1>threshold1) rf1sum += rf1;
					if (rf2>threshold2) rf2sum += rf2;
				}

				linearray1[ix] = rf1sum;
				MF1LineDistributionParameter.LineAveW += rf1sum;
				//MF1LineDistributionParameter.LineQAveW+=rf1sum*rf1sum;
				if (rf1sum>MF1LineDistributionParameter.LineMaxW) MF1LineDistributionParameter.LineMaxW = rf1sum;

				linearray2[ix] = rf2sum;
				MF2LineDistributionParameter.LineAveW += rf2sum;
				//MF2LineDistributionParameter.LineQAveW+=rf2sum*rf2sum;
				if (rf2sum>MF2LineDistributionParameter.LineMaxW) MF2LineDistributionParameter.LineMaxW = rf2sum;

			}
			MF1LineDistributionParameter.LineAveW /= float(ix);
			MF2LineDistributionParameter.LineAveW /= float(ix);
			//MF1LineDistributionParameter.LineQAveW/=float(ix);
			//MF2LineDistributionParameter.LineQAveW/=float(ix);
			linearray1[ix] = -1.0;
			linearray2[ix] = -1.0;
		}


		int SFTB_SeamFinderTextureBasedT::CalculateLineSections(float *linearray, int npixx, MFLineDistributionParameterT & MFLDP, MFPeakSectionT * MFPeaksect, int & MFPeaksectAnz)
		{
			int i, x1, x2, spotlength, gapsize, onspotlength;
			bool onspot, ongap;
			float threshold;
			//float sigmaW;
			//float MaxW;
			float w, spotMaxW, spotSumW;

			MFPeaksectAnz = 0;
			onspotlength = gapsize = 0;
			spotSumW = spotMaxW = 0.0;
			x1 = x2 = 0;
			//threshold= DmfThreshold;
			//threshold= 0.5 * DmfThreshold * 2 * SingleSideGridEvaluation;

			threshold = fUsedmfThreshold*MFLDP.LineMaxW + (1.0f - fUsedmfThreshold)*MFLDP.LineAveW;

			//sigmaW=sqrt(MFLDP.LineQAveW-MFLDP.LineAveW*MFLDP.LineAveW);
			//threshold =  MFLDP.LineAveW+0.4*sigmaW;

			//  printf("MFLDP.LineMaxW=%g  MFLDP.LineAveW=%g MFLDP.LineQAveW=%g threshold=%g\n",MFLDP.LineMaxW,MFLDP.LineAveW,MFLDP.LineQAveW,threshold);

#ifdef SFTBCONSOLEDEBUG
	//	SFTBWINCONENABLE(mcon.magenta();)
	printf("MFLDP.LineMaxW=%g  MFLDP.LineAveW=%g  threshold=%g\n", MFLDP.LineMaxW, MFLDP.LineAveW, threshold);
	//printf("sigmaW=%g  MFLDP.LineAveW+sigmaW=%g \n",sigmaW,MFLDP.LineAveW+sigmaW);
	//	SFTBWINCONENABLE(mcon.white();)
#endif
			onspot = ongap = false;

			//MaxW = 0.0;
			for (i = 0; i<npixx; ++i)
			{
				w = linearray[i];
				if (w<0.0) break;

				if (w >= threshold)
				{
					if (onspot)
					{
						//Spot weiter auffuellen
#ifdef SFTBCONSOLEDEBUG
	printf("Spot weiter auffuellen: i=%d w=%g \n", i, w);
#endif
						x2 = i;
						if (w>spotMaxW) spotMaxW = w;
						spotSumW += w;
						onspotlength++;
						continue;
					}
					else
					{
						if (!ongap)
						{
							//start a new spot
#ifdef SFTBCONSOLEDEBUG
	printf("start a new spot: i=%d w=%g \n", i, w);
#endif
							x1 = x2 = i;
							spotMaxW = w;
							spotSumW = w;
							onspotlength = 1;
						}
						else
						{
							//nach ueberbruecktem Gap: Spot weiter auffuellen
#ifdef SFTBCONSOLEDEBUG
	printf("bridged gap fill spot: i=%d w=%g \n", i, w);
#endif
							x2 = i;
							if (w>spotMaxW) spotMaxW = w;
							spotSumW += w;
							onspotlength++;
						}
						onspot = true;
						ongap = false;
						continue;
					}
				}
				else
				{ // w<threshold
					if (onspot)
					{

#ifdef SFTBCONSOLEDEBUG
	printf("off spot: i=%d w=%g \n", i, w);
#endif

						onspot = false;
						ongap = true;
						gapsize = 1;
						continue;
					}
					else
					{
						if (!ongap) continue;
						// so we are ongap:
						++gapsize;

#ifdef SFTBCONSOLEDEBUG
	printf("off spot on gap: i=%d w=%g gapsize=%d onspotlength=%d\n", i, w, gapsize, onspotlength);
#endif

						spotlength = 1 + x2 - x1;
						if (gapsize<param.MaxGapBridge) continue; //bridge a normal gap

						//spot fertig
						ongap = false;

#ifdef SFTBCONSOLEDEBUG
	SFTBWINCONENABLE(mcon.yellow();)
	printf("spot %d fertig: i=%d x1=%d x2=%d    spotlength=%d  spotMaxW=%g  spotSumW=%g\n", MFPeaksectAnz + 1, i, x1, x2, spotlength, spotMaxW, spotSumW);
	SFTBWINCONENABLE(mcon.white();)
#endif

						MFPeaksect[MFPeaksectAnz].xL = x1;
						MFPeaksect[MFPeaksectAnz].xR = x2;
						MFPeaksect[MFPeaksectAnz].spotMaxW = spotMaxW;
						MFPeaksect[MFPeaksectAnz].onspotlength = onspotlength;
						MFPeaksect[MFPeaksectAnz].spotlength = spotlength;
						MFPeaksect[MFPeaksectAnz].spotSumW = spotSumW;

						MFPeaksectAnz++;
					}
				}
			}

			if (MFPeaksectAnz == 0)
			{
				return 1;
			}

			return 0;
		}


		int SFTB_SeamFinderTextureBasedT::LineSectionEvaluation(MFPeakSectionT * MFPeaksect1, int & MFPeaksectAnz1, MFPeakSectionT * MFPeaksect2, int & MFPeaksectAnz2, int & xLraw, int & xRraw, unsigned char & quality) //Zeilenabschitte zu Nahtrandpositionen verarbeiten
		{
			int i, i1, i2, MaxWGesamtTypeflag;
			float MaxW1, MaxW2, MaxWGesamt, rdif;
			int iquality;

			i1 = i2 = -1;
			MaxWGesamtTypeflag = 1;//default

			//***********************
			//seek maximum sum over peaksection on both sides of the seam
			MaxWGesamt = -1;
			for (i = 0; i<MFPeaksectAnz1; ++i)
			{
				if (MFPeaksect1[i].spotSumW>MaxWGesamt)
				{
					MaxWGesamt = MFPeaksect1[i].spotSumW;
					MaxWGesamtTypeflag = 1;
					i1 = i;
				}
			}

			for (i = 0; i<MFPeaksectAnz2; ++i)
			{
				if (MFPeaksect2[i].spotSumW>MaxWGesamt)
				{
					MaxWGesamt = MFPeaksect2[i].spotSumW;
					MaxWGesamtTypeflag = 2;
					i2 = i;
				}
			}

			MaxW1 = MaxW2 = MaxWGesamt;
			//***********************

			if (MaxWGesamtTypeflag == 1)
			{
				//Bester Bereich ist auf der linken Nahtseite ,i1 gesetzt
				xLraw = MFPeaksect1[i1].xL;
				xRraw = MFPeaksect1[i1].xR; //erstmal

				MaxW2 = -1; //iteriere die Loser auf der rechten Nahtseite
				for (i = 0; i<MFPeaksectAnz2; ++i)
				{
					//if(MFPeaksect1[i1].xL<MFPeaksect2[i].xR) //Forderung ganz Sloppy: Rechter Nahtbereich nicht ganz links von linkem Nahtbereich  Fehler:3, 0verweigert:0
					if (MFPeaksect1[i1].xL<MFPeaksect2[i].xR - param.MinSeamWidth)  //Forderung etwas verschaerft
						//if(MFPeaksect2[i].xL-MFPeaksect1[i1].xL>MinSeamWidth) //Forderung Mittel: Rechter Nahtbereich linke Seite etwas rechts von linkem Nahtbereich linke Seite Fehler:3, verweigert:0
						//if(MFPeaksect1[i1].xR<MFPeaksect2[i].xL) //Forderung streng: Rechter Nahtbereich ganz rechts von linkem Nahtbereich Fehler:26, verweigert:0
					{
						//Loser passt zur linken Nahtseite
						if (MFPeaksect2[i].spotSumW>MaxW2)
						{
							MaxW2 = MFPeaksect2[i].spotSumW;
							i2 = i;
						}
					}
				}
			}
			else
			{
				//Bester Bereich ist auf der rechten Nahrseite ,i2 gesetzt
				xLraw = MFPeaksect2[i2].xL;
				xRraw = MFPeaksect2[i2].xR; //erstmal

				MaxW1 = -1; //iteriere die Loser auf der linken Nahtseite
				for (i = 0; i<MFPeaksectAnz1; ++i)
				{
					//if(MFPeaksect1[i].xL<MFPeaksect2[i2].xR) //Forderung Sloppy: Linker Nahtbereich nicht ganz rechts von rechtem Nahtbereich
					if (MFPeaksect1[i].xL<MFPeaksect2[i2].xR - param.MinSeamWidth) //Forderung etwas verschaerft
						//if(MFPeaksect2[i2].xR-MFPeaksect1[i].xR>MinSeamWidth) //Forderung Mittel: Rechter Nahtbereich rechte Seite etwas rechts von linkem Nahtbereich
						//if(MFPeaksect1[i].xR<MFPeaksect2[i2].xL) //Forderung streng: Rechter Nahtbereich ganz rechts von linkem Nahtbereich
					{
						//Loser passt zur rechten Nahtseite
						if (MFPeaksect1[i].spotSumW>MaxW1)
						{
							MaxW1 = MFPeaksect1[i].spotSumW;
							i1 = i;
						}
					}
				}
			}

			//BAUSTELLE

			SFTB_SeamQuality.SumMeritFunctionOnLeftSeam = MaxW1;
			SFTB_SeamQuality.SumMeritFunctionOnRightSeam = MaxW2;


			SFTB_SeamQuality.WidthLeftSeam = SFTB_SeamQuality.WidthRightSeam = 0;

			if (i1 >= 0)
			{
				SFTB_SeamQuality.WidthLeftSeam = MFPeaksect1[i1].onspotlength;
			}

			if (i2 >= 0)
			{
				SFTB_SeamQuality.WidthRightSeam = MFPeaksect1[i2].onspotlength;
			}

			if (i1 >= 0 && i2 >= 0)
			{
				//Normale Auswertung
				xLraw = (MFPeaksect1[i1].xL<MFPeaksect2[i2].xL) ? MFPeaksect1[i1].xL : MFPeaksect2[i2].xL;
				xRraw = (MFPeaksect1[i1].xR>MFPeaksect2[i2].xR) ? MFPeaksect1[i1].xR : MFPeaksect2[i2].xR;

				//rdif = 0 .. 1
				rdif = float(std::abs(MFPeaksect1[i1].onspotlength - MFPeaksect1[i2].onspotlength)) / (MFPeaksect1[i1].onspotlength + MFPeaksect1[i2].onspotlength);

				iquality = (int)(100 + 155.0 * (1.0 - rdif));
				if (iquality<255) quality = iquality; else quality = 255;
			}
			else
			{
				//xLraw xRraw nur von einer Seite: besser als nix Aber: shitty markieren
				quality = 255 / 10; //10%
			}

#ifdef SFTBCONSOLEDEBUG
	printf("i1=%d   i2=%d\n", i1, i2);
#endif

			return 0;
		}

		//********************************************************************************************************************************
		//********************************************************************************************************************************

		SFTB_SeamQualityT::SFTB_SeamQualityT()
		{
			pp = &sd;
			pp->debuglevel = 0;
			SeamQualityValidFlag = false;
			SumMeritFunctionOnLeftSeam = 0.0;
			SumMeritFunctionOnRightSeam = 0.0;
			WidthLeftSeam = 0;
			WidthRightSeam = 0;
			TimeSeriesRankCenter = 0;
			TimeSeriesRankWidth = 0;
			SeamWidth = 0.0;
			SeamCenter = 0.0;
			SumMeritFunctionNvaluesYdir = 1;
			SumMeritFunctionXSubsampling = 1;
		}


		/*
		Rueckgabe: 0 -> Keine Aussage moeglich
		1 	-> das is eine Naht
		-1 -> das is keine Naht
		*/

		//neues Parametersetting
		int SFTB_SeamQualityT::calculateSeamQuality(void)
		{
			double rdif;
			double widthsum;

			//teile Summe ueber den Spot durch Anzahl der Y-projezierten Werte
			if (SumMeritFunctionNvaluesYdir>0)
			{
				SumMeritFunctionOnLeftSeam = SumMeritFunctionXSubsampling*SumMeritFunctionOnLeftSeam / SumMeritFunctionNvaluesYdir;
				SumMeritFunctionOnRightSeam = SumMeritFunctionXSubsampling*SumMeritFunctionOnRightSeam / SumMeritFunctionNvaluesYdir;
			}

			MaxSumMeritFunction = (SumMeritFunctionOnLeftSeam>SumMeritFunctionOnRightSeam) ? SumMeritFunctionOnLeftSeam : SumMeritFunctionOnRightSeam;
			MinSumMeritFunction = (SumMeritFunctionOnLeftSeam<SumMeritFunctionOnRightSeam) ? SumMeritFunctionOnLeftSeam : SumMeritFunctionOnRightSeam;

#if(0)
	static FILE *fh;
	static int ibild = 0;
	printf("Write file SFTBQtest.dat\n");
	fh = fopen("SFTBQtest.dat", "a");
	fprintf(fh, "%d %g %g %d %d %d %d %g %g %d %d\n", ibild, SumMeritFunctionOnLeftSeam, SumMeritFunctionOnRightSeam, TimeSeriesRankCenter, TimeSeriesRankWidth, WidthLeftSeam, WidthRightSeam, SeamWidth, SeamCenter, SumMeritFunctionNvaluesYdir, SumMeritFunctionXSubsampling);
	fclose(fh);
	++ibild;
#endif

			if (pp->debuglevel>5)
			{
				if (SeamQualityValidFlag)
				{
					printf("SFTB_SeamQuality.SumMeritFunctionOnLeftSeam =%g\n", SumMeritFunctionOnLeftSeam);
					printf("SFTB_SeamQuality.SumMeritFunctionOnRightSeam=%g\n", SumMeritFunctionOnRightSeam);
					printf("SFTB_SeamQuality.MaxSumMeritFunction=%g\n", MaxSumMeritFunction);
					printf("SFTB_SeamQuality.TimeSeriesRankCenter=%d\n", TimeSeriesRankCenter);
					printf("SFTB_SeamQuality.TimeSeriesRankWidth =%d\n", TimeSeriesRankWidth);
					printf("SFTB_SeamQuality.WidthLeftSeam =%d\n", WidthLeftSeam);
					printf("SFTB_SeamQuality.WidthRightSeam=%d\n", WidthRightSeam);
					printf("SFTB_SeamQuality.SeamWidth=%g\n", SeamWidth);
					printf("SFTB_SeamQuality.SumMeritFunctionNvaluesYdir=%d\n", SumMeritFunctionNvaluesYdir);
					printf("SFTB_SeamQuality.SumMeritFunctionXSubsampling=%d\n", SumMeritFunctionXSubsampling);

				}
				else
				{
					printf("SFTB_SeamQuality not calculated\n");
				}
			}


			if (!SeamQualityValidFlag) return 0; //Keine Auswertung gemacht -> keine Aussage moeglich

			// jeden Datensatz nur einmal auswerten:
			SeamQualityValidFlag = false;

			//*******************************************

			// put this to file: SFTB_SeamQuality.cpp  in function:  SFTB_SeamQualityT::calculateSeamQuality(void)

			int v0 = 9;
			int v1 = 78;
			int v2 = 24;
			int v3 = 3;
			int v4 = 2;
			double d0 = 9.52089e+007;
			double d1 = 4.1924e+008;
			double d2 = 3.57552e+008;

			//*******************************************

			if (TimeSeriesRankCenter<v0) return 0; //SFTB Nahtzentrum passt nicht zur Prognose der Zeitreihe -> keine Aussage moeglich
			if (TimeSeriesRankWidth<v1) return 0; //SFTB Nahtbreite passt nicht zur Prognose der Zeitreihe -> keine Aussage moeglich


			widthsum = WidthLeftSeam + WidthRightSeam;
			if (widthsum == 0) return -1; // ->  KeineNaht gefunden
			if (widthsum<v2) return 0; // -> gefundene Bereiche zu schmal -> keine Aussage

			// WidthLeftSeam and WidthRightSeam are >=0
			//rdif = 0 .. 1
			rdif = float(std::abs(WidthLeftSeam - WidthRightSeam)) / widthsum;

			if (pp->debuglevel>8)
			{
				printf("rdif=%g\n", rdif);
			}

			// Summe Meritfunction sehr hoch -> alles klar Naht erkannt   1.1
			if (SumMeritFunctionOnLeftSeam >d0) return 1;
			if (SumMeritFunctionOnRightSeam>d1) return 1;



			double RankProduct;
			RankProduct = double(TimeSeriesRankCenter)* TimeSeriesRankWidth;

			if (pp->debuglevel>5)
			{
				printf("MaxSumMeritFunction=%g\n", MaxSumMeritFunction);
				printf("RankProduct=%g\n", RankProduct);
			}

			if (MaxSumMeritFunction >d2 && TimeSeriesRankCenter>v3 && TimeSeriesRankWidth>v4) return 1;

			return 0;
		}

		//********************************************************************************************************************************
		//********************************************************************************************************************************

		//Default: Read only routines

		int SFTB_RF_CalculatorBaseclassT::setCoefficients(int &inLx1, int &inLy1, int &inLx2, int &inLy2)
		{
			inLx1 = Lx1;
			inLy1 = Ly1;
			inLx2 = Lx2;
			inLy2 = Ly2;

			return -1;
		}

		SFTB_RF_CalculatorBaseclassT::~SFTB_RF_CalculatorBaseclassT()
		{

		}

		//********************************************************************************************************************************
		//********************************************************************************************************************************

		SFTB_RF_CalculatorT::SFTB_RF_CalculatorT()
		{
			Lx1 = -4;
			Ly1 = -4;
			Lx2 = 4;
			Ly2 = -4;
			templatesizeX = 16;
			templatesizeY = 64;

			kern1 = NULL;
			kern2 = NULL;

			kern1 = new signed char[2 * templatesizeX*templatesizeY]; //2* da zwei kernel fuer sin und cos
			kern2 = new signed char[2 * templatesizeX*templatesizeY];
			RaumfrequenzVorbelegen(Lx1, Ly1, Lx2, Ly2);

		}

		SFTB_RF_CalculatorT::~SFTB_RF_CalculatorT()
		{
			if (kern1 != NULL) delete[] kern1;
			if (kern2 != NULL) delete[] kern2;
		}

		void SFTB_RF_CalculatorT::f(SFTB_roiT & roi, float &rf1, float &rf2)
		{
			long i;
			signed short x;
			signed char *tp1, *tp2;
			//unsigned char *ip;
			const byte		*ip;	    ///< Startadresse des Rois

			signed long a1s, a1c;
			signed long a2s, a2c;
			unsigned long JumpToNextLine;
			tp1 = kern1;
			tp2 = kern2;

			JumpToNextLine = roi.pitch - templatesizeX;
			ip = roi.start;

			a1s = a1c = a2s = a2c = 0;
			for (x = 0, i = 0L; i<templatesizeX*templatesizeY; ++i)
			{
				a1s += (*tp1)*(*ip);
				++tp1;
				a1c += (*tp1)*(*ip);
				++tp1;

				a2s += (*tp2)*(*ip);
				++tp2;
				a2c += (*tp2)*(*ip);
				++tp2;

				++ip;
				++x;
				if (x >= templatesizeX)
				{
					x = 0;
					ip += JumpToNextLine;
				}
			}

			rf1 = (double(a1s)*a1s + double(a1c)*a1c) / ((127.0*templatesizeX)*templatesizeY);
			rf2 = (double(a2s)*a2s + double(a2c)*a2c) / ((127.0*templatesizeX)*templatesizeY);

		}

		void SFTB_RF_CalculatorT::RaumfrequenzVorbelegen(signed int inpLx1, signed int inpLy1, signed int inpLx2, signed int inpLy2)
		{
			//printf("RaumfrequenzVorbelegen(%d,%d,%d,%d)\n",inpLx1,inpLy1,inpLx2,inpLy2);

			RaumfrequenzVorbelegen(inpLx1, inpLy1, kern1); //18_Correlation mit Bildern 1-9
			//RaumfrequenzFileoutput("rf1",kern1);
			RaumfrequenzVorbelegen(inpLx2, inpLy2, kern2); //
		}

		void SFTB_RF_CalculatorT::RaumfrequenzFileoutput(char *fnpf, signed char *kernel)
		{
			FILE *fh1, *fh2;
			int x, y;
			signed char *tp;
			char str[100];
			tp = kernel;

			sprintf(str, "%ss.dat", fnpf);
			fh1 = fopen(str, "w");

			sprintf(str, "%sc.dat", fnpf);
			fh2 = fopen(str, "w");

			for (y = 0; y<templatesizeY; ++y)
			{
				for (x = 0; x<templatesizeX; ++x)
				{
					fprintf(fh1, "%d ", *tp);
					++tp;
					fprintf(fh2, "%d ", *tp);
					++tp;
				}
				fprintf(fh1, "\n");
				fprintf(fh2, "\n");
			}
			fclose(fh1);
			fclose(fh2);

		}


		void SFTB_RF_CalculatorT::RaumfrequenzVorbelegen(signed int Lx, signed int Ly, signed char *kernel)
		{
			int x, y;
			signed char *tp;
			tp = kernel;


			for (y = 0; y<templatesizeY; ++y)
			{
				for (x = 0; x<templatesizeX; ++x)
				{
					//FeatureGenerator.cpp :
					//a11s+=sin(x*Lx*2.0*M_PI/ima.npixx+y*Ly*M_PI/ima.npixy)*ima(x,y);
					//a11c+=cos(x*Lx*2.0*M_PI/ima.npixx+y*Ly*M_PI/ima.npixy)*ima(x,y);

					*tp = (signed char)(127 * sin(x*Lx*2.0*M_PI / templatesizeX + y*Ly*M_PI / templatesizeY));
					++tp;

					*tp = (signed char)(127 * cos(x*Lx*2.0*M_PI / templatesizeX + y*Ly*M_PI / templatesizeY));
					++tp;
				}
			}
		}



		int SFTB_RF_CalculatorT::setCoefficients(int & inLx1, int & inLy1, int & inLx2, int & inLy2)
		{
			Lx1 = inLx1;
			Ly1 = inLy1;
			Lx2 = inLx2;
			Ly2 = inLy2;
			RaumfrequenzVorbelegen(Lx1, Ly1, Lx2, Ly2);
			return 0;
		}


	} // namespace precitec
} // namespace filter
