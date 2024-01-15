/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		JS
 * 	@date		2016
 * 	@brief		This filter stores data elements in a ring buffer and models a curve through the data
 */


#include <filter/buffer.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include <overlay/layerType.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <fliplib/TypeToDataTypeImpl.h>

// WM includes
#include "modelCurve.h"
#include "model.h"

namespace precitec {
using namespace interface;
using namespace image;
using namespace geo2d;
namespace filter {

	const std::string ModelCurve::m_oFilterName("ModelCurve");

	const std::string ModelCurve::PIPENAME_MIN = std::string("Min");
	const std::string ModelCurve::PIPENAME_MAX = std::string("Max");
	const std::string ModelCurve::PIPENAME_OUTLIER = std::string("Outlier"); // the pipename is not adapted to the variable name
	const std::string ModelCurve::PIPENAME_KOEFF1 = std::string("Koeff1");   // the pipename is not adapted to the variable name
	const std::string ModelCurve::PIPENAME_KOEFF2 = std::string("Koeff2");   // the pipename is not adapted to the variable name
	const std::string ModelCurve::PIPENAME_CURVE = std::string("Curve");


	const std::string ModelCurve::m_oParamSlot("Slot");				///< Parameter: slot, into which the data is written.
	const std::string ModelCurve::m_oParamMode("Mode");				///< Parameter: different processing modi
	const std::string ModelCurve::m_oParamTicks("Ticks");			///< Parameter: usually encoder tick number which corrsponds to 360 degree
	const std::string ModelCurve::m_oParamConsensSet("ConsensSet");		///< Parameter: consens set- the number of values inside below the threshold compared to the model
	const std::string ModelCurve::m_oParamErrorThreshold("ErrorThreshold");	///< Parameter: oulier thrsshold
	const std::string ModelCurve::m_oParamIterations("Iterations");		///< Parameter: iterations for the RANSAC processing
	const std::string ModelCurve::m_oParamLowerThreshold("LowerThreshold");	///< Parameter: Lower threshold for valid values
	const std::string ModelCurve::m_oParamUpperThreshold("UpperThreshold");	///< Parameter: Upper thrshold for vaid values
	const std::string ModelCurve::m_oParamDegree("Degree");			///< Parameter: degree for the mathematical model
	const std::string ModelCurve::m_oParamEraseKernelSize("KernelSize");			///< Parameter: KErnel Size for erasing data
	const std::string ModelCurve::m_oParamEraseKernelDistance("KernelDistance");			///< Parameter: degree for the distance between the erasing windows
	const std::string ModelCurve::m_oParamTickSize("TickSize");			///< Parameter: ticksize for the processing start
	const std::string ModelCurve::m_oParamDrawFactor("DrawFactor");			///< Parameter: factor to scale the values
	const std::string ModelCurve::m_oParamDrawMax("DrawMax");			///< Parameter: axis max for drawing
	const std::string ModelCurve::m_oParamDrawMin("DrawMin");			///< Parameter: axis min for drawing
	const std::string ModelCurve::m_oParamYAxisFactor("YAxisFactor");			///< Parameter: axis min for drawing
	const std::string ModelCurve::m_oParamPeriodLength("PeriodLength");			///< Parameter: peroid Length as a start value for the LM algorithm
	const std::string ModelCurve::m_oParamModelType("ModelType");			///< Parameter: Model Type of Fit
	const std::string ModelCurve::m_oParamEraseStart("EraseStart");			///< Parameter: Erase Start for the data elimination
	const std::string ModelCurve::m_oParamHold("Hold");			///< Parameter: Hold afit coefficient
	const std::string ModelCurve::m_oParamLearnParam1("LearnParam1");			///< Parameter: lambda1 for gradient descnet in non linear fit
	const std::string ModelCurve::m_oParamLearnParam2("LearnParam2");			///< Parameter: lambda2 for gradient descent in non linear fit



	ModelCurve::ModelCurve() :
		TransformFilter(ModelCurve::m_oFilterName, Poco::UUID{"D7E5D010-87A8-491F-BD72-467D18A25A76"}),
		m_pPipeInData(nullptr),
		m_pPipeInPos(nullptr),
		m_oPipeOutMin(this, ModelCurve::PIPENAME_MIN),
		m_oPipeOutMax(this, ModelCurve::PIPENAME_MAX),
		m_oPipeOutOutlier(this, ModelCurve::PIPENAME_OUTLIER),  // the pipename is not adapted to the variable name
		m_oPipeOutKoeff1(this, ModelCurve::PIPENAME_KOEFF1),     // the pipename is not adapted to the variable name
		m_oPipeOutKoeff2(this, ModelCurve::PIPENAME_KOEFF2),      // the pipename is not adapted to the variable name
		m_oPipeOutCurve(this, ModelCurve::PIPENAME_CURVE),
		m_oCount(0),
		m_bPaintFlag(false),
		m_oProcessingFlag(false),
		m_oSlot(1),
		m_oMode(0),
		m_oTicks(10000),
		m_oConsensSet(100),
        m_oErrorThreshold(10),
		m_oIterations(10),
		m_oLowerThreshold(0),
		m_oUpperThreshold(80),
		m_oDegree(4),
		m_oEraseKernelSize(10),
		m_oEraseKernelDistance(20),
		m_oTickSize(100),
		m_oDrawFactor(2),
		m_oDrawMax(150),
		m_oDrawMin(0),
		m_oYAxisFactor(2),
		m_oPeriodLength(360000),
		m_oModelType(0),                  //Test -- initialise the new parameters
		m_oEraseStart(0),
		m_oHold(2),
		m_oLearnParam1(0.1),
		m_oLearnParam2(2)
{
		parameters_.add(m_oParamSlot, fliplib::Parameter::TYPE_uint, m_oSlot);
		parameters_.add(m_oParamMode, fliplib::Parameter::TYPE_uint, m_oMode);
		parameters_.add(m_oParamTicks, fliplib::Parameter::TYPE_uint, m_oTicks);
		parameters_.add(m_oParamConsensSet, fliplib::Parameter::TYPE_uint, m_oConsensSet);
		parameters_.add(m_oParamErrorThreshold, fliplib::Parameter::TYPE_double, m_oErrorThreshold);
		parameters_.add(m_oParamIterations, fliplib::Parameter::TYPE_uint, m_oIterations);
		parameters_.add(m_oParamLowerThreshold, fliplib::Parameter::TYPE_double, m_oLowerThreshold);
		parameters_.add(m_oParamUpperThreshold, fliplib::Parameter::TYPE_double, m_oUpperThreshold);
		parameters_.add(m_oParamDegree, fliplib::Parameter::TYPE_uint, m_oDegree);
		parameters_.add(m_oParamEraseKernelSize, fliplib::Parameter::TYPE_int, m_oEraseKernelSize);
		parameters_.add(m_oParamEraseKernelDistance, fliplib::Parameter::TYPE_int, m_oEraseKernelDistance);
		parameters_.add(m_oParamTickSize, fliplib::Parameter::TYPE_uint, m_oTickSize);
		parameters_.add(m_oParamDrawFactor, fliplib::Parameter::TYPE_double, m_oDrawFactor);
		parameters_.add(m_oParamDrawMax, fliplib::Parameter::TYPE_double, m_oDrawMax);
		parameters_.add(m_oParamDrawMin, fliplib::Parameter::TYPE_double, m_oDrawMin);
		parameters_.add(m_oParamYAxisFactor, fliplib::Parameter::TYPE_double, m_oYAxisFactor);
		parameters_.add(m_oParamPeriodLength, fliplib::Parameter::TYPE_double, m_oPeriodLength);
		parameters_.add(m_oParamModelType, fliplib::Parameter::TYPE_int, m_oModelType);
		parameters_.add(m_oParamEraseStart, fliplib::Parameter::TYPE_int, m_oEraseStart);
		parameters_.add(m_oParamHold, fliplib::Parameter::TYPE_int, m_oHold);
		parameters_.add(m_oParamLearnParam1, fliplib::Parameter::TYPE_double, m_oLearnParam1);
		parameters_.add(m_oParamLearnParam2, fliplib::Parameter::TYPE_double, m_oLearnParam2);

        setInPipeConnectors({{Poco::UUID("4F7D7151-F3EF-4A16-9314-CBF4231960BF"), m_pPipeInData, "Data", 1024, "data"},
        {Poco::UUID("258C3F63-BBD0-4B17-ACC9-320D9BF2E049"), m_pPipeInPos, "Position", 1024, "pos", fliplib::PipeConnector::ConnectionType::Optional}});
        setOutPipeConnectors({{Poco::UUID("10ABB7A2-4C77-4B32-8EF0-805593E4AC71"), &m_oPipeOutMin, "Min", 0, ""},
        {Poco::UUID("31609BF4-713C-452B-9D7D-E77F8A244EFB"), &m_oPipeOutMax, "Max", 0, ""},
        {Poco::UUID("AAFB0CE5-4070-49CC-B654-D41B4279ECCF"), &m_oPipeOutOutlier, "Outlier", 0, ""},
        {Poco::UUID("79C720E1-63FA-4851-B615-2CF6E38C8CDE"), &m_oPipeOutKoeff1, "Koeff1", 0, ""},
        {Poco::UUID("F1AD45E3-E0A7-46FB-B844-AAB485ECA900"), &m_oPipeOutKoeff2, "Koeff2", 0, ""},
        {Poco::UUID("A5F70285-E3AA-4C2B-9916-F294009ECB2E"), &m_oPipeOutCurve, "Curve", 0, ""}});
        setVariantID(Poco::UUID("954E8D59-467A-4FF7-A051-1001EE03B62D"));
}

ModelCurve::~ModelCurve()
{
}

void ModelCurve::setParameter()
{
		TransformFilter::setParameter();
		m_oSlot = parameters_.getParameter(ModelCurve::m_oParamSlot).convert<unsigned int>();
		m_oMode = parameters_.getParameter(ModelCurve::m_oParamMode).convert<unsigned int>();
		m_oTicks = parameters_.getParameter(ModelCurve::m_oParamTicks).convert<unsigned int>();
		m_oConsensSet = parameters_.getParameter(ModelCurve::m_oParamConsensSet).convert<unsigned int>();
		m_oErrorThreshold = parameters_.getParameter(ModelCurve::m_oParamErrorThreshold).convert<double>();
		m_oIterations = parameters_.getParameter(ModelCurve::m_oParamIterations).convert<unsigned int>();
		m_oLowerThreshold = parameters_.getParameter(ModelCurve::m_oParamLowerThreshold).convert<double>();
		m_oUpperThreshold = parameters_.getParameter(ModelCurve::m_oParamUpperThreshold).convert<double>();
		m_oDegree = parameters_.getParameter(ModelCurve::m_oParamDegree).convert<unsigned int>();
		m_oEraseKernelSize = parameters_.getParameter(ModelCurve::m_oParamEraseKernelSize).convert<int>();
		m_oEraseKernelDistance = parameters_.getParameter(ModelCurve::m_oParamEraseKernelDistance).convert<int>();
		m_oTickSize = parameters_.getParameter(ModelCurve::m_oParamTickSize).convert<unsigned int>();
		m_oDrawFactor = parameters_.getParameter(ModelCurve::m_oParamDrawFactor).convert<double>();
		m_oDrawMax = parameters_.getParameter(ModelCurve::m_oParamDrawMax).convert<double>();
		m_oDrawMin = parameters_.getParameter(ModelCurve::m_oParamDrawMin).convert<double>();
		m_oYAxisFactor = parameters_.getParameter(ModelCurve::m_oParamYAxisFactor).convert<double>();
		m_oPeriodLength = parameters_.getParameter(ModelCurve::m_oParamPeriodLength).convert<double>();
		m_oModelType = parameters_.getParameter(ModelCurve::m_oParamModelType).convert<int>();
		m_oEraseStart = parameters_.getParameter(ModelCurve::m_oParamEraseStart).convert<int>();
		m_oHold = parameters_.getParameter(ModelCurve::m_oParamHold).convert<int>();
		m_oLearnParam1 = parameters_.getParameter(ModelCurve::m_oParamLearnParam1).convert<double>();
		m_oLearnParam2 = parameters_.getParameter(ModelCurve::m_oParamLearnParam2).convert<double>();


	} // setParameter.




//Zeichnen der berechneten Kurve
void ModelCurve::paint()
{
		if (m_oVerbosity == eNone || m_pTrafo.isNull())  // filter should not paint anything on verbosity eNone
		{
			return;
		} // if

//		const Trafo		&rTrafo(*m_pTrafo);

		OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
//		OverlayLayer	&rLayerPosition(rCanvas.getLayerPosition());
		OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

		if (m_bPaintFlag)
		{
			// max. Bildgroesse nutzen
			int maxXImgSize = static_cast<int>(m_oDrawMax);    //400;  //static_cast<int>(m_oHwRoi.x);

			//Punkte zeichnen - alle Werte auf Bildbreite und Hoehe skalieren
			int maxVecSize = m_oVecX.size();

			//int y0 =     static_cast<double>(maxYImgSize) * m_oYAxisFactor;
			int y0 = static_cast<int>(m_oYAxisFactor);


			double xDrawRange = static_cast<double>(maxXImgSize);
			double xValueRange = m_oMaxPos - m_oMinPos;

			geo2d::TPoint<int>  oDrawPoint(0,0);
			geo2d::TPoint<int>  oDrawPointModel(0,0);

			for (unsigned int m=0; m<xDrawRange; m++)
			{
				oDrawPoint.x = m;
				oDrawPoint.y = y0;

				rLayerContour.add(new OverlayPoint(oDrawPoint, Color::Blue()));

			}

			for(int x = 0; x < maxVecSize; x++)
			{
				if ( (xValueRange * m_oVecX[x]) == 0 ) //zero division
				{
					oDrawPoint.x = static_cast<int>(x);
					oDrawPointModel.x = static_cast<int>(x);
				}
				else
				{
					oDrawPoint.x = static_cast<int>( xDrawRange / xValueRange * m_oVecX[x]); //aufsteigend
					oDrawPointModel.x = static_cast<int>( xDrawRange / xValueRange * m_oVecX[x]); //aufsteigend
				}


				oDrawPoint.y = y0 - static_cast<int>(m_oVecY[x]* m_oDrawFactor);
				oDrawPointModel.y = y0 - static_cast<int>(m_oResultCurve[x]* m_oDrawFactor);

				rLayerContour.add(new OverlayPoint ( oDrawPoint, Color::Yellow()));
				rLayerContour.add(new OverlayPoint ( oDrawPointModel, Color::Red()));

			}
		}

}


void ModelCurve::arm(const fliplib::ArmStateBase& state)
{
		if (state.getStateID() == eSeamStart)
		{
			_ownBuffer.reset(); 						// Puffer zuruecksetzen bei neuer Naht

			//member setzen - ticks per 360 Grad
			_ownBuffer.setTicksPer360(m_oTicks);
			//_ownBuffer.setWidths(m_oWidth, m_oWidthMedian);

			// get product information
			const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
			m_oTriggerDelta = pProductData->m_oTriggerDelta;

			// reset / initialize the memory for the slot that was selected by the user ...
			// Daten
			Buffer& rData = BufferSingleton::getInstanceData();
			if (pProductData->m_oNumTrigger < 100000)
			{
				rData.init(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, pProductData->m_oNumTrigger);
			}
			else
			{
				rData.init(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, 100000);
			}
			//Zugriff auf den slot
			m_pData = rData.get(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam);   //holt Daten aus dem Slot



			// Encoder ticks
			Buffer& rPos = BufferSingleton::getInstancePos();
			if (pProductData->m_oNumTrigger < 100000)
				rPos.init(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, pProductData->m_oNumTrigger);
			else
				rPos.init(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, 100000);

			m_pPos = rPos.get(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam);

			m_oCount = 0;
			m_oProcessingFlag = true;
		} // if

		if (state.getStateID() == eSeamEnd)
		{

			//setze member ticks per 360Grad
			_ownBuffer.setTicksPer360(m_oTicks);

			//int startIndex = 0;
			//int endIndex = 0;


			// Hier muessen  noch Tests gemacht werden mit Drehungen > 360 grad, bzw. Encodereingang
			if (!_ownBuffer.is360()) // keine 360 Grad gedreht
			{
				// Analysefehler ausloesen!
				// Tja, was machen?
			}


			// Ring-Puffer in Slot ablegen, damit alles wie bisher aussieht
			int curSize = _ownBuffer.getCurrentSize();
			//std::cout << "Wie gross ist der Puffer am Ende : " << curSize << std::endl;
			for (int i = 0; i < curSize; i++)  //Wert fuerWert umkopieren
			{
				SingleDataSet oneDataSet = _ownBuffer.getOneDataSet(i);

				m_pData->ref()[i] = std::tie(oneDataSet.data, oneDataSet.data_rank);
				m_pPos->ref()[i] = std::tie(oneDataSet.pos, oneDataSet.pos_rank);

				//std::cout << "*****" <<std::get<eData>(m_pData->ref()[i]) << " ***** " <<std::get<eData>(m_pPos->ref()[i]) << std::endl;
			}


		}

		if (m_oVerbosity >= eHigh)
		{
			wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(), state.getStateID());

		} // if
} // arm

bool ModelCurve::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
		if (p_rPipe.tag() == "data")
			m_pPipeInData = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&p_rPipe);
		if (p_rPipe.tag() == "pos")
			m_pPipeInPos = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&p_rPipe);

		return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe

void ModelCurve::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& e)
{
		poco_assert_dbg(m_pPipeInData != nullptr); // to be asserted by graph editor

		// checks, should never happen ...
		if (m_pData == std::shared_ptr< interface::GeoDoublearray >() || m_pPos == std::shared_ptr< interface::GeoDoublearray >())
		{
			wmLog(eError, "Buffer is not initialized, cannot write any elements!\n");
            preSignalAction();
			return;
		}
		if ((int)(m_pData->ref().size()) <= m_oCount || (int)(m_pPos->ref().size()) <= m_oCount)
		{
			wmLog(eError, "Buffer is too small, cannot write more elements!\n");
            preSignalAction();
			return;
		}

		if(m_oModelType==eGauss) //not implemented
		{
		    precitec::wmLog(precitec::LogType::eWarning, "Datenverarbeitung Gauss Fit nicht implementiert");
		}

		// data
		const GeoDoublearray &rGeoDoubleDataIn = m_pPipeInData->read(m_oCounter);

		m_pTrafo = rGeoDoubleDataIn.context().trafo();

		// Kontext auslesen aus der data pipe
		const ImageContext&			rContextX(rGeoDoubleDataIn.context());

		// HW ROI Werte holen
		m_oHwRoi.x = rContextX.HW_ROI_x0;
		m_oHwRoi.y = rContextX.HW_ROI_y0;



		double currentData = rGeoDoubleDataIn.ref().getData()[0];
		int currentDataRank = rGeoDoubleDataIn.ref().getRank()[0];



		// position information - do we have actual information from the in-pipe, or do we simply calculate the position based on the time?
		int oRank = eRankMax;
		double currentPos;
		int currentPosRank;


		// Hack fuer Filtertest********Params setzen nur fuer Filtertest Parameter setzen
		/*
		m_oTriggerDelta = 2.06;
		m_oMode = 0;
		m_oIterations = 0;
		m_oDrawMin = 2.0;
		m_oEraseKernelDistance = 5;
		m_oEraseKernelSize = 5;
	    m_oEraseStart = 2;
		m_oErrorThreshold = 50;
		*/

		if (m_pPipeInPos == nullptr) //------- Achtung Test
		{
                //m_oTiggerDelta kommt in 1/1000 mm oder 1/1000 Grad daher
				double oPos = static_cast<double>(m_oCount) * m_oTriggerDelta; //-- nicht verfuegbar in der Simulation
				currentPos = oPos;
				currentPosRank = oRank;
				if (m_oVerbosity >= eHigh)
				{
					std::cout << "Laufende Position: " << currentPos << std::endl;
					std::cout << "Image Counter, TriggerDelta: " << m_oCount << "," << m_oTriggerDelta << std::endl;
				}
		}
		else
		{
			const GeoDoublearray &rGeoDoublePosIn = m_pPipeInPos->read(m_oCounter);
			currentPos = rGeoDoublePosIn.ref().getData()[0];
			currentPosRank = rGeoDoublePosIn.ref().getRank()[0];
		}



		//Werte in temporären Puffer schreiben - bis die Verarbeitung stattfindet
		int oOwnBufferSize = 0;
		if (m_oProcessingFlag)
		{
			_ownBuffer.addOneDataSet(SingleDataSet(currentData, currentDataRank, currentPos, currentPosRank));

			//Test own buffer in die Puffer m_pData und m_pPos:
			SingleDataSet oneDataSet = _ownBuffer.getOneDataSet(m_oCount);

			m_pData->ref()[m_oCount] = std::tie(oneDataSet.data, oneDataSet.data_rank);
			m_pPos->ref()[m_oCount] = std::tie(oneDataSet.pos, oneDataSet.pos_rank);

			oOwnBufferSize = _ownBuffer.getCurrentSize();

		}
		// some debug output
		if (m_oVerbosity >= eHigh)
		{
			wmLog(eInfo, "BufferRecorder: Storing %f (r:%d) at %f in slot %d element %d!\n", std::get<eData>(m_pData->ref()[m_oCount]), std::get<eRank>(m_pData->ref()[m_oCount]), std::get<eData>(m_pPos->ref()[m_oCount]), m_oSlot, m_oCount);
		}



		//Werte auf die outpipe schreiben:
		if (currentPos <= m_oTickSize)
		{
			m_oMinOut.assign(1, 0.0, eRankMin);
			m_oMaxOut.assign(1, 0.0, eRankMin);
			m_oOutlierOut.assign(1, 0.0, eRankMin);
			m_oKoeff1Out.assign(1, 0.0, eRankMin);
			m_oKoeff2Out.assign(1, 0.0, eRankMin);
			//Kurve zuweisen ????


			m_oCurveOut.assign(1, 0.0, eRankMin);
		}
		else if	((currentPos > m_oTickSize) && (m_oProcessingFlag) &&(m_oModelType!=eGauss)&&(oOwnBufferSize >5)  ) //Verarbeitung anstossen
		{

			m_oMaxData = -1000000000000.0;
			m_oMinData = 1000000000000.0;

			m_oMaxPos = -1000000000000.0;
			m_oMinPos = 1000000000000.0;

			m_oVecX.clear();
			m_oVecY.clear();

			m_oVecXOrig.clear();


			//Puffer in jedem Fall sortieren
			_ownBuffer.sort();

			//get the stored values from the temp buffer:
			int curSize = _ownBuffer.getCurrentSize();
			std::cout << "Model Curve Processing start with date size: " <<curSize<<", tickSize "<<m_oTickSize<<", and current Pos "<<currentPos<<std::endl;

			std::stringstream sstr;
			sstr << curSize << ", tickSize " << m_oTickSize << ", Pos " << currentPos; // << std::endl;
			std::string str(sstr.str());
			wmLogTr(precitec::eInfo, "QnxMsg.Workflow.ModelStart","Curve Processing started with buffer size %s",str.c_str() );

			std::vector<double> sortVec;
			for (int i = 0; i < curSize; i++)  //Wert fuerWert umkopieren
			{
				SingleDataSet oneDataSet = _ownBuffer.getOneDataSet(i);

				//original x Werte ehalten
				m_oVecXOrig.push_back(oneDataSet.pos);

				//Datengrenze beruecksichtigen
				if ((oneDataSet.data > m_oLowerThreshold) && (oneDataSet.data < m_oUpperThreshold))
				{
					// rank beruecksichtigen
					if ((oneDataSet.pos_rank >0) && (oneDataSet.data_rank))
					{
						m_oVecX.push_back(oneDataSet.pos); //x Position i.e. Encode ticks
						m_oVecY.push_back(oneDataSet.data);

						sortVec.push_back(oneDataSet.data);

						if (oneDataSet.data > m_oMaxData)
							m_oMaxData = oneDataSet.data;
						if (oneDataSet.data < m_oMinData)
							m_oMinData = oneDataSet.data;

						if (oneDataSet.pos > m_oMaxPos)
							m_oMaxPos = oneDataSet.pos;
						if (oneDataSet.pos < m_oMinPos)
							m_oMinPos = oneDataSet.pos;
					}
				}
			}


			//std::cout << "Starte ransac verfahren mit " << m_oVecX.size() << " Daten, Min/Max: " << m_oMinData << " " << m_oMaxData << std::endl;
			if(m_oMode == 1)
			{

			   wmLog(eInfo, "No Buffer Correction, data eliminating if iteration > 1\n");
			  //data elimination will be done in the ransac process
			  //without front up cleaning

			}

			// buffer correction in front of the fit processing
			// this correction eliminates the percent highest and lowest values
			if ((m_oMode == 2) && (m_oDrawMin>0))
			{
				double high = 0.0;
				double low  = 0.0;
				double percent = m_oDrawMin; //10 -- Parameter nutzen
				int status = bufferCorrection( sortVec,m_oVecY, high,low,percent);
				if(status <0)
				{
					wmLog(eInfo, "Buffer Correction: high - low entry is too small\n");
				}
				else
				{
					//Daten sollten eingeschraenkt sein:
					m_oMaxData = high;
					m_oMinData = low;
				}

				// some debug output
				if (m_oVerbosity >= eMedium)
				{
					wmLog(eInfo, "Buffer Correction min max: high value %f and low value %f\n", high,low);

				}
			}

			//this correction decreases the distance from mean y value to all the other values by the factor percent
			if ((m_oMode == 3) && (m_oDrawMin>0))
			{

				double percent = m_oDrawMin; //10 -- Parameter nutzen
				double min = 0.0;
				double max = 0.0;
				double mostVal = bufferMeanCorrection(m_oVecY,min,max,percent); //minimaler und maximaler Wert sollten ausgegeben werden.
				if (mostVal < 0)
				{
					wmLog(eInfo, "Buffer Correction: high - low entry is too small\n");
				}
				else
				{
					//sonst gab es keine Korrektur...
					if (max<m_oMaxData)
						m_oMaxData= max;
					if (min>m_oMinData)
						m_oMinData= min;

				}
				// some debug output
				if (m_oVerbosity >= eMedium)
				{
					wmLog(eInfo, "Buffer Mean Correction: most occurence value%f\n", mostVal);

				}
			}

			m_oMeanData = calcMeanValue(m_oVecY);
			double oAmplitude = (m_oMaxData - m_oMinData) / 2.0;
			int oHold = m_oHold;   // set periodlength fix: coeff 2
			double olambda1 = m_oLearnParam1;
			double olambda2 = m_oLearnParam2;

			std::cout << "Start Werte nach Korrektur " << m_oVecX.size() << " Daten, Min/Max: " << m_oMinData << " " << m_oMaxData
				<<" mean: "<<m_oMeanData<<" amplitude: "<<oAmplitude<< std::endl;

			sstr.str("");
			sstr << m_oVecX.size() << ", Min " << m_oMinData << ", Max " << m_oMaxData;// << std::endl;
			std::string str2(sstr.str());

			if (m_oVerbosity >= eMedium)
			{
			    wmLogTr(precitec::eInfo, "QnxMsg.Workflow.ModelProcess", "Start data processing with buffer size %s", str2.c_str());
			}

			if (m_oPeriodLength <= 0.0)
				m_oPeriodLength = 370;

            /*
			std::cout << "ConsensSet: " << m_oConsensSet << ", ErrorThreshold: " << m_oErrorThreshold << ", Iterations: " << m_oIterations
				<< ", Degree: " << m_oDegree << ", KernelSize: " << m_oEraseKernelSize << ", Distance: " << m_oEraseKernelDistance
				<< ", Mode: " << m_oMode << ", Period: " << m_oPeriodLength << ", Type: " << m_oModelType << std::endl;
            */

			Model fitRansac(m_oConsensSet,m_oErrorThreshold,m_oIterations,m_oDegree,
				m_oEraseKernelSize,m_oEraseKernelDistance, m_oMode,m_oVecX,m_oVecY,m_oPeriodLength,
				static_cast<FitType>(m_oModelType),m_oEraseStart,m_oMeanData,oAmplitude,oHold,olambda1,olambda2);

			//fit operations
			fitRansac.processRansac();

			//clean signal with the errorThreshold
			//and process cleaned data again
			fitRansac.processCleanSignal();

			//eliminate outliers ---

			m_oResultCurve = fitRansac.getCurve();


			/*Testfile check data
			std::ofstream file;
			file.open("results3.txt",std::ios::app);
			int k=0;
			for(int m=0;m<m_oVecX.size();m++)
			{
			    file << m_oVecX[m] << ";" << m_oResultCurve[m] << std::endl;
			    k++;
			}
			file.close();
			std::cout <<" "<<k<<" Werte nach fit gespeichert"<<std::endl;
            */

			m_oMinData = fitRansac.getMin();
			m_oMaxData = fitRansac.getMax();
			m_oMeanData = fitRansac.getMean();
			m_oOutlier = fitRansac.getOutlier();
			m_oKoeff1 = fitRansac.getKoeff1();
			m_oKoeff2 = fitRansac.getKoeff2();
			m_oKoeff3 = fitRansac.getKoeff3();
			m_oConsens = fitRansac.getConsens();
			m_oError = fitRansac.getError();
			m_oCost = fitRansac.getCost();


			std::cout << "Koeffizienten - Koeff1: " << m_oKoeff1 << " Koeff2: " << m_oKoeff2 << " Koeff3: " << m_oKoeff3 << std::endl;


			// No Consens values: outlier values in percent regarding all values:
			if (m_oVecX.size() > 0)
				m_oConsens = 100 - m_oConsens; //Output as non Consens Values: Outlier in percent


			//in case of sine modell transpose the coefficient 3 to get the period length
			// problem negative coefficient
			if(m_oModelType==eSine)
			{
				if(m_oKoeff3>0.0)
				{
			        m_oKoeff3 = PI2 / m_oKoeff3;
				    std::cout << "PI2/m_oKoeff3: " <<m_oKoeff3<< std::endl;
				}
				else if (m_oKoeff3<0.0 )
				{
					m_oKoeff3 = PI2 / (-1.0*m_oKoeff3);
					std::cout << "PI2/m_oKoeff3 negative: " <<m_oKoeff3<< std::endl;
				}
			}

			//Ausgangsarrays setzen
			m_oMinOut.assign(1, m_oMinData, eRankMin);
			m_oMaxOut.assign(1, m_oMaxData, eRankMin);
			m_oOutlierOut.assign(1,m_oConsens, eRankMin);
			m_oKoeff1Out.assign(1, m_oKoeff1, eRankMin);  // offset
			m_oKoeff2Out.assign(1, m_oKoeff3, eRankMin);  // Periodlength in case of sine, else the x^2 term


			std::vector<double> coefficients = fitRansac.getCoefficients();

			std::cout << "Ransac beendet, min und max Data: " << m_oMinData << "," << m_oMaxData << std::endl;
			//std::cout << "Koeffizienten: "<<

			if (m_oVerbosity >= eMedium)
			{
			    sstr.str("");
			    sstr<<" min: "<< m_oMinData << ", max: " << m_oMaxData;// << std::endl;
			    std::string str3(sstr.str());

			    //std::string str(sstr.str());
			    wmLogTr(precitec::eInfo, "QnxMsg.Workflow.ModelEnd", "Processing ended with %s", str3.c_str());
                sstr.str("");
                sstr<<"Coefficients: ";
                for(unsigned int i =0; i< coefficients.size();++i)
            	   sstr<<coefficients[i]<<",";
                std::string OutString(sstr.str());
                wmLog(precitec::eInfo, OutString.c_str());
			}

			//wmLogTr(precitec::eDebug, "QnxMsg.Workflow.ModelEnd", "RANSAC Results size %i, minData %f, maxData %f", m_oVecX.size(), m_oMinData, m_oMaxData);

			m_oCurveOut.assign(m_oResultCurve.size(), 0.0, eRankMax);
			std::vector<double> &dummy = m_oCurveOut.getData();


			for (unsigned int j=0; j<dummy.size();++j)
			{
				dummy[j] = m_oResultCurve[j] ;
			}



			if (m_oVecY.size() == m_oResultCurve.size())
			{
				//m_oVecY = m_oResultCurve;
				//jetzt alle Werte in den _ownBuffer zurueck um im seamEnd
				//alle Werte zu sichern
				_ownBuffer.reset(); 						// Puffer zuruecksetzen um neue Daten zu sichern

				for (unsigned n = 0; n < m_oResultCurve.size(); ++n)
				{
					_ownBuffer.addOneDataSet(SingleDataSet(m_oResultCurve[n], 255, m_oVecX[n],255));
				}

			}
			else
			{
				wmLog(eDebug, "Groesse der berechneten Kurve ungleich y Groesse \n");
			}

			//daten zeichnen:
			m_bPaintFlag = true;
			//diese Berechnung nicht mehr durchfuehren:
			m_oProcessingFlag = false;

		}//else if

		// senden
		//logTiming();
		const GeoDoublearray	oGeoOutMin(rGeoDoubleDataIn.context(), m_oMinOut, rGeoDoubleDataIn.analysisResult(), rGeoDoubleDataIn.rank());
		const GeoDoublearray	oGeoOutMax(rGeoDoubleDataIn.context(), m_oMaxOut, rGeoDoubleDataIn.analysisResult(), rGeoDoubleDataIn.rank());
		const GeoDoublearray	oGeoOutOutlier(rGeoDoubleDataIn.context(), m_oOutlierOut, rGeoDoubleDataIn.analysisResult(), rGeoDoubleDataIn.rank());
		const GeoDoublearray	oGeoOutKoeff1(rGeoDoubleDataIn.context(), m_oKoeff1Out, rGeoDoubleDataIn.analysisResult(), rGeoDoubleDataIn.rank());
		const GeoDoublearray	oGeoOutKoeff2(rGeoDoubleDataIn.context(), m_oKoeff2Out, rGeoDoubleDataIn.analysisResult(), rGeoDoubleDataIn.rank());
		const GeoDoublearray	oGeoOutCurve(rGeoDoubleDataIn.context(), m_oCurveOut, rGeoDoubleDataIn.analysisResult(), rGeoDoubleDataIn.rank());

		// ok, we are done here, lets increase write-index
		m_oCount++;
		preSignalAction();
		m_oPipeOutMin.signal(oGeoOutMin);
		m_oPipeOutMax.signal(oGeoOutMax);
		m_oPipeOutOutlier.signal(oGeoOutOutlier);
		m_oPipeOutKoeff1.signal(oGeoOutKoeff1);
		m_oPipeOutKoeff2.signal(oGeoOutKoeff2);
		m_oPipeOutCurve.signal(oGeoOutCurve);
} // proceedGroup


void ModelCurve::generateSinus(std::vector<double> &xVec, std::vector<double> &yVec, double &yMax)
{
	//test
	const double PI2 = 6.283185307179586476925286766559;
	unsigned int xSize = xVec.size();

	//generate random numbers for sinus curve
	std::vector<double> randVec(xSize);

	for(unsigned int i = 0; i < randVec.size(); ++i)
	{
		randVec[i] = (static_cast<double>(rand() % 10 - 1)) / 2.0;
		//outlier:
		//if (i % 30 == 0)
		//	randVec[i] += 15.0;
	}

	double y0 = yMax / 2;
	for (unsigned int i = 0; i < xSize; i++)
	{
		xVec[i] = i;
		yVec[i] = y0 + sin((double)i / (static_cast<double>(xSize)) * PI2) * ((y0 / 100) * 80) + (randVec[i]);
	}

}


//Entferne die % groessten und kleinsten Werte aus dem Puffer
int ModelCurve::bufferCorrection(std::vector<double> &sortVec,std::vector<double> &yVec, double &upThresh, double &lowThresh,double & percent)
{
	//reicht diese Sortierung
	if (sortVec.size() < 10)
		return -1;

	std::sort(sortVec.begin(),sortVec.end() );

	//Eintrag nach den % niedrigsten Werten
	int iLow =static_cast<int>(( (static_cast<double>(sortVec.size()) / 100.0) * percent));
	//Eintrag nach den % hoechsten Werten
	int iUpper = sortVec.size() - iLow-1; // iLow kann 0 sein


	if ( (iUpper - iLow) > 3) //sonst sinnlos
	{
		upThresh  = sortVec[iUpper];
		lowThresh = sortVec[iLow];

		double upperVal = sortVec[iUpper-1];
		double lowerVal = sortVec[iLow+1];

		for(unsigned int j = 0; j<yVec.size();++j )
		{
		    	if( yVec[j] >= upThresh )
		    			yVec[j]=upperVal ;
		        if( yVec[j]<= lowThresh )
		    	      	yVec[j] =lowerVal ;
		}

		//Maximaler und minimaler Wert waeren damit:
		upThresh  = upperVal;
		lowThresh = lowerVal;

		return 1;
	}
	else
		return -1;

}



//aktuell nur fuer positive Werte ( pixel positionen: 0..max)
double ModelCurve::bufferMeanCorrection(std::vector<double> &yVec, double &minData, double &maxData, double & percent)
{

	int bufferSize = yVec.size();
	if (percent <= 0)
		return -1;

	if (bufferSize <= 10) //sonst sinnlos
		return -1;

	double sum = 0.0;
	int i = 0;
	for (i = 0; i < bufferSize; ++i)
	{
		sum += yVec[i];
	}

	if (i>0)
		sum = sum / static_cast<double>(i);
	else
		return -1;

	//percent in faktor:
	percent = percent/100.0;

	// Alle Werte ausserhalb max histo raus:
	double mean = sum;
	double distance = 0.0;
	double val = 0.0;
	double distanceKorr = 0.0;

	double low = 1000000.0;
	double high = -10000000.0;

	for (int j = 0; j<bufferSize; ++j)
	{
		val = yVec[j];
		distance = val - mean;
		distanceKorr = distance * percent;
		yVec[j] = mean + distanceKorr;

		if (yVec[j]>high)
			high = yVec[j];
		if (yVec[j]<low)
			low = yVec[j];

	}

	minData = low;
	maxData = high;

	return  mean;


}



double ModelCurve::calcMeanValue(std::vector<double> &rYVec)
{
	double mean = 0.0;
	unsigned int i = 0;
	for (i= 0; i < rYVec.size(); ++i)
	{
		mean += rYVec[i];
	}
	if (i>0)
	{
		mean /= static_cast<double>(i);
	}
	return(mean);
}


///////////////////////////////////////////////////////////////////////
// OwnBuffer
///////////////////////////////////////////////////////////////////////

SingleDataSet::SingleDataSet()
{
	data = 0;
	data_rank = 0;
	pos = 0;
	pos_rank = 0;
}

SingleDataSet::SingleDataSet(const SingleDataSet & anotherSet)
{
	data = anotherSet.data;
	data_rank = anotherSet.data_rank;
	pos = anotherSet.pos;
	pos_rank = anotherSet.pos_rank;
}

SingleDataSet::SingleDataSet(double data_, int data_rank_, double pos_, int pos_rank_)
{
	data=data_;
	data_rank = data_rank_;
	pos = pos_;
	pos_rank = pos_rank_;
}

TempBuffer::TempBuffer()
{
	reset();
}

void TempBuffer::reset()
{
	_dataSet.clear();
}

void TempBuffer::addOneDataSet(SingleDataSet set)
{
	_dataSet.push_back(set);
}

SingleDataSet TempBuffer::getOneDataSet(int i)
{
	if ( (i<0) || (i>=(int)_dataSet.size()) ) return SingleDataSet();
	return _dataSet[i];
}

//sind die Daten aufsteigend ?
bool TempBuffer::isSorted()
{
	for (int i=1; i<(int)_dataSet.size(); i++)
		if (_dataSet[i-1].pos > _dataSet[i].pos)
			return false;

	return true;
}

//gehen die Encoder Positionen ueber 360 grad?
bool TempBuffer::is360()
{
	double min = getMinPos();
	double max = getMaxPos();

	double totalDist = max - min;

	return (totalDist > _ticksPer360);
}

// Daten sortieren
void TempBuffer::sort()
{ // Bubble Sort
	int size = _dataSet.size();
	if (size <= 1) return;
	for (int i=0; i<size-1; i++)
	{
		for (int j=0; j<size-1-i; j++)
		{
			if (_dataSet[j].pos > _dataSet[j+1].pos) exchange(j, j+1);
		}
	}
}

void TempBuffer::exchange(int i, int j)
{
	if (i==j) return;
	if (i>=(int)_dataSet.size()) return;
	if (j>=(int)_dataSet.size()) return;

	SingleDataSet dataSet = _dataSet[i];
	_dataSet[i] = _dataSet[j];
	_dataSet[j] = dataSet;
}

//member setzen
void TempBuffer::setTicksPer360(int number)
{
	_ticksPer360 = number;
}


int TempBuffer::getCurrentSize()
{
	return _dataSet.size();
}

std::vector<double>  TempBuffer::getDataY()	//den Ergebniswert
{
	std::vector<double> vecY;

	unsigned int size = _dataSet.size();
	for(unsigned int i=0; i<size; ++i )
	{
		if(_dataSet[i].data_rank >0)
			vecY.push_back(_dataSet[i].data );
	}
	return vecY;
}

std::vector<double>  TempBuffer::getDataX()
{
	std::vector<double> vecX;

	unsigned int size = _dataSet.size();
	for(unsigned int i=0; i<size; ++i )
	{
		if(_dataSet[i].pos_rank >0)
			vecX.push_back(_dataSet[i].pos );
	}
	return vecX;

}



void TempBuffer::calcStartEndValues(int & startIndex, int & endIndex)
{
	int size = _dataSet.size();
	startIndex = 0;
	endIndex = size - 1;

	double minPos = getMinPos();
	double maxPos = getMaxPos();

	if (maxPos - minPos <= _ticksPer360) return;

	double startPos =  (maxPos - minPos - _ticksPer360) / 2 + minPos;
	double endPos = startPos + _ticksPer360;

	for (int i=0; i<size; i++)
	{
		if (_dataSet[i].pos >= startPos)
		{
			startIndex = i;
			break;
		}
	}

	for (int i=size-1; i>=0; i--)
	{
		if (_dataSet[i].pos <= endPos)
		{
			endIndex = i;
			break;
		}
	}
}

double TempBuffer::getMinPos()
{
	int size = _dataSet.size();
	if (size <= 0) return 0;
	double minPos = _dataSet[0].pos;
	for (int i=1; i<size; i++)
	{
		if (_dataSet[i].pos < minPos) minPos = _dataSet[i].pos;
	}
	return minPos;
}

double TempBuffer::getMaxPos()
{
	int size = _dataSet.size();
	if (size <= 0) return 0;
	double maxPos = _dataSet[size-1].pos;
	for (int i=0; i<size-1; i++)
	{
		if (_dataSet[i].pos > maxPos) maxPos = _dataSet[i].pos;
	}
	return maxPos;
}

void TempBuffer::reduceToStartEnd(int start, int end)
{
	int currentSize = _dataSet.size();
	if (end<=start) return; // Reduzierung unzulaessig
	if (end>=currentSize) return; //Reduzierung nicht moeglich, Ende ausserhalb
	if (start<0) return; //Reduzierung nicht moeglich, Start ausserhalb

	if (start==0) // Reduzierung nur hinten
	{
		_dataSet.resize(end + 1);
		return;
	}

	currentSize = end - start + 1;

	for (int i=0; i<currentSize; i++)
	{
		_dataSet[i]=_dataSet[i+start];
	}
	_dataSet.resize(currentSize);
}

bool TempBuffer::isInRange(double posTicks1, double posTicks2, double angleMax)
{
	if (std::abs(angleMax)<0.000001) return ( std::abs(posTicks1-posTicks2)<0.000001 );

	double ticksPerOneDegree = ((double)(_ticksPer360)) / 360.0;

	double posAngle1 = posTicks1 / ticksPerOneDegree;
	double posAngle2 = posTicks2 / ticksPerOneDegree;

	while (posAngle1 <    0) posAngle1 = posAngle1 + 360;
	while (posAngle2 <    0) posAngle2 = posAngle2 + 360;
	while (posAngle1 >= 360) posAngle1 = posAngle1 - 360;
	while (posAngle2 >= 360) posAngle2 = posAngle2 - 360;

	if (std::abs(posAngle1 - posAngle2) <= angleMax) return true;
	posAngle1 += 360.0;
	if (std::abs(posAngle1 - posAngle2) <= angleMax) return true;
	posAngle1 -= 360.0;
	posAngle2 += 360.0;
	if (std::abs(posAngle1 - posAngle2) <= angleMax) return true;
	return false;
}





int TempBuffer::round(double d)
{
	return (int)(d+0.5);
}

void TempBuffer::makePosModulo()
{
	for (int i=0; i<(int)_dataSet.size(); i++)
	{
		int pos_int = (int)(_dataSet[i].pos + 0.5);
		_dataSet[i].pos = pos_int % _ticksPer360;
	}
}



void TempBuffer::setDataForBadRank(int badRank, double dataToSet)
{
	for (int i = 0; i < getCurrentSize(); i++)
	{
		SingleDataSet & oneSet = _dataSet[i];
		if (oneSet.data_rank <= badRank)
		{
			oneSet.data = dataToSet;
		}
	}
}



void TempBuffer::interpolateLinearOnRange(int startLastValid, int endFirstValid)
{
	// Sicherheitschecks, dann evtl. nix machen
	if (startLastValid < 0) return;
	if (endFirstValid >= (int)_dataSet.size()) return;

	if (endFirstValid-startLastValid <= 1) return; // Ebenfalls nix machen (ist ja nix da)

	double startData = _dataSet[startLastValid].data;
	double startRank = _dataSet[startLastValid].data_rank;
	double startPos  = _dataSet[startLastValid].pos;

	double dataDiff = _dataSet[endFirstValid].data      - _dataSet[startLastValid].data;
	double rankDiff = _dataSet[endFirstValid].data_rank - _dataSet[startLastValid].data_rank;
	double posDiff  = _dataSet[endFirstValid].pos       - _dataSet[startLastValid].pos;

	double dataDiffPerTick = dataDiff / posDiff;
	double dataRankDiffPerTick = rankDiff / posDiff;

	for (int i=startLastValid+1; i<endFirstValid; i++) // jetzt alle Luecken fuellen
	{
		posDiff = _dataSet[i].pos - startPos; // neudef.
		_dataSet[i].data = startData + posDiff * dataDiffPerTick;
		_dataSet[i].data_rank = round(startRank + posDiff * dataRankDiffPerTick);
	}
}




} // namespace filter
} // namespace precitec
