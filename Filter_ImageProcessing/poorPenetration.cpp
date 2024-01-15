/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter tries to detect a poor penetration.
*/

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "poorPenetration.h"

// Konstante, wie viel Prozent von Ausreissern eliminiert werden
#define __PER_CENT_KILL 10

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string PoorPenetration::m_oFilterName = std::string("PoorPenetration");

		const std::string PoorPenetration::PIPENAME_RESULT_OUT = std::string("ResultOut");


		PoorPenetration::PoorPenetration() :
			TransformFilter(PoorPenetration::m_oFilterName, Poco::UUID{"74EDB01E-95F8-48CC-8550-B45B11144E26"}),
			m_pPipeInImageFrame(NULL),
			m_oMode(0),
			m_oDisplay(0),
			m_oWindowX(4),
			m_oWindowY(4),
			m_oAnzPosUnder(4),
			m_oAnzPosUpper(4),
			m_oLowerThreshold(50),
		    m_oUpperThreshold(100),
			m_oMaxWidth(40)
		   {

			m_pPipeOutResult = new SynchronePipe< interface::GeoPoorPenetrationCandidatearray >(this, PoorPenetration::PIPENAME_RESULT_OUT);


			// Set default values of the parameters of the filter
			parameters_.add("Mode", Parameter::TYPE_int, m_oMode);
			parameters_.add("Display", Parameter::TYPE_int, m_oDisplay);

			parameters_.add("WindowX", Parameter::TYPE_int, m_oWindowX);
			parameters_.add("WindowY", Parameter::TYPE_int, m_oWindowY);

			parameters_.add("AnzPosUpper", Parameter::TYPE_int, m_oAnzPosUpper);
			parameters_.add("AnzPosUnder", Parameter::TYPE_int, m_oAnzPosUnder);

			parameters_.add("LowerThreshold", Parameter::TYPE_int, m_oLowerThreshold);
			parameters_.add("UpperThreshold", Parameter::TYPE_int, m_oUpperThreshold);
			parameters_.add("MaxWidth", Parameter::TYPE_int, m_oMaxWidth);

            setInPipeConnectors({{Poco::UUID("BA148BCD-CB34-4BAF-87BC-49E215277C1F"), m_pPipeInImageFrame, "ImageFrameIn", 0, "ImageFrameIn"}});
            setOutPipeConnectors({{Poco::UUID("8994196E-D817-478A-A7F4-709BF868DD8B"), m_pPipeOutResult, PIPENAME_RESULT_OUT, 0, ""}});
            setVariantID(Poco::UUID("071777FA-50A9-4969-9802-1BE5D9D65E75"));
		}

		PoorPenetration::~PoorPenetration()
		{
			delete m_pPipeOutResult;
		}

		void PoorPenetration::setParameter()
		{
			TransformFilter::setParameter();
			m_oMode      = parameters_.getParameter("Mode").convert<int>();
			m_oDisplay   = parameters_.getParameter("Display").convert<int>();

			m_oWindowX   = parameters_.getParameter("WindowX").convert<int>();
			m_oWindowY   = parameters_.getParameter("WindowY").convert<int>();

			m_oAnzPosUpper = parameters_.getParameter("AnzPosUpper").convert<int>();
			m_oAnzPosUnder = parameters_.getParameter("AnzPosUnder").convert<int>();

			m_oUpperThreshold = parameters_.getParameter("UpperThreshold").convert<int>();
			m_oLowerThreshold = parameters_.getParameter("LowerThreshold").convert<int>();

			m_oMaxWidth       = parameters_.getParameter("MaxWidth").convert<int>();

		} // setParameter

		bool PoorPenetration::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);


			return BaseFilter::subscribe(p_rPipe, p_oGroup);

		} // subscribe

		void PoorPenetration::paint()
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
				OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

				// paint here

				const std::vector<PoorPenetrationPoint> & rPointList = _overlay.getPointContainer();
				const std::vector<PoorPenetrationLine> & rLineList = _overlay.getLineContainer();
				const std::vector<PoorPenetrationRectangle> & rRectangleList = _overlay.getRectangleContainer();

				for (auto && point : rPointList)
				{
					rLayerContour.add<OverlayPoint>(rTrafo(Point(point.x, point.y)), point.color);
				}

				for (auto && line: rLineList)
				{
					rLayerContour.add<OverlayLine>(rTrafo(Point(line.x1, line.y1)), rTrafo(Point(line.x2, line.y2)), line.color);
				}

				for (auto && rectangle : rRectangleList)
				{
					rLayerContour.add<OverlayRectangle>(rTrafo(Rect(rectangle.x, rectangle.y, rectangle.width, rectangle.height)), rectangle.color);
				}



			}
			catch (...)
			{
				return;
			}
		} // paint


		void PoorPenetration::proceed(const void* sender, fliplib::PipeEventArgs& e)
		{
			//poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor


			geo2d::PoorPenetrationCandidatearray oOutCandidate;


			// Read out image frame from pipe
			const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);

			try
			{
				m_oSpTrafo = rFrameIn.context().trafo();
				// Extract actual image and size
				const BImage &rImageIn = rFrameIn.data();

				// Read-out laserline
				//const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read();
				m_oSpTrafo = rFrameIn.context().trafo();
				//_overlay.setTrafo(*m_oSpTrafo);
				_overlay.reset();
				// And extract byte-array
				//const VecDoublearray& rLaserarray = rLaserLineIn.ref();
				// input validity check


				//if (inputIsInvalid(rFrameIn))
				if (false)
				{
					PoorPenetrationCandidate cand;
					oOutCandidate.getData().push_back(cand);
					oOutCandidate.getRank().push_back(0);

					const GeoPoorPenetrationCandidatearray &rCandidate = GeoPoorPenetrationCandidatearray(rFrameIn.context(), oOutCandidate, rFrameIn.analysisResult(), interface::NotPresent);

					preSignalAction();


					m_pPipeOutResult->signal(rCandidate);

					return; // RETURN
				}

				m_hasPainting = true;

				NoSeamFind noSeamFind(rImageIn);
				SDisplayBadPen displayBadPen[3];

				noSeamFind.CheckForErrors(displayBadPen, m_oDisplay, m_oMode, m_oWindowX, m_oWindowY,m_oAnzPosUpper,m_oAnzPosUnder,m_oLowerThreshold,m_oUpperThreshold,m_oMaxWidth);
				_overlay = noSeamFind.getOverlay();
				std::vector<PoorPenetrationCandidate> candidates = noSeamFind.getCandidates();

				for (unsigned int i = 0; i < candidates.size(); i++)
				{
					PoorPenetrationCandidate cand = candidates[i];
					oOutCandidate.getData().push_back(cand);
					oOutCandidate.getRank().push_back(255);
				}


				const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type


				GeoPoorPenetrationCandidatearray oCandidate = GeoPoorPenetrationCandidatearray(rFrameIn.context(), oOutCandidate, oAnalysisResult, filter::eRankMax);

				preSignalAction();


				m_pPipeOutResult->signal(oCandidate);


			}
			catch (...)
			{
				PoorPenetrationCandidate cand;
				oOutCandidate.getData().push_back(cand);
				oOutCandidate.getRank().push_back(0);



				const GeoPoorPenetrationCandidatearray &rCandidate = GeoPoorPenetrationCandidatearray(rFrameIn.context(), oOutCandidate, rFrameIn.analysisResult(), interface::NotPresent);

				preSignalAction();


				m_pPipeOutResult->signal(rCandidate);

				return;
			}
		} // proceedGroup

		//////////////////////////////////////////
		// RingBuffer
		//////////////////////////////////////////

		RingBuffer::RingBuffer()
		{
			reset(0);
			_isValid = false;
		}

		RingBuffer::~RingBuffer()
		{
		}

		void RingBuffer::reset(int size)
		{
			_maxSize = size;
			_isFull = false;
			_curPos = 0;
			_curSum = 0;
			_isValid = (size > 0);
		}

		void RingBuffer::addValue(int value)
		{
			if (_maxSize == 0) return; //RingPuffer ungueltig, da Groesse gleich Null
			if (!_isValid) return;

			if (_isFull) _curSum -= _ringBufferEntry[_curPos]; // wenn voll, zu ueberschreibendes Element abziehen
			_curSum += value;
			_ringBufferEntry[_curPos] = value;
			_curPos++;

			if (_curPos >= _maxSize) //RingPuffer voll!
				_isFull = true;
			_curPos %= _maxSize;
		}

		int RingBuffer::getMean()
		{
			if (!_isValid) return 0;
			if (!_isFull && (_curPos == 0)) return 0; // RingPuffer voellig leer

			//double sum = 0;

			//Anzahl gueltiger Eintraege bestimmen
			int maxIndex = _isFull ? _maxSize : _curPos;

			//kompletten RingPuffer aufsummieren
			//for (int i = 0; i<maxIndex; i++) sum += _ringBufferEntry[i];

			//Summe durch die Anzahl der Elemente teilen und zurueckgeben
			return ((int)(_curSum / maxIndex + 0.5));
		}

		bool RingBuffer::isFull()
		{
			return _isFull;
		}

		bool RingBuffer::isValid()
		{
			return _isValid;
		}

		/////////////////////////
		// Box-Summer
		/////////////////////////

		BoxSummer::BoxSummer(BImage image)
		{
			_image = image;

			_oldSum = 0;
			_oldX1 = _oldX2 = _oldY1 = _oldY2 = -100;

			_width = image.width();
			_height = image.height();
		}

		BoxSummer::~BoxSummer()
		{
		}

		long BoxSummer::getSum(int x1, int y1, int x2, int y2, int speederX, int speederY)
		{
			//Inits
			long sum = 0;
			int index = 0;
			int row, column;
			int diff = (x2 - x1 + 1);

			if ((speederX != 1) || (speederY != 1))
			{
				int counter = 0;
				int size = (y2 - y1 + 1)*(x2 - x1 + 1);
				for (row = y1; row <= y2; row += speederY)
				{
					for (column = x1; column <= x2; column += speederX)
					{
						sum += _image[row][column];
						//sum += pBild_[zeile * xAnz_ + spalte];
						counter++;
					}
				}
				double fac = ((double)(size)) / counter;
				sum = (int)(sum*fac + 0.5);
				_oldX1 = -100;
			}
			else
			{
				if ((_oldX1 + 1 == x1) && (_oldX2 + 1 == x2) && (_oldY1 == y1) && (_oldY2 == y2))
				{ // Kasten eins weiter => Sonderfall! Spalte links raus, Spalte rechts rein...
					column = _oldX1;
					sum = _oldSum;
					for (row = y1; row <= y2; row++)
					{
						//index = row * xAnz_ + column;
						sum -= _image[row][column]; //links raus
						sum += _image[row][column + diff]; //rechts rein
					}
				}
				else
				{ // Summe komplett neu berechnen

					for (row = y1; row <= y2; row++)
					{
						index = 0;
						for (column = x1; column <= x2; column++)
						{
							sum += _image[row][x1+index];
							index++;
						}
					}
				}
				_oldX1 = x1;
				_oldX2 = x2;
				_oldY1 = y1;
				_oldY2 = y2;
				_oldSum = sum;
			}
			return sum;
		}

		/////////////////////////////////////////
		// ImgSeamPos
		/////////////////////////////////////////

		ImgSeamPos::ImgSeamPos()
		{
			_pos.reserve(20);
			reset();
		}


		void ImgSeamPos::reset()
		{
			for (int i = 0; i<20; i++) { _pos[i] = 0; }
			_full = false;
			_sumPos = 0;
			_counter = 0;
		}

		bool ImgSeamPos::isFull()
		{
			return _full;
		}

		void ImgSeamPos::addPos(int pos)
		{
			if (_full)
			{
				int middlePos = getPos();
				if (std::abs(middlePos - pos)>100) //voll und grosse Abweichung
				{
					pos = (2 * middlePos + pos) / 3; // zum Mittelwert ziehen
				}
			}

			_sumPos -= _pos[_counter];

			_pos[_counter] = pos;

			_sumPos += pos;

			_counter++;
			if (_counter >= 20) _full = true;
			_counter = _counter % 20;
		}

		int ImgSeamPos::getPos()
		{
			if (_full)
			{
				return (_sumPos / 20);
			}
			else
			{
				return (_counter > 0) ? (_sumPos / _counter) : 0;
			}
		}

		////////////////////////////////////////
		// SeamPositions
		////////////////////////////////////////

		SeamPositions::SeamPositions()
		{
			imgSeamPos.reserve(500);
			reset();
		}

		void SeamPositions::reset()
		{
			for (int i = 0; i<500; i++) imgSeamPos[i].reset();
			_curImg = 0;
		}

		void SeamPositions::addPos(int pos, bool isFirst)
		{
			if (isFirst) _curImg = 0;
			imgSeamPos[_curImg].addPos(pos);
			_curImg++;
			_curImg %= 500;
		}

		void SeamPositions::addNoPos(bool isFirst)
		{
			if (isFirst) _curImg = 0;
			_curImg++;
			_curImg %= 500;
		}

		int SeamPositions::getPos()
		{
			int tmpNr = (_curImg + 499) % 500;
			if (imgSeamPos[tmpNr].isFull())
			{
				return imgSeamPos[tmpNr].getPos();
			}
			else
			{
				return -1;
			}
		}

		//////////////////////////////
		// ProductPositions
		//////////////////////////////

		ProductPositions::ProductPositions()
		{
			_seamPositions.reserve(18);
			reset();
		}

		void ProductPositions::reset()
		{
			for (int i = 0; i<18; i++) _seamPositions[i].reset();
		}

		void ProductPositions::addPos(int seamNr, int pos, bool isFirst)
		{

			_seamPositions[seamNr].addPos(pos, isFirst);
		}

		void ProductPositions::addNoPos(int seamNr, bool isFirst)
		{
			_seamPositions[seamNr].addNoPos(isFirst);
		}

		int ProductPositions::getPos(int seamNr)
		{
			return _seamPositions[seamNr].getPos();
		}

		////////////////////////////////////////
		// CLASS CIntStatistic1Dim
		////////////////////////////////////////

		Statistic1Dim::Statistic1Dim(int size, int noOfValues)
		{
			_valArray.reserve(size); // zum Halten der Werte, vielleicht ueberfluessig
			_bucket.reserve(noOfValues); // die Eimer fuer die Werte
			_maxSize = size; // speichert die MaxSize
			_curSize = 0; // aktuelle Anzahl reseten
			_numberOfValues = noOfValues;

			reset();
		}

		Statistic1Dim::~Statistic1Dim()
		{
		}

		void Statistic1Dim::reset()
		{
			for (int i = 0; i<_maxSize; i++) _valArray[i] = 0;
			for (int i = 0; i<_numberOfValues; i++) _bucket[i] = 0;

			_curSize = 0;
		}

		int Statistic1Dim::addValue(int value)
		{
			if (_curSize >= _maxSize) return -1; // bereits voll
			_valArray[_curSize] = value;
			_bucket[value]++;
			_curSize++;
			return 0;
		}

		int Statistic1Dim::getNumber(int place)
		{
			if (place<1) return -1;
			if (place>_curSize) return -1;

			//printf("Place: %d\n", place);

			bool bBreak = false;
			int counter = -1;
			int sum = 0;

			while (!bBreak && (counter<_numberOfValues))
			{
				counter++;
				sum += _bucket[counter];
				if (sum >= place) bBreak = true;
			}

			//printf("Break %d, counter %d\n", bBreak, counter);

			if (bBreak) return counter; else return -1;
		}

		int Statistic1Dim::getMedian()
		{
			if ((_curSize % 2) == 0) // akt. Groesse durch 2 teilbar, gerade
			{
				int one = getNumber(_curSize / 2);
				if (one == -1) printf("Fehler!\n");
				int two = getNumber(_curSize / 2 + 1);
				if (two == -1) printf("Fehler!\n");
				return (one + two) / 2;
			}
			else // ungerade
			{
				return getNumber(_curSize / 2 + 1);
			}
		}

		// ***********************************************************
		// *           Class GeradenRegression                      *
		// ***********************************************************


		GeradenRegression::GeradenRegression()
		{
			reset();
		}

		GeradenRegression::~GeradenRegression()
		{

		}

		void GeradenRegression::reset()
		{
			_iAnzahl = 0;
			_sx = _sy = _sxx = _syy = _sxy = 0;
			_bErgebnis = false;
		}

		void GeradenRegression::calcGerade(double &m, double &b)
		{
			if (_iAnzahl<2)
			{
				_m = m = 1000000000;
				_b = b = 1000000000;
				_bErgebnis = false;
			}

			b = _b = (_sy*_sxx - _sxy*_sx) / (_iAnzahl*_sxx - _sx*_sx);
			m = _m = (_sy - _b*_iAnzahl) / _sx;
			_bErgebnis = true;
		}

		double GeradenRegression::getAbstand(double c1, double c2)
		{
			if (!_bErgebnis) return 100000000;
			double fx = _m*c1 + _b;
			double fehler;
			fehler = (fx>c2) ? fx - c2 : c2 - fx;
			return fehler;
		}

		double GeradenRegression::getError()
		{
			if (!_bErgebnis) return 1000000000;

			double fehler;
			//	fehler = m_m*m_m*Sxx + m_b*m_m*Sx - m_m*Sxy + m_b*Sx + m_iAnzahl*m_b*m_b
			//			- m_b*Sy - m_m*Sxy - m_b*Sy + Syy;

			fehler = _syy + _m*_m*_sxx + _iAnzahl*_b*_b - 2 * _m*_sxy - 2 * _b*_sy + 2 * _m*_b*_sx;
			if (fehler<0) printf("ist negativ\n");
			fehler = sqrt(fehler / _iAnzahl);

			return fehler;
		}

		void GeradenRegression::addPoint(double x, double y)
		{
			_iAnzahl++;
			_bErgebnis = false;

			_sx += x;
			_sxx += x*x;
			_sy += y;
			_syy += y*y;
			_sxy += x*y;
		}

		bool GeradenRegression::ergebnisDa()
		{
			return _bErgebnis;
		}

		void GeradenRegression::calcGerade()
		{
			if (_iAnzahl<2)
			{
				_m = 1000000000;
				_b = 1000000000;
				_bErgebnis = false;
			}

			_b = (_sy*_sxx - _sxy*_sxx) / (_iAnzahl*_sxx - _sx*_sx);
			_m = (_sy - _b*_iAnzahl) / _sx;
			_bErgebnis = true;
		}

		double GeradenRegression::getSteigung()
		{
			if (!_bErgebnis) return 1000000000;
			else return _m;
		}

		double GeradenRegression::getYAbschnitt()
		{
			if (!_bErgebnis) return 1000000000;
			else return _b;
		}

		//////////////////////////////////////
		// Median
		//////////////////////////////////////

		SimpleMedian::SimpleMedian(int size, int noOfValues)
		{
			_valArray.reserve(size);
			_bucket.reserve(noOfValues);
			_maxSize = size;
			_curSize = 0;
			_numberOfValues = noOfValues;

			resetBuckets();
			resetArray();

		}

		SimpleMedian::~SimpleMedian()
		{
		}

		void SimpleMedian::resetBuckets(void)
		{
			for (int i = 0; i<_numberOfValues; i++) _bucket[i] = 0;
		}

		void SimpleMedian::resetArray(void)
		{
			for (int i = 0; i<_maxSize; i++) _valArray[i] = 0;
			_curSize = 0;
			_curMin = _numberOfValues - 1;
			_curMax = 0;
			_sum = 0;
		}


		int SimpleMedian::addValue(int i)
		{
			if (i<0) return -1;
			if (i >= _numberOfValues) return -1;
			if (_curSize >= _maxSize) return -1;

			_valArray[_curSize] = i;
			_curSize++;
			_bucket[i]++;

			_sum += i;

			if (i>_curMax) _curMax = i;
			if (i<_curMin) _curMin = i;

			return 0;
		}


		int SimpleMedian::getNumber(int place)
		{
			if (place<1) return -1;
			if (place>_curSize) return -1;

			//printf("Place: %d\n", place);

			bool bBreak = false;
			int counter = -1;
			int sum = 0;

			while (!bBreak && (counter<_numberOfValues))
			{
				counter++;
				sum += _bucket[counter];
				if (sum >= place) bBreak = true;
			}

			//printf("Break %d, counter %d\n", bBreak, counter);

			if (bBreak) return counter; else return -1;
		}


		int SimpleMedian::getMedian()
		{
			if ((_curSize % 2) == 0) // akt. Groesse durch 2 teilbar, gerade
			{
				int one = getNumber(_curSize / 2);
				if (one == -1) printf("Fehler!\n");
				int two = getNumber(_curSize / 2 + 1);
				if (two == -1) printf("Fehler!\n");
				return (one + two) / 2;
			}
			else // ungerade
			{
				return getNumber(_curSize / 2 + 1);
			}
		}


		double SimpleMedian::getMean()
		{
			return (_sum / _curSize);
		}

		void SNoSeamResult::Reset()
		{
			bFilled = false;

			NoSeamProb = 0;
			box_x1 = 0;
			box_y1 = 0;
			box_x2 = 0;
			box_y2 = 0;

			left_x = 0;
			right_x = 0;
			width = 0;

			StdAbwLinks = 0;
			StdAbwRechts = 0;
			StdAbwBreite = 0;

			Gradient = 0;

			greyvalAreaIn = 0;
			greyvalAreaOut = 0;
			greyvalChain = 0; //

			LauflaengeLinks = 0;
			LauflaengeRechts = 0;

			nr = 0;
		}

		//----------------------------------------------------------------------------
		// Class:  NoSeamFind
		//----------------------------------------------------------------------------



		NoSeamFind::NoSeamFind(BImage image)
		{
			//SetBildPointer(pBild);
			_image = image;
			_width = image.width();
			_height = image.height();
			Reset();
		}

		NoSeamFind::NoSeamFind()
		{
			Reset();
		}

		NoSeamFind::~NoSeamFind()
		{
			// Do nothing
		}

		void NoSeamFind::setImage(BImage image)
		{
			_image = image;
			_width = image.width();
			_height = image.height();
		}

		// Die Hauptroutine
		// Macht die eigentliche Ueberpruefung
		void NoSeamFind::CheckForErrors(SDisplayBadPen displayBadPen[], int display, int mode, int windowX, int windowY, int anzPosUpper, int anzPosUnder,int lowerThresh,int upperThresh, int maxWidth)
		{
			_windowX = windowX;
			_windowY = windowY;
			_anzPosUpper = anzPosUpper;
			_anzPosUnder = anzPosUnder;
			_lowerThreshold = lowerThresh;
			_upperThreshold = upperThresh;
			_minimaWidth = maxWidth;

			//bool bIsDeveloper = false;
			_displayParameter = display;

            //Aus dem ROI am Rand noch mal 5 Spalten/Zeilen wegnehmen
			int y1 = 5;
			int y2 = _height-5;
			int x1 = 5;
			int x2 = _width-5;

			// Kontur bestimmen mittels dunkler Stellen
			FillKonturMulti(x1, x2, y1, y2,mode); // kostet Zeit (etwa die Haelfte)




			//if (mode == 2) return 0; //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (_displayParameter == 121)
			{
				DrawKontur_2();
			}
			else if (_displayParameter == 131)
			{
				DrawKontur_3();
			}
			else if (_displayParameter == 191)
			{
				DrawKontur_1();
				DrawKontur_2();
				DrawKontur_3();
			}
			else if (_displayParameter == 141)
			{
				DrawXPos();
			}
			else if (_displayParameter == 151)
			{
				DrawXPosKanten();
			}


			// Einzelne Minima killen (alle 3 Minima-Gruppen)

			int seamParIRobRes = 10; // !!!!
			KillSingleMinima(seamParIRobRes);  // Reserve 2 nicht mehr noetig

			//if (mode == 3) return 0; //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (_displayParameter == 12)
			{
				DrawKontur_1();
			}

			// DrawXPos();

			if (_displayParameter == 122)
			{
				DrawKontur_2();
			}
			else if (_displayParameter == 132)
			{
				DrawKontur_3();
			}
			else if (_displayParameter == 192)
			{
				DrawKontur_1();
				DrawKontur_2();
				DrawKontur_3();
			}
			else if (_displayParameter == 142)
			{
				DrawXPos();
			}
			else if (_displayParameter == 152)
			{
				DrawXPosKanten();
			}

			// Minima checken:
			// 1. alle moeglichst nach links schieben
			PackMultiMinima();

			if (_displayParameter == 162)
			{
				DrawXPosKanten();
			}

			// 2. wenn mehrere  =>  eliminieren! Es gibt dann nur noch Minimas in Gruppe 1
			EliminateMultiMinima(_displayParameter);  // Res1 nicht mehr noetig

			if (_displayParameter == 143)
			{
				DrawXPosMin1();
			}
			else if (_displayParameter == 153)
			{
				DrawXPosMin1Kanten();
			}

			// In Ketten zusammenfassen, es gibt max. ANZAHL_KETTEN Ketten
			int SeamParIRobRes2 = 11;
			SearchForChainsCont(SeamParIRobRes2);

			if (_displayParameter == 13)
			{
				DrawKontur_1();
			}

			if (_displayParameter == 144)
			{
				DrawXPosMin1(); //nicht implementiert
			}
			else if (_displayParameter == 154)
			{
				DrawXPosMin1Kanten(); //nicht implementiert
			}

			// Geradenregression durch die Konturpunkte
			// Sucht m & b von der Geraden zu jeder Kette,
			// um Luecken in den gefundenen Ketten kuenstlich
			// mit einem Dummy-Minimum zu schliessen
			CalcChainsMB();

			// Punkte, die nicht zu Ketten gehoeren, raus
			// Luecken in Ketten mit Dummy-Minima fuellen
			DeleteAllButChains();

			if (_displayParameter == 14)
			{
				DrawKontur_1();
			}

			if (_displayParameter == 145)
			{
				DrawXPosMin1(); //nicht implementiert
			}
			else if (_displayParameter == 155)
			{
				DrawXPosMin1Kanten(); //nicht implementiert
			}


			// Aus Kanten Li + Re der Ketten: Pos min + max in X
			// =>  wenn gross = zappeln = IO !!
			// Nur die laengste Kette wird berechnet. Die anderen werden geloescht!
			int SeamParIRobRes = 16;
			CalcChainsKanten(SeamParIRobRes);

			if (_displayParameter == 146)
			{
				DrawKettePos();
			}
			else if (_displayParameter == 156)
			{
				DrawKetteKante();
			}

			DrawKettePos(); //????

			// Umgebende Rechtecke berechnen
			//CalcBoxes(SeamPar.IRob.iDisplay[0] == 15);
			CalcBoxes(false);

			// Fuer jede Kette die Daten/Parameter berechnen
			for (int i = 0; i < ANZAHL_KETTEN; i++) // kostet etwa die Haelfte der Zeit
			{
				NoSeamResult[i].Reset();
				NoSeamResult[i].nr = i;

				// Berechne die Daten von Kette i
				CalcBoxMulti(i);

				// Auswertung der Kette i
				if (NoSeamResult[i].bFilled)
				{
					CheckBoxMulti(i);

					// Pruefe die Daten von Kette i mit den 3 Parameter-Gruppen
					//LK[i].isOK = ((GetResultFromCode(0, NoSeamResult[i].result) != 1)  // Check mit Parameter von Gruppe 1
					//	&& (GetResultFromCode(1, NoSeamResult[i].result) != 1)  // Check mit Parameter von Gruppe 2
					//	&& (GetResultFromCode(2, NoSeamResult[i].result) != 1)  // Check mit Parameter von Gruppe 3
					//	);

				}
			}

			// Wenn nicht 'umgebendes Rechteck anzeigen', dann die Ketten-Box anzeigen
			//if (_displayParameter)
			//	DrawBoxes(_displayParameter == 15);

			DrawBoxes(true);
		}


		// Berechnet die Konturpunkte mittels ScanLine. Input = ROI.
		void NoSeamFind::FillKonturMulti(int x1, int x2, int y1, int y2, const int &mode)
		{
			int  iMin1XL=0;
			int  iMin1XR=0;
			int  iMin1Pos=0;
			int  iMin1Val=0;
			int  iMin2XL=0;
			int  iMin2XR=0;
			int  iMin2Pos=0;
			int  iMin2Val=0;
			int  iMin3XL=0;
			int  iMin3XR=0;
			int  iMin3Pos=0;
			int  iMin3Val=0;

			int  iCol = 0;
			int  iDisplayLine = 0;

			const int sizeY = _windowY;

            // Anzeige der Intensitaet eines ausgewaehlten Streifens
            // Display > 1000:  die hinteren 3 Ziffern geben die 'Streifen-Nummer'
            if (_displayParameter > 1000)
            {
                iDisplayLine = _displayParameter % 1000;
            }

			for (int zeile = y1; zeile < (y2 - (sizeY-1)); zeile += 10)
			{
				//Streifen anzeigen, wenn ausgewaehlter Streifen = dem akt. Streifen
				//bDisplayStreifen = (iDisplayLine == iStreifen);

				//sucht 3 minima pro Zeile
				ScanLineMulti(zeile,
					x1, x2,
					iMin1XL, iMin1XR,
					iMin1Pos, iMin1Val,
					iMin2XL, iMin2XR,
					iMin2Pos, iMin2Val,
					iMin3XL, iMin3XR,
					iMin3Pos, iMin3Val,
					iDisplayLine,
					mode
					);

				//traegt die minima in konturen ein, prueft auf Abstaende usw.
				AddPointMulti(zeile,
					iMin1XL, iMin1XR,
					iMin1Pos, iMin1Val,
					iMin2XL, iMin2XR,
					iMin2Pos, iMin2Val,
					iMin3XL, iMin3XR,
					iMin3Pos, iMin3Val,
					iCol
					);
			}
		}


		/**********************************************************************
		* Description:  Scannt eine Linie im Bild durch und sucht bis zu     *
		*               3 Minima.                                           *
		*                                                                    *
		* Parameter:    line:         Y-Position des Streifens               *
		*               startx:       X-Position (absolut) Streifenanfang    *
		*               endx:         X-Position (absolut) Streifenende      *
		*               iMinxPos:     X-Position (absolut) des x-ten Min's   *
		*               iMinxVal:     Grauwert des x-ten Min's (1 Pixel)     *
		*               bDisplayStreifen:  Den Streifen graphisch anzeigen   *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::ScanLineMulti(int line,
			int startx, int endx,
			int &iMin1XL, int &iMin1XR,
			int &iMin1Pos, int &iMin1Val,
			int &iMin2XL, int &iMin2XR,
			int &iMin2Pos, int &iMin2Val,
			int &iMin3XL, int &iMin3XR,
			int &iMin3Pos, int &iMin3Val,
			int iDisplayLine,
			const int& mode
			)
		{
			// Ablage der gefilterten Intensitaeten des Streifens
			int SummenPixel[1000];
			int Anzahl = 0;
			int summe = 0;
			// Filtern der gefundenen 4 x 4 - Intensitaeten
			int FDS_FILTER_SIZE = _windowX * _windowY;

			int MinWert = 255 * FDS_FILTER_SIZE;// 16;
			int MinIndex = 0;


			int spaltenzaehler;

			// Kleinster gefundener Intensitaetswert des Streifens
			int iMinScan = 255 * FDS_FILTER_SIZE; //16
			// Groesster gefundener Intensitaetswert  des Streifens
			int iMaxScan = 0;
			// Schwellen fuer Minimum-Suche


			int iSchwelleScanUp;
			int iSchwelleScanLo;
			int iFDSSumme;

			// 'Breite'-Schwellen fuer Minima-Suche
			const int ANZ_POS_UNTER_SCHWELLE = _anzPosUnder;  // test
			const int ANZ_POS_UEBER_SCHWELLE = _anzPosUpper;


			//Eingangsdaten checken
			if (startx > endx)
			{
				int tmp;
				tmp = startx;
				startx = endx;
				endx = tmp;
			}

			if (startx < 0)         startx = 0;
			if (endx > _width)      endx = _width;
			if (line < 0)           line = 0;
			if (line > _height - 10) line = _height - 10;

			for (int o = 0; o<1000; o++) SummenPixel[o] = 0;

			for (spaltenzaehler =startx + _windowX/2; spaltenzaehler < endx - _windowX/2; spaltenzaehler++)
			{


				summe = 0;
				for (int m = 0; m < _windowY; ++m)
					for (int n = spaltenzaehler - _windowX/2; n < spaltenzaehler + ((_windowX + 1)) / 2; ++n)
					    summe += _image[line + m][n];

				SummenPixel[spaltenzaehler]=summe;
				iFDSSumme = summe;
				Anzahl++;  // Zaehlt die Summenauswertungen in einer Zeile

				// Kleinste Intensitaet suchen
				if (iFDSSumme < iMinScan)
					iMinScan = iFDSSumme;

				// Groesste Intensitaet suchen
				if (iFDSSumme > iMaxScan)
					iMaxScan = iFDSSumme;

				//if (summe<MinWert)
				if (iFDSSumme < MinWert)
				{
					//neues Max
					//MinWert  = summe;  // neuer kleinster Grauwert
					MinWert = iFDSSumme;  // neuer kleinster Grauwert
					MinIndex = spaltenzaehler;// -sizeX / 2;  // x-Pos neuer minGrauwwert
				}

				// Anzeige der Intensitaet eines ausgewaehlten Streifens
				if (iDisplayLine)
				{
					if (line==iDisplayLine) //Der Startwert im ROI
					_overlay.addPoint(spaltenzaehler, iFDSSumme/FDS_FILTER_SIZE, Color::Yellow());

				}

			}//spaltenzaehler

			// Min und Max aus obigem Streifen-Scan  =>  nochmals durch und schauen,
			// ob mehr als 1 Minimum da

			if ((((iMaxScan - iMinScan) / FDS_FILTER_SIZE) < 30) || mode == 1)
			{
				// Alte Methode: nimm das kleinste gefundene Min
				iMin1XL = MinIndex;
				iMin1XR = MinIndex;
				iMin1Pos = MinIndex;
				iMin1Val = MinWert / FDS_FILTER_SIZE;// 16;
				iMin2Pos = 0;
				iMin3Pos = 0;
				for (unsigned int m= 0; m<5;++m)
				    _overlay.addPoint(iMin1Pos,line+m, Color::Yellow());


				return;
			}
			else if (mode==0)
			{
				iSchwelleScanUp = int(0.50 * (iMaxScan - iMinScan) + iMinScan);
				iSchwelleScanLo = int(0.33 * (iMaxScan - iMinScan) + iMinScan);
			}
			else
			{
				iSchwelleScanLo = _lowerThreshold*FDS_FILTER_SIZE;
				iSchwelleScanUp = _upperThreshold*FDS_FILTER_SIZE;
			}

			// Variablen fuer Minimum-Suche
			int iFDSMinStatus = 0;
			iMin1Pos = 0;
			iMin1Val = FDS_FILTER_SIZE * 256;
			iMin2Pos = 0;
			iMin2Val = FDS_FILTER_SIZE * 256;
			iMin3Pos = 0;
			iMin3Val = FDS_FILTER_SIZE * 256;
			int iMin4XL = 0;
			int iMin4XR = 0;
			int iMin4Pos = 0;
			int iMin4Val = FDS_FILTER_SIZE * 256;
			int iAnzPosUeberSchwelle = 0;
			int iAnzPosUnterSchwelle = 0;

			for (spaltenzaehler = startx + 2; spaltenzaehler <= endx - 2; spaltenzaehler++)
			{

				// Anfang der Suche  =>  ueber Schwelle oben?
				if (iFDSMinStatus == 0)
				{
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						iFDSMinStatus = 1;
					}
					else
					{
						// Ist noch unter der Schwelle!
						// => warte
					}
				}

				// Hohe Intensitaet
				else if (iFDSMinStatus == 1)
				{
					// Unter Schwelle unten?
					if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						iMin1XL = spaltenzaehler;
						iMin1Pos = spaltenzaehler;
						iMin1Val = SummenPixel[spaltenzaehler];
						iAnzPosUnterSchwelle = 1;
						iFDSMinStatus = 2;
					}
				}

				// Unter Schwelle unten
				else if (iFDSMinStatus == 2)
				{
					// Wieder ueber Schwelle unten?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						// Wieder ueber Schwelle unten.
						iMin1XR = spaltenzaehler;
						iFDSMinStatus = 3;
					}
					else
					{
						if (SummenPixel[spaltenzaehler] < iMin1Val)
						{
							// Neues Min 1
							iMin1Pos = spaltenzaehler;
							iMin1Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
					}
				}

				// Zwischen Schwelle unten und oben
				else if (iFDSMinStatus == 3)
				{
					// Wieder ueber Schwelle oben?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						// Ist Min 1 OK?
						if (iAnzPosUnterSchwelle >= ANZ_POS_UNTER_SCHWELLE)
						{
							// Ist gutes Min
							iAnzPosUeberSchwelle = 1;
							iFDSMinStatus = 4;
						}
						else
						{
							// Kein brauchbares Min  =>  nochmals starten
							iMin1Pos = 0;
							iMin1Val = FDS_FILTER_SIZE * 256;
							iFDSMinStatus = 1;
						}
					}
					// Wieder unter Schwelle unten?
					else if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						if (SummenPixel[spaltenzaehler] < iMin1Val)
						{
							// Neues Min 1
							iMin1Pos = spaltenzaehler;
							iMin1Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 2;
					}
				}

				// Ueber Schwelle oben, genuegend lange?
				else if (iFDSMinStatus == 4)
				{
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						iAnzPosUeberSchwelle++;
						if (iAnzPosUeberSchwelle >= ANZ_POS_UEBER_SCHWELLE)
						{
							// Gutes Min 1
							//printf("ipNSF:  Min 1 Pos %d Wert %d\n", iMin1Pos, iMin1Val);
							iFDSMinStatus = 5;
						}
					}
					// Wieder zwischen Schwelle unten und oben?
					else if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						iFDSMinStatus = 3;
					}
					else
					{
						// Noch kein gutes Min !!
						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 2;
					}
				}

				// Hohe Intensitaet nach Min 1
				else if (iFDSMinStatus == 5)
				{
					// Unter Schwelle unten?
					if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						iMin2XL = spaltenzaehler;
						iMin2Pos = spaltenzaehler;
						iMin2Val = SummenPixel[spaltenzaehler];
						iAnzPosUnterSchwelle = 1;
						iFDSMinStatus = 6;
					}
				}

				// Unter Schwelle unten
				else if (iFDSMinStatus == 6)
				{
					// Wieder ueber Schwelle unten?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						// Wieder ueber Schwelle unten.
						iMin2XR = spaltenzaehler;
						iFDSMinStatus = 7;
					}
					else
					{
						if (SummenPixel[spaltenzaehler] < iMin2Val)
						{
							// Neues Min 2
							iMin2Pos = spaltenzaehler;
							iMin2Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
					}
				}

				// Zwischen Schwelle unten und oben
				else if (iFDSMinStatus == 7)
				{
					// Wieder ueber Schwelle oben?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						// Ist Min 2 OK?
						if (iAnzPosUnterSchwelle >= ANZ_POS_UNTER_SCHWELLE)
						{
							// Ist gutes Min
							iAnzPosUeberSchwelle = 1;
							iFDSMinStatus = 8;
						}
						else
						{
							// Kein brauchbares Min  =>  nochmals starten
							iMin2Pos = 0;
							iMin2Val = FDS_FILTER_SIZE * 256;
							iFDSMinStatus = 5;
						}
					}
					// Wieder unter Schwelle unten?
					else if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						if (SummenPixel[spaltenzaehler] < iMin2Val)
						{
							// Neues Min 2
							iMin2Pos = spaltenzaehler;
							iMin2Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 6;
					}
				}

				// Ueber Schwelle oben, genuegend lange?
				else if (iFDSMinStatus == 8)
				{
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						iAnzPosUeberSchwelle++;
						if (iAnzPosUeberSchwelle >= ANZ_POS_UEBER_SCHWELLE)
						{
							// Gutes Min 2
							//printf("ipNSF:  Min 2 Pos %d Wert %d\n", iMin2Pos, iMin2Val);
							iFDSMinStatus = 9;
						}
					}
					// Wieder zwischen Schwelle unten und oben?
					else if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						iFDSMinStatus = 7;
					}
					else
					{
						// Noch kein gutes Min !!
						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 6;
					}
				}

				// Hohe Intensitaet nach Min 2
				else if (iFDSMinStatus == 9)
				{
					// Unter Schwelle unten?
					if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						iMin3XL = spaltenzaehler;
						iMin3Pos = spaltenzaehler;
						iMin3Val = SummenPixel[spaltenzaehler];
						iAnzPosUnterSchwelle = 1;
						iFDSMinStatus = 10;
					}
				}

				// Unter Schwelle unten
				else if (iFDSMinStatus == 10)
				{
					// Wieder ueber Schwelle unten?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						// Wieder ueber Schwelle unten.
						iMin3XR = spaltenzaehler;
						iFDSMinStatus = 11;
					}
					else
					{
						if (SummenPixel[spaltenzaehler] < iMin3Val)
						{
							// Neues Min 3
							iMin3Pos = spaltenzaehler;
							iMin3Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
					}
				}

				// Zwischen Schwelle unten und oben
				else if (iFDSMinStatus == 11)
				{
					// Wieder ueber Schwelle oben?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						// Ist Min 3 OK?
						if (iAnzPosUnterSchwelle >= ANZ_POS_UNTER_SCHWELLE)
						{
							// Ist gutes Min
							iAnzPosUeberSchwelle = 1;
							iFDSMinStatus = 12;
						}
						else
						{
							// Kein brauchbares Min  =>  nochmals starten
							iMin3Pos = 0;
							iMin3Val = FDS_FILTER_SIZE * 256;
							iFDSMinStatus = 9;
						}
					}
					// Wieder unter Schwelle unten?
					else if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						if (SummenPixel[spaltenzaehler] < iMin3Val)
						{
							// Neues Min 3
							iMin3Pos = spaltenzaehler;
							iMin3Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 10;
					}
				}

				// Ueber Schwelle oben, genuegend lange?
				else if (iFDSMinStatus == 12)
				{
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						iAnzPosUeberSchwelle++;
						if (iAnzPosUeberSchwelle >= ANZ_POS_UEBER_SCHWELLE)
						{
							// Gutes Min 3
							//printf("ipNSF:  Min 3 Pos %d Wert %d\n", iMin3Pos, iMin3Val);
							iFDSMinStatus = 13;
						}
					}
					// Wieder zwischen Schwelle unten und oben?
					else if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						iFDSMinStatus = 11;
					}
					else
					{
						// Noch kein gutes Min !!
						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 10;
					}
				}

				// Hohe Intensitaet nach Min 3
				else if (iFDSMinStatus == 13)
				{
					if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						iMin4XL = spaltenzaehler;
						iMin4Pos = spaltenzaehler;
						iMin4Val = SummenPixel[spaltenzaehler];
						iAnzPosUnterSchwelle = 1;
						iFDSMinStatus = 14;
					}
				}

				// Unter Schwelle unten
				else if (iFDSMinStatus == 14)
				{
					// Wieder ueber Schwelle unten?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						// Wieder ueber Schwelle unten.
						iMin4XR = spaltenzaehler;
						iFDSMinStatus = 15;
					}
					else
					{
						if (SummenPixel[spaltenzaehler] < iMin4Val)
						{
							// Neues Min 4
							iMin4Pos = spaltenzaehler;
							iMin4Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
					}
				}

				// Zwischen Schwelle unten und oben
				else if (iFDSMinStatus == 15)
				{
					// Wieder ueber Schwelle oben?
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						// Ist Min 4 OK?
						if (iAnzPosUnterSchwelle >= ANZ_POS_UNTER_SCHWELLE)
						{
							// Ist gutes Min
							iAnzPosUeberSchwelle = 1;
							iFDSMinStatus = FDS_FILTER_SIZE;// 16;
						}
						else
						{
							// Kein brauchbares Min  =>  nochmals starten
							iMin4Pos = 0;
							iMin4Val = FDS_FILTER_SIZE * 256;
							iFDSMinStatus = 13;
						}
					}
					// Wieder unter Schwelle unten?
					else if (SummenPixel[spaltenzaehler] < iSchwelleScanLo)
					{
						if (SummenPixel[spaltenzaehler] < iMin4Val)
						{
							// Neues Min 4
							iMin4Pos = spaltenzaehler;
							iMin4Val = SummenPixel[spaltenzaehler];
						}

						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 14;
					}
				}

				// Ueber Schwelle oben, genuegend lange?
				else if (iFDSMinStatus == 16)
				{
					if (SummenPixel[spaltenzaehler] >= iSchwelleScanUp)
					{
						iAnzPosUeberSchwelle++;
						if (iAnzPosUeberSchwelle >= ANZ_POS_UEBER_SCHWELLE)
						{
							// Gutes Min 4
							//printf("ipNSF:  Min 4 Pos %d Wert %d\n", iMin4Pos, iMin4Val);
							// Min 4 pruefen:  ist's besser als eines der 3 anderen?
							// =>  ersetzen
							// Ist Min 1 das schwaechste?
							if ((iMin4Val < iMin1Val)
								&& (iMin1Val < iMin2Val)
								&& (iMin1Val < iMin3Val)
								)
							{
								// Min 1 ersetzen
								iMin1XL = iMin2XL;
								iMin1XR = iMin2XR;
								iMin1Pos = iMin2Pos;
								iMin1Val = iMin2Val;
								iMin2XL = iMin3XL;
								iMin2XR = iMin3XR;
								iMin2Pos = iMin3Pos;
								iMin2Val = iMin3Val;
								iMin3XL = iMin4XL;
								iMin3XR = iMin4XR;
								iMin3Pos = iMin4Pos;
								iMin3Val = iMin4Val;
							}
							// Ist Min 2 das schwaechste?
							else if ((iMin4Val < iMin2Val)
								&& (iMin2Val < iMin1Val)
								&& (iMin2Val < iMin3Val)
								)
							{
								// Min 2 ersetzen
								iMin2XL = iMin3XL;
								iMin2XR = iMin3XR;
								iMin2Pos = iMin3Pos;
								iMin2Val = iMin3Val;
								iMin3XL = iMin4XL;
								iMin3XR = iMin4XR;
								iMin3Pos = iMin4Pos;
								iMin3Val = iMin4Val;
							}
							// Ist Min 3 das schwaechste?
							else if ((iMin4Val < iMin3Val)
								&& (iMin3Val < iMin1Val)
								&& (iMin3Val < iMin2Val)
								)
							{
								// Min 3 ersetzen
								iMin3XL = iMin4XL;
								iMin3XR = iMin4XR;
								iMin3Pos = iMin4Pos;
								iMin3Val = iMin4Val;
							}

							// Naechstes Min 4 suchen
							iFDSMinStatus = 13;
						}
					}
					// Wieder zwischen Schwelle unten und oben?
					else if (SummenPixel[spaltenzaehler] >= iSchwelleScanLo)
					{
						iFDSMinStatus = 15;
					}
					else
					{
						// Noch kein gutes Min !!
						iAnzPosUnterSchwelle++;
						iFDSMinStatus = 14;
					}
				}

			}  // for (spaltenzaehler=startx+2; spaltenzaehler<=endx-2; spaltenzaehler++)


			// Auswertung des End-Status
			// -------------------------

			//Filtertest:
			//iFDSMinStatus = 17;

			switch (iFDSMinStatus)
			{
			case 1:
			case 2:
			{
				// Ist kein gutes Min 1
				iMin1Pos = 0;
			}
				break;

			case 3:
			case 4:
			{
				if (iAnzPosUnterSchwelle < ANZ_POS_UNTER_SCHWELLE)
				{
					// Ist kein gutes Min 1
					iMin1Pos = 0;
				}
			}
				break;

			case 5:
			case 6:
			{
				// Ist kein gutes Min 2
				iMin2Pos = 0;
			}
				break;

			case 7:
			case 8:
			{
				if (iAnzPosUnterSchwelle < ANZ_POS_UNTER_SCHWELLE)
				{
					// Ist kein gutes Min 2
					iMin2Pos = 0;
				}
			}
				break;

			case 9:
			case 10:
			{
				// Ist kein gutes Min 3
				iMin3Pos = 0;
			}
				break;

			case 11:
			case 12:
			{
				if (iAnzPosUnterSchwelle < ANZ_POS_UNTER_SCHWELLE)
				{
					// Ist kein gutes Min 3
					iMin3Pos = 0;
				}
			}
				break;

			case 13:
			case 14:
				// Ist kein gutes Min 4
				break;

			case 15:
			case 16:
			{
				if (iAnzPosUnterSchwelle >= ANZ_POS_UNTER_SCHWELLE)
				{
					// Ist gutes Min 4  =>  pruefen: ist's besser als eines der 3
					// anderen?  =>  ersetzen
					// Ist Min 1 das schwaechste?
					if ((iMin4Val < iMin1Val)
						&& (iMin1Val < iMin2Val)
						&& (iMin1Val < iMin3Val)
						)
					{
						// Min 1 ersetzen
						iMin1XL = iMin2XL;
						iMin1XR = iMin2XR;
						iMin1Pos = iMin2Pos;
						iMin1Val = iMin2Val;
						iMin2XL = iMin3XL;
						iMin2XR = iMin3XR;
						iMin2Pos = iMin3Pos;
						iMin2Val = iMin3Val;
						iMin3XL = iMin4XL;
						iMin3XR = iMin4XR;
						iMin3Pos = iMin4Pos;
						iMin3Val = iMin4Val;
					}
					// Ist Min 2 das schwaechste?
					else if ((iMin4Val < iMin2Val)
						&& (iMin2Val < iMin1Val)
						&& (iMin2Val < iMin3Val)
						)
					{
						// Min 2 ersetzen
						iMin2XL = iMin3XL;
						iMin2XR = iMin3XR;
						iMin2Pos = iMin3Pos;
						iMin2Val = iMin3Val;
						iMin3XL = iMin4XL;
						iMin3XR = iMin4XR;
						iMin3Pos = iMin4Pos;
						iMin3Val = iMin4Val;
					}
					// Ist Min 3 das schwaechste?
					else if ((iMin4Val < iMin3Val)
						&& (iMin3Val < iMin1Val)
						&& (iMin3Val < iMin2Val)
						)
					{
						// Min 3 ersetzen
						iMin3XL = iMin4XL;
						iMin3XR = iMin4XR;
						iMin3Pos = iMin4Pos;
						iMin3Val = iMin4Val;
					}
				}  // if (iAnzPosUnterSchwelle >= ANZ_POS_UNTER_SCHWELLE)
			}
				break;

			case 17:
				break;

			default:
			{
				printf("FDS: ScanLineMulti, bad state (%d)!\n", iFDSMinStatus);
			}
			}  // switch ( iFDSMinStatus )


			// Auswertung der Min's
			// --------------------
			// Min 1 gefunden?
			if (iMin1Pos)
			{
				if (_displayParameter == 101)
				{
					for (int i = 0; i < _windowY; i++)
						_overlay.addPoint(iMin1Pos, line+i, Color::Green());

				}

				iMin1Val /= FDS_FILTER_SIZE; // 16;
			}

			// Min 2 gefunden?
			if (iMin2Pos)
			{
				if (_displayParameter == 101)
				{
					for (int i = 0; i < _windowY; i++)
						_overlay.addPoint(iMin2Pos, line+i, Color::Red());

				}

				iMin2Val /= FDS_FILTER_SIZE;// 16;
			}

			// Min 3 gefunden?
			if (iMin3Pos)
			{
				if (_displayParameter == 101)
				{
					for (int i = 0; i < _windowY; i++)
						_overlay.addPoint(iMin3Pos, line+i, Color::Cyan());


				}

				iMin3Val /= FDS_FILTER_SIZE;
			}
		}  // ScanLineMulti




		/**********************************************************************
		* Description:  Fuellt die mit "ScanLine" gefundenen Minimas (max. 3 *
		*               Stueck) von einer Zeile in die Kontur-Liste ein.     *
		*                                                                    *
		* Parameter:    zeile:        Y-Position des Streifens               *
		*               iMin n XL:    Linke Kante von Min n                  *
		*               iMin n XL:    Rechte Kante von Min n                 *
		*               iMin n Pos:   X-Position (absolut) des n-ten Min's   *
		*               iMin n Val:   Grauwert des n-ten Min's (1 Pixel)     *
		*               iDispCol:     Anzeige-farbe des Min's:               *
		*                             0 = gruen = OK                         *
		*                             1 = rot   = FDS                        *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::AddPointMulti(int zeile,
			int iMin1XL, int iMin1XR,
			int iMin1Pos, int iMin1Val,
			int iMin2XL, int iMin2XR,
			int iMin2Pos, int iMin2Val,
			int iMin3XL, int iMin3XR,
			int iMin3Pos, int iMin3Val,
			int iDispCol
			)
		{
			//const int MIN_BREITE_MIN = 35;
			//CB 40 ist notwendig, da sonst breitere FDS nicht erkannt wird
            // maximal erlaubte Breite in Pixel (Kante rechts-links) des Minimum
			const int MIN_BREITE_MIN = _minimaWidth;
			// Distanz von Min zu next Min in x-Richtung
			//minimal geforderter Abstand zwischen den einzelnen Minima pro Zeile
			const int CHECK_DIST = 5;

			if (iKonturAnzahl == ANZAHL_KONTUR)
			{
				printf("Fehler in NoSeamFind: zu viele Konturen!\n");
				return;
			}


			// Gefundene Min's ablegen
			// -----------------------

			// Pruefen, ob mehr als 1 Min:
			// Ja  =>  bei allen pruefen, ob Breite Min > 30?  =>  loeschen und
			//         links-schieben
			// Nein  =>  Min lassen

			// Min 3 da?

			// Init fuer Min 3
			Kontur3[iKonturAnzahl].zeile = zeile;     // Zeile des Minimum, in 10-er Schritten
			Kontur3[iKonturAnzahl].DispCol = iDispCol;  // 0 = gruen = OK, 1 = rot = FDS

			if ((iMin3Pos)
				&& ((iMin3XR - iMin3XL) < MIN_BREITE_MIN)  // Min nicht zu 'breit' ?
				)
			{
				Kontur3[iKonturAnzahl].XL = iMin3XL;   // Linke Seite von Min
				Kontur3[iKonturAnzahl].XR = iMin3XR;   // Rechte Seite von Min
				Kontur3[iKonturAnzahl].xPos = iMin3Pos;  // xPos von Grauwertmin
				Kontur3[iKonturAnzahl].GW = iMin3Val;  // GrauwertMinimum
				Kontur3[iKonturAnzahl].Del = false;     // Diesen Kontur-Wert NICHT auslassen
				Kontur3[iKonturAnzahl].KetteGruppeNach = 0; // Kein 'nachfolgendes passendes' Minimum
			}
			else
			{
				// Kein (gutes) Min 3
				Kontur3[iKonturAnzahl].xPos = 0;      // xPos von Grauwertmin
				Kontur3[iKonturAnzahl].Del = true;   // Diesen Kontur-Wert auslassen
			}

			// Min 2 da?

			// Init fuer Min 2
			Kontur2[iKonturAnzahl].zeile = zeile;     // Zeile des Minimum, in 10-er Schritten
			Kontur2[iKonturAnzahl].DispCol = iDispCol;  // 0 = gruen = OK, 1 = rot = FDS

			if ((iMin2Pos)
				&& ((iMin2XR - iMin2XL) < MIN_BREITE_MIN)  // Min nicht zu 'breit' ?
				)
			{
				Kontur2[iKonturAnzahl].XL = iMin2XL;   // Linke Seite von Min
				Kontur2[iKonturAnzahl].XR = iMin2XR;   // Rechte Seite von Min
				Kontur2[iKonturAnzahl].xPos = iMin2Pos;  // xPos von Grauwertmin
				Kontur2[iKonturAnzahl].GW = iMin2Val;  // GrauwertMinimum
				Kontur2[iKonturAnzahl].Del = false;     // Diesen Kontur-Wert NICHT auslassen
				Kontur2[iKonturAnzahl].KetteGruppeNach = 0; // Kein 'nachfolgendes passendes' Minimum
			}
			else if (Kontur3[iKonturAnzahl].xPos)  // War ein Min 3 da?
			{
				// Es gibt ein Min 3 !!  =>  Dieses hierher verschieben
				Kontur2[iKonturAnzahl].XL = iMin3XL;   // Linke Seite von Min
				Kontur2[iKonturAnzahl].XR = iMin3XR;   // Rechte Seite von Min
				Kontur2[iKonturAnzahl].xPos = iMin3Pos;  // xPos von Grauwertmin
				Kontur2[iKonturAnzahl].GW = iMin3Val;  // GrauwertMinimum
				Kontur2[iKonturAnzahl].Del = false;     // Diesen Kontur-Wert NICHT auslassen
				Kontur2[iKonturAnzahl].KetteGruppeNach = 0; // Kein 'nachfolgendes passendes' Minimum

				// Min 3 loeschen!
				Kontur3[iKonturAnzahl].xPos = 0;      // xPos von Grauwertmin
				Kontur3[iKonturAnzahl].Del = true;   // Diesen Kontur-Wert auslassen
			}
			else
			{
				// Kein (gutes) Min 2
				Kontur2[iKonturAnzahl].xPos = 0;      // xPos von Grauwertmin
				Kontur2[iKonturAnzahl].Del = true;   // Diesen Kontur-Wert auslassen
			}

			// Min 1 da?

			// Init fuer Min 1
			Kontur1[iKonturAnzahl].zeile = zeile;     // Zeile des Minimum, in 10-er Schritten
			Kontur1[iKonturAnzahl].DispCol = iDispCol;  // 0 = gruen = OK, 1 = rot = FDS

			if ((iMin1Pos)
				&& ((iMin1XR - iMin1XL) < MIN_BREITE_MIN)  // Min nicht zu 'breit' ?
				)
			{
				Kontur1[iKonturAnzahl].XL = iMin1XL;   // Linke Seite von Min
				Kontur1[iKonturAnzahl].XR = iMin1XR;   // Rechte Seite von Min
				Kontur1[iKonturAnzahl].xPos = iMin1Pos;  // xPos von Grauwertmin
				Kontur1[iKonturAnzahl].GW = iMin1Val;  // GrauwertMinimum
				Kontur1[iKonturAnzahl].Del = false;     // Diesen Kontur-Wert NICHT auslassen
				Kontur1[iKonturAnzahl].KetteGruppeNach = 0; // Kein 'nachfolgendes passendes' Minimum
			}
			else if (Kontur2[iKonturAnzahl].xPos)  // War ein Min 2 da?
			{
				// Min 2 auf Min 1 verschieben
				// Achtung! Min 2 koennte 'verschobenes' Min 3 sein
				Kontur1[iKonturAnzahl].XL = Kontur2[iKonturAnzahl].XL;   // Linke Seite von Min
				Kontur1[iKonturAnzahl].XR = Kontur2[iKonturAnzahl].XR;   // Rechte Seite von Min
				Kontur1[iKonturAnzahl].xPos = Kontur2[iKonturAnzahl].xPos;  // xPos von Grauwertmin
				Kontur1[iKonturAnzahl].GW = Kontur2[iKonturAnzahl].GW;  // GrauwertMinimum
				Kontur1[iKonturAnzahl].Del = false;     // Diesen Kontur-Wert NICHT auslassen
				Kontur1[iKonturAnzahl].KetteGruppeNach = 0; // Kein 'nachfolgendes passendes' Minimum

				// Min 2 loeschen!
				Kontur2[iKonturAnzahl].xPos = 0;      // xPos von Grauwertmin
				Kontur2[iKonturAnzahl].Del = true;   // Diesen Kontur-Wert auslassen

				// Gibt's ein Min 3?  =>  auf Min 2 verschieben!
				if (Kontur3[iKonturAnzahl].xPos)
				{
					// Es gibt ein Min 3 !!  =>  Dieses jetzt auf Min 2 verschieben
					Kontur2[iKonturAnzahl].XL = iMin3XL;   // Linke Seite von Min
					Kontur2[iKonturAnzahl].XR = iMin3XR;   // Rechte Seite von Min
					Kontur2[iKonturAnzahl].xPos = iMin3Pos;  // xPos von Grauwertmin
					Kontur2[iKonturAnzahl].GW = iMin3Val;  // GrauwertMinimum
					Kontur2[iKonturAnzahl].Del = false;     // Diesen Kontur-Wert NICHT auslassen
					Kontur2[iKonturAnzahl].KetteGruppeNach = 0; // Kein 'nachfolgendes passendes' Minimum

					// Min 3 loeschen!
					Kontur3[iKonturAnzahl].xPos = 0;      // xPos von Grauwertmin
					Kontur3[iKonturAnzahl].Del = true;   // Diesen Kontur-Wert auslassen
				}
			}
			else
			{
				// Kein (gutes) Min 1
				Kontur1[iKonturAnzahl].xPos = 0;      // xPos von Grauwertmin
				Kontur1[iKonturAnzahl].Del = true;   // Diesen Kontur-Wert auslassen
			}


			if (_displayParameter == 102)
			{
				if (Kontur1[iKonturAnzahl].xPos)
				{
					_overlay.addPoint(Kontur1[iKonturAnzahl].xPos, zeile, Color::Green());
					_overlay.addPoint(Kontur1[iKonturAnzahl].xPos, zeile+1, Color::Green());
					_overlay.addPoint(Kontur1[iKonturAnzahl].xPos, zeile+2, Color::Green());
					_overlay.addPoint(Kontur1[iKonturAnzahl].xPos, zeile+3, Color::Green());


				}
				if (Kontur2[iKonturAnzahl].xPos)
				{
					_overlay.addPoint(Kontur2[iKonturAnzahl].xPos, zeile, Color::Red());
					_overlay.addPoint(Kontur2[iKonturAnzahl].xPos, zeile + 1, Color::Red());
					_overlay.addPoint(Kontur2[iKonturAnzahl].xPos, zeile + 2, Color::Red());
					_overlay.addPoint(Kontur2[iKonturAnzahl].xPos, zeile + 3, Color::Red());


				}
				if (Kontur3[iKonturAnzahl].xPos)
				{
					_overlay.addPoint(Kontur3[iKonturAnzahl].xPos, zeile, Color::Cyan());
					_overlay.addPoint(Kontur3[iKonturAnzahl].xPos, zeile + 1, Color::Cyan());
					_overlay.addPoint(Kontur3[iKonturAnzahl].xPos, zeile + 2, Color::Cyan());
					_overlay.addPoint(Kontur3[iKonturAnzahl].xPos, zeile + 3, Color::Cyan());


				}
			}


			// Minimas pruefen
			// ***************

			// Wenn schon ein Min zuvor passend war, Gruppe des passenden merken
			// Passendes Min = Position der beiden Kanten "aehnlich" oder
			//                 Position der Min's und einer Kante "aehnlich"

#if DEBUGV || DEBUGV_GLOBAL
			const int CheckKont = 20;  // Zeile - 1
#endif

			// Min's von erster Zeile = 1. Kontur
			if (iKonturAnzahl == 0)
			{
#if DEBUGV || DEBUGV_GLOBAL
				if (Kontur1[0].xPos)
				{
					if (0 == CheckKont)
						printf("= Min 1 = %d\n", Kontur1[0].xPos);
				}
				if (Kontur2[0].xPos)
				{
					if (0 == CheckKont)
						printf("= Min 2 = %d\n", Kontur2[0].xPos);
				}
				if (Kontur3[0].xPos)
				{
					if (0 == CheckKont)
						printf("= Min 3 = %d\n", Kontur3[0].xPos);
				}
#endif

				if (Kontur1[0].xPos)
				{
					// Das erste Min der neuen Kette (Gruppe 1)
					Kontur1[0].KetteNr = 1;
					Kontur1[0].KetteGruppeVor = 0;
				}
				if (Kontur2[0].xPos)
				{
					// Das erste Min der neuen Kette (Gruppe 2)
					Kontur2[0].KetteNr = 1;
					Kontur2[0].KetteGruppeVor = 0;
				}
				if (Kontur3[0].xPos)
				{
					// Das erste Min der neuen Kette (Gruppe 3)
					Kontur3[0].KetteNr = 1;
					Kontur3[0].KetteGruppeVor = 0;
				}
			}

			else  // Wenn nicht die 1. Kontur
			{

				// 3 neue Minimas
				// --------------

				// Wenn 3 Min's da  =>  alle pruefen
				if (Kontur3[iKonturAnzahl].xPos)
				{
					// Passt Min 3 bereits gut zu Gruppe 3?
					if ((Kontur3[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 3 zuvor ein Min?
                        && noBadMinimum(Kontur3[iKonturAnzahl], Kontur3[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 3: OK\n");
#endif
						Kontur3[iKonturAnzahl].KetteNr = Kontur3[iKonturAnzahl - 1].KetteNr + 1;
						Kontur3[iKonturAnzahl].KetteGruppeVor = 3;

						// Jetzt Min 2 pruefen
						// Passt Min 2 bereits gut zu Gruppe 2?
						if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                            && noBadMinimum(Kontur2[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 3: OK, Min 2: OK\n");
#endif
							Kontur2[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 2;

							// Jetzt noch Min 1 pruefen
							// Passt Min 1 gut zu Gruppe 1?
							if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: OK, Min 2: OK, Min 1: OK\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
							}
							// Min 1 passt nicht zu Gruppe 1
							else
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: OK, Min 2: OK, Min 1: ?\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
							}
						}
						// Passt Min 2 besser zu Gruppe 1?
						else if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                            && noBadMinimum(Kontur2[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 3: OK, Min 2: 2 => 1, Min 1 ?\n");
#endif
							Kontur2[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 1;

							// =>  Min 1 MUSS neu sein!
							Kontur1[iKonturAnzahl].KetteNr = 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
						}
						// Min 2 passt nicht speziell zu Gruppe 1 oder 2
						else
						{
							Kontur2[iKonturAnzahl].KetteNr = 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 0;

							// Passt Min 1 bereits gut zu Gruppe 1?
							if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: OK, Min 2: ?, Min 1: OK\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
							}
							// Passt Min 1 besser zu Gruppe 2?
							else if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: OK, Min 2: ?, Min 1 => 2\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 2;
							}
							// Min 1 passt nicht zu Gruppe 1 oder 2
							else
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: OK, Min 2: ?, Min 1: ?\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
							}
						}  // else  Min 2
					}  // Min 3 passt bereits gut zu Gruppe 3

					// Passt Min 3 besser zu Gruppe 2?
					else if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                        && noBadMinimum(Kontur3[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 3: 3 => 2\n");
#endif
						Kontur3[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
						Kontur3[iKonturAnzahl].KetteGruppeVor = 2;

						// Jetzt Min 2 pruefen
						// Passt Min 2 gut zu Gruppe 1?
						if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                            && noBadMinimum(Kontur2[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 3: 3 => 2, Min 2: 2 => 1, Min 1 ?\n");
#endif
							Kontur2[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 1;

							// =>  Min 1 MUSS neu sein!
							Kontur1[iKonturAnzahl].KetteNr = 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
						}
						// Min 2 passt nicht speziell zu Gruppe 1
						else
						{
							Kontur2[iKonturAnzahl].KetteNr = 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 0;

							// Passt Min 1 gut zu Gruppe 1?
							if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: 3 => 2, Min 2: ?, Min 1: OK\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
							}
							// Min 1 passt nicht zu Gruppe 1
							else
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: 3 => 2, Min 2: ?, Min 1: ?\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
							}
						}  // else  Min 2
					}  // else if Min 3  =>  Gruppe 2

					// Passt Min 3 besser zu Gruppe 1?
					else if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                        && noBadMinimum(Kontur3[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 3: 3 => 1, Min 2 ?, Min 1 ?\n");
#endif
						Kontur3[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
						Kontur3[iKonturAnzahl].KetteGruppeVor = 1;

						// =>  Min 2 MUSS neu sein!
						Kontur2[iKonturAnzahl].KetteNr = 1;
						Kontur2[iKonturAnzahl].KetteGruppeVor = 0;

						// =>  Min 1 MUSS neu sein!
						Kontur1[iKonturAnzahl].KetteNr = 1;
						Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
					}

					// Min 3 passt nicht speziell zu Gruppe 1, 2 oder 3
					else
					{
						Kontur3[iKonturAnzahl].KetteNr = 1;
						Kontur3[iKonturAnzahl].KetteGruppeVor = 0;

						// Passt Min 2 bereits gut zu Gruppe 2?
						if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                            && noBadMinimum(Kontur2[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 3: ?, Min 2: OK\n");
#endif
							Kontur2[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 2;

							// Jetzt noch Min 1 pruefen
							// Passt Min 1 gut zu Gruppe 1?
							if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: OK, Min 1: OK\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
							}
							// Min 1 passt nicht zu Gruppe 1
							else
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: OK, Min 1: ?\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
							}
						}
						// Passt Min 2 besser zu Gruppe 1?
						else if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                            && noBadMinimum(Kontur2[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 3: ?, Min 2: 2 => 1, Min 1 ?\n");
#endif
							Kontur2[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 1;

							// =>  Min 1 MUSS neu sein!
							Kontur1[iKonturAnzahl].KetteNr = 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
						}
						// Passt Min 2 besser zu Gruppe 3?
						else if ((Kontur3[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 3 zuvor ein Min?
                            && noBadMinimum(Kontur2[iKonturAnzahl], Kontur3[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 3: ?, Min 2: 2 => 3\n");
#endif
							Kontur2[iKonturAnzahl].KetteNr = Kontur3[iKonturAnzahl - 1].KetteNr + 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 3;

							// Passt Min 1 bereits gut zu Gruppe 1?
							if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: 2 => 3, Min 1: OK\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
							}
							// Passt Min 1 besser zu Gruppe 2?
							else if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: 2 => 3, Min 1 => 2\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 2;
							}
							// Min 1 passt nicht zu Gruppe 1 oder 2
							else
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: 2 => 3, Min 1: ?\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
							}
						}
						// Min 2 passt nicht speziell zu Gruppe 1, 2 oder 3
						else
						{
							Kontur2[iKonturAnzahl].KetteNr = 1;
							Kontur2[iKonturAnzahl].KetteGruppeVor = 0;

							// Passt Min 1 bereits gut zu Gruppe 1?
							if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: ?, Min 1: OK\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
							}
							// Passt Min 1 besser zu Gruppe 2?
							else if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: ?, Min 1 => 2\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 2;
							}
							// Passt Min 1 besser zu Gruppe 3?
							else if ((Kontur3[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 3 zuvor ein Min?
                                && noBadMinimum(Kontur1[iKonturAnzahl], Kontur3[iKonturAnzahl - 1], CHECK_DIST)
								)
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: ?, Min 1 => 3\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = Kontur3[iKonturAnzahl - 1].KetteNr + 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 3;
							}
							// Min 1 passt nicht zu Gruppe 1, 2 oder 3
							else
							{
#if DEBUGV || DEBUGV_GLOBAL
								if (iKonturAnzahl == CheckKont)
									printf("= Min 3: ?, Min 2: ?, Min 1: ?\n");
#endif
								Kontur1[iKonturAnzahl].KetteNr = 1;
								Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
							}
						}  // else  Min 2
					}  // else  Min 3

				}  // Wenn 3 Min's


				// 2 neue Minimas
				// --------------

				// Wenn 2 Min's da  =>  Min 1 und 2 testen
				else if (Kontur2[iKonturAnzahl].xPos)
				{
					// Passt Min 2 bereits gut zu Gruppe 2?
					if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                        && noBadMinimum(Kontur2[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 2: OK\n");
#endif
						Kontur2[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
						Kontur2[iKonturAnzahl].KetteGruppeVor = 2;

						// Jetzt noch Min 1 pruefen
						// Passt Min 1 gut zu Gruppe 1?
						if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                            && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: OK, Min 1: OK\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
						}
						// Min 1 passt nicht zu Gruppe 1
						else
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: OK, Min 1: ?\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
						}
					}
					// Passt Min 2 besser zu Gruppe 1?
					else if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                        && noBadMinimum(Kontur2[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 2: 2 => 1, Min 1 ?\n");
#endif
						Kontur2[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
						Kontur2[iKonturAnzahl].KetteGruppeVor = 1;

						// =>  Min 1 MUSS neu sein!
						Kontur1[iKonturAnzahl].KetteNr = 1;
						Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
					}
					// Passt Min 2 besser zu Gruppe 3?
					else if ((Kontur3[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 3 zuvor ein Min?
                        && noBadMinimum(Kontur2[iKonturAnzahl], Kontur3[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 2: 2 => 3\n");
#endif
						Kontur2[iKonturAnzahl].KetteNr = Kontur3[iKonturAnzahl - 1].KetteNr + 1;
						Kontur2[iKonturAnzahl].KetteGruppeVor = 3;

						// Passt Min 1 bereits gut zu Gruppe 1?
						if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                            && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: 2 => 3, Min 1: OK\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
						}
						// Passt Min 1 besser zu Gruppe 2?
						else if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                            && noBadMinimum(Kontur1[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: 2 => 3, Min 1 => 2\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 2;
						}
						// Min 1 passt nicht zu Gruppe 1 oder 2
						else
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: 2 => 3, Min 1: ?\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
						}
					}
					// Min 2 passt nicht speziell zu Gruppe 1, 2 oder 3
					else
					{
						Kontur2[iKonturAnzahl].KetteNr = 1;
						Kontur2[iKonturAnzahl].KetteGruppeVor = 0;

						// Passt Min 1 bereits gut zu Gruppe 1?
						if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                            && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: ?, Min 1: OK\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
						}
						// Passt Min 1 besser zu Gruppe 2?
						else if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                            && noBadMinimum(Kontur1[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: ?, Min 1 => 2\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 2;
						}
						// Passt Min 1 besser zu Gruppe 3?
						else if ((Kontur3[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 3 zuvor ein Min?
                            && noBadMinimum(Kontur1[iKonturAnzahl], Kontur3[iKonturAnzahl - 1], CHECK_DIST)
							)
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: ?, Min 1 => 3\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = Kontur3[iKonturAnzahl - 1].KetteNr + 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 3;
						}
						// Min 1 passt nicht zu Gruppe 1, 2 oder 3
						else
						{
#if DEBUGV || DEBUGV_GLOBAL
							if (iKonturAnzahl == CheckKont)
								printf("= Min 2: ?, Min 1: ?\n");
#endif
							Kontur1[iKonturAnzahl].KetteNr = 1;
							Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
						}
					}  // else  Min 2
				}  // Wenn 2 Min's


				// 1 neues Minimum
				// ---------------

				// Wenn nur Min 1 da  =>  Min 1 testen
				else if (Kontur1[iKonturAnzahl].xPos)
				{
					// Passt Min 1 bereits gut zu Gruppe 1?
					if ((Kontur1[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 1 zuvor ein Min?
                        && noBadMinimum(Kontur1[iKonturAnzahl], Kontur1[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 1: OK\n");
#endif
						Kontur1[iKonturAnzahl].KetteNr = Kontur1[iKonturAnzahl - 1].KetteNr + 1;
						Kontur1[iKonturAnzahl].KetteGruppeVor = 1;
					}
					// Passt Min 1 besser zu Gruppe 2?
					else if ((Kontur2[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 2 zuvor ein Min?
                        && noBadMinimum(Kontur1[iKonturAnzahl], Kontur2[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 1 => 2\n");
#endif
						Kontur1[iKonturAnzahl].KetteNr = Kontur2[iKonturAnzahl - 1].KetteNr + 1;
						Kontur1[iKonturAnzahl].KetteGruppeVor = 2;
					}
					// Passt Min 1 besser zu Gruppe 3?
					else if ((Kontur3[iKonturAnzahl - 1].xPos)   // Hatte Gruppe 3 zuvor ein Min?
                        && noBadMinimum(Kontur1[iKonturAnzahl], Kontur3[iKonturAnzahl - 1], CHECK_DIST)
						)
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 1 => 3\n");
#endif
						Kontur1[iKonturAnzahl].KetteNr = Kontur3[iKonturAnzahl - 1].KetteNr + 1;
						Kontur1[iKonturAnzahl].KetteGruppeVor = 3;
					}
					// Min 1 passt nicht zu Gruppe 1, 2 oder 3
					else
					{
#if DEBUGV || DEBUGV_GLOBAL
						if (iKonturAnzahl == CheckKont)
							printf("= Min 1: ?\n");
#endif
						Kontur1[iKonturAnzahl].KetteNr = 1;
						Kontur1[iKonturAnzahl].KetteGruppeVor = 0;
					}
				}  // Wenn nur Min 1

				// Gar kein brauchbares Min von ScanLine!!
#if DEBUGV || DEBUGV_GLOBAL
				else
					if (iKonturAnzahl == CheckKont)
						printf("= Kein Min ?\n");
#endif

			}  // if (iKonturAnzahl > 0)

			iKonturAnzahl++;
		}


		/**********************************************************************
		* Description:  Loescht Punkte mit zu wenig Nachbarn.                *
		*                                                                    *
		* Parameter:    iMinMinima:   Min. Anzahl Minimas fuer Kette         *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::KillSingleMinima(int iMinMinima)
		{
			const int MIN_FOR_MINIMA_KETTE = 4;

			// Von hinten her 'aufraeumen', was zu klein ist
			for (int i = iKonturAnzahl - 1; i >= 0; i--)
			{

#if DEBUGV_KILL_SINGLE || DEBUGV_GLOBAL
				printf("V%2d: ", i);
				if (!Kontur1[i].Del)
					printf("no1 %2d v1 %3d n1 %3d ", Kontur1[i].KetteNr, Kontur1[i].KetteGruppeVor, Kontur1[i].KetteGruppeNach);
				else
					printf("no1 -- v1 --- n1 --- ");
				if (!Kontur2[i].Del)
					printf("no2 %2d v2 %3d n2 %3d ", Kontur2[i].KetteNr, Kontur2[i].KetteGruppeVor, Kontur2[i].KetteGruppeNach);
				else
					printf("no2 -- v1 --- n1 --- ");
				if (!Kontur3[i].Del)
					printf("no3 %2d v3 %3d n3 %3d ", Kontur3[i].KetteNr, Kontur3[i].KetteGruppeVor, Kontur3[i].KetteGruppeNach);
				else
					printf("no3 -- v1 --- n1 --- ");
				printf("\n");
#endif

				// Min Gruppe 1
				if (!Kontur1[i].Del)  // Wenn nicht bereits draussen
				{
					// Wenn schon als 'zu einer Kette gehoerend' markiert
					// =>  sein lassen
					if ((Kontur1[i].KetteGruppeNach)
						// Neue Kette: lange genug?
						|| (Kontur1[i].KetteNr >= MIN_FOR_MINIMA_KETTE)
						)
					{
						// Ist's nicht das erste in der Kette  =>  den Vorgaenger
						// als 'zu einer Kette gehoerend' markieren!
						if (Kontur1[i].KetteNr == 1)
						{
							// Ist das erste Ketten-Element  =>  Do nothing!
						}
						else if (i == 0)
						{
							printf("Fehler in NoSeamFind: Erste Zeile Kontur1 zeigt ueber ROI (KetteNr = %d)\n", Kontur1[i].KetteNr);
						}
						else if (Kontur1[i].KetteGruppeVor == 1)
						{
							// Verbunden mit Gruppe 1
							Kontur1[i - 1].KetteGruppeNach = 1;
						}
						else if (Kontur1[i].KetteGruppeVor == 2)
						{
							// Verbunden mit Gruppe 2
							Kontur2[i - 1].KetteGruppeNach = 1;
						}
						else if (Kontur1[i].KetteGruppeVor == 3)
						{
							// Verbunden mit Gruppe 3
							Kontur3[i - 1].KetteGruppeNach = 1;
						}
						else
						{
							printf("Fehler in NoSeamFind: Gruppen-Kette Kontur1 abgebrochen bei Zeile %d (1)\n", i);
						}
					}
					// Ist von zu kurzem Ketten-Teil!  =>  dieses Min loeschen!
					else
					{
						Kontur1[i].Del = true;
					}
				}

				// Min Gruppe 2
				if (!Kontur2[i].Del)  // Wenn nicht bereits draussen
				{
					// Wenn schon als 'zu einer Kette gehoerend' markiert
					// =>  sein lassen
					if ((Kontur2[i].KetteGruppeNach)
						// Neue Kette: lange genug?
						|| (Kontur2[i].KetteNr >= MIN_FOR_MINIMA_KETTE)
						)
					{
						// Ist's nicht das erste in der Kette  =>  den Vorgaenger
						// als 'zu einer Kette gehoerend' markieren!
						if (Kontur2[i].KetteNr == 1)
						{
							// Ist das erste Ketten-Element  =>  Do nothing!
						}
						else if (i == 0)
						{
							printf("Fehler in NoSeamFind: Erste Zeile Kontur2 zeigt ueber ROI (KetteNr = %d)\n", Kontur2[i].KetteNr);
						}
						else if (Kontur2[i].KetteGruppeVor == 1)
						{
							// Verbunden mit Gruppe 1
							Kontur1[i - 1].KetteGruppeNach = 2;
						}
						else if (Kontur2[i].KetteGruppeVor == 2)
						{
							// Verbunden mit Gruppe 2
							Kontur2[i - 1].KetteGruppeNach = 2;
						}
						else if (Kontur2[i].KetteGruppeVor == 3)
						{
							// Verbunden mit Gruppe 3
							Kontur3[i - 1].KetteGruppeNach = 2;
						}
						else
						{
							printf("Fehler in NoSeamFind: Gruppen-Kette Kontur2 abgebrochen bei Zeile %d (2)\n", i);
						}
					}
					// Ist von zu kurzem Ketten-Teil!  =>  dieses Min loeschen!
					else
					{
						Kontur2[i].Del = true;
					}
				}

				// Min Gruppe 3
				if (!Kontur3[i].Del)  // Wenn nicht bereits draussen
				{
					// Wenn schon als 'zu einer Kette gehoerend' markiert
					// =>  sein lassen
					if ((Kontur3[i].KetteGruppeNach)
						// Neue Kette: lange genug?
						|| (Kontur3[i].KetteNr >= MIN_FOR_MINIMA_KETTE)
						)
					{
						// Ist's nicht das erste in der Kette  =>  den Vorgaenger
						// als 'zu einer Kette gehoerend' markieren!
						if (Kontur3[i].KetteNr == 1)
						{
							// Ist das erste Ketten-Element  =>  Do nothing!
						}
						else if (i == 0)
						{
							printf("Fehler in NoSeamFind: Erste Zeile Kontur3 zeigt ueber ROI (KetteNr = %d)\n", Kontur3[i].KetteNr);
						}
						else if (Kontur3[i].KetteGruppeVor == 1)
						{
							// Verbunden mit Gruppe 1
							Kontur1[i - 1].KetteGruppeNach = 3;
						}
						else if (Kontur3[i].KetteGruppeVor == 2)
						{
							// Verbunden mit Gruppe 2
							Kontur2[i - 1].KetteGruppeNach = 3;
						}
						else if (Kontur3[i].KetteGruppeVor == 3)
						{
							// Verbunden mit Gruppe 3
							Kontur3[i - 1].KetteGruppeNach = 3;
						}
						else
						{
							printf("Fehler in NoSeamFind: Gruppen-Kette abgebrochen bei Zeile %d (3)\n", i);
						}
					}
					// Ist von zu kurzem Ketten-Teil!  =>  dieses Min loeschen!
					else
					{
						Kontur3[i].Del = true;
					}
				}

#if DEBUGV_KILL_SINGLE || DEBUGV_GLOBAL
				printf("N%2d: ", i);
				if (!Kontur1[i].Del)
					printf("no1 %2d v1 %3d n1 %3d ", Kontur1[i].KetteNr, Kontur1[i].KetteGruppeVor, Kontur1[i].KetteGruppeNach);
				else
					printf("no1 -- v1 --- n1 --- ");
				if (!Kontur2[i].Del)
					printf("no2 %2d v2 %3d n2 %3d ", Kontur2[i].KetteNr, Kontur2[i].KetteGruppeVor, Kontur2[i].KetteGruppeNach);
				else
					printf("no2 -- v1 --- n1 --- ");
				if (!Kontur3[i].Del)
					printf("no3 %2d v3 %3d n3 %3d ", Kontur3[i].KetteNr, Kontur3[i].KetteGruppeVor, Kontur3[i].KetteGruppeNach);
				else
					printf("no3 -- v1 --- n1 --- ");
				printf("\n");
#endif

			}  // for (int i = iKonturAnzahl - 1; i >= 0; i--)
		}


		/**********************************************************************
		* Description:  Verschiebt die Minimas einer Zeile nach links.       *
		*                                                                    *
		* Parameter:                                                         *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::PackMultiMinima(void)
		{
			// Erste Zeile separat, da 'oberhalb' nix ist
			// ------------------------------------------

			// Existiert 3. Min. ?
			if (!Kontur3[0].Del)
			{
				// Ja  =>  ist Platz von Min 2 frei?
				if (Kontur2[0].Del)
				{
					// Ja, Platz von Min 2 ist frei

					// Ist auch Platz von Min 1 frei?
					if (Kontur1[0].Del)
					{
						// Ja, auch Platz von Min 1 ist frei
						// =>  Min 3 nach Min 1

						// In Folgezeile den 'Platzwechsel' setzen
						if (Kontur3[0].KetteGruppeNach == 1)
							Kontur1[1].KetteGruppeVor = 1;
						else if (Kontur3[0].KetteGruppeNach == 2)
							Kontur2[1].KetteGruppeVor = 1;
						else if (Kontur3[0].KetteGruppeNach == 3)
							Kontur3[1].KetteGruppeVor = 1;
						// else   Kontur3[0].KetteGruppeNach == 0  = letztes Ketten-Element!

						// Min verschieben
						MoveMin3NachMin1(0);
						Kontur3[0].Del = true;
					}
					else
					{
						// Platz Min 1 ist besetzt
						// => Min 3 nach Min 2

						// In Folgezeile den 'Platzwechsel' setzen
						if (Kontur3[0].KetteGruppeNach == 1)
							Kontur1[1].KetteGruppeVor = 2;
						else if (Kontur3[0].KetteGruppeNach == 2)
							Kontur2[1].KetteGruppeVor = 2;
						else if (Kontur3[0].KetteGruppeNach == 3)
							Kontur3[1].KetteGruppeVor = 2;
						// else   Kontur3[0].KetteGruppeNach == 0  = letztes Ketten-Element!

						// Min verschieben
						MoveMin3NachMin2(0);
						Kontur3[0].Del = true;
					}
				}
				else
				{
					// Platz Min 2 ist besetzt

					// Ist Platz von Min 1 frei?
					if (Kontur1[0].Del)
					{
						// Ja, Platz von Min 1 ist frei
						// =>  Min 2 nach Min 1

						// In Folgezeile den 'Platzwechsel' setzen
						if (Kontur2[0].KetteGruppeNach == 1)
							Kontur1[1].KetteGruppeVor = 1;
						else if (Kontur2[0].KetteGruppeNach == 2)
							Kontur2[1].KetteGruppeVor = 1;
						else if (Kontur2[0].KetteGruppeNach == 3)
							Kontur3[1].KetteGruppeVor = 1;
						// else   Kontur2[0].KetteGruppeNach == 0  = letztes Ketten-Element!

						// Min verschieben
						MoveMin2NachMin1(0);

						// => Min 3 nach Min 2

						// In Folgezeile den 'Platzwechsel' setzen
						if (Kontur3[0].KetteGruppeNach == 1)
							Kontur1[1].KetteGruppeVor = 2;
						else if (Kontur3[0].KetteGruppeNach == 2)
							Kontur2[1].KetteGruppeVor = 2;
						else if (Kontur3[0].KetteGruppeNach == 3)
							Kontur3[1].KetteGruppeVor = 2;
						// else   Kontur3[0].KetteGruppeNach == 0  = letztes Ketten-Element!

						// Min verschieben
						MoveMin3NachMin2(0);
						Kontur3[0].Del = true;
					}
					// else   Auch Min 1 ist besetzt  =>  Do nothing!
				}
			}  // if ( !Kontur3[0].Del )

			// Existiert 2. Min. ?
			else if (!Kontur2[0].Del)
			{
				// Ja  =>  ist Platz von Min 1 frei?
				if (Kontur1[0].Del)
				{
					// Ja, Platz von Min 1 ist frei
					// =>  Min 2 nach Min 1

					// In Folgezeile den 'Platzwechsel' setzen
					if (Kontur2[0].KetteGruppeNach == 1)
						Kontur1[1].KetteGruppeVor = 1;
					else if (Kontur2[0].KetteGruppeNach == 2)
						Kontur2[1].KetteGruppeVor = 1;
					else if (Kontur2[0].KetteGruppeNach == 3)
						Kontur3[1].KetteGruppeVor = 1;
					// else   Kontur2[0].KetteGruppeNach == 0  = letztes Ketten-Element!

					// Min verschieben
					MoveMin2NachMin1(0);
					Kontur2[0].Del = true;
				}
				// else   Min 1 ist besetzt  =>  Do nothing!
			}


			// Jetzt die mittleren Zeilen
			// --------------------------

			for (int i = 1; i < (iKonturAnzahl - 1); i++)
			{
				// Existiert 3. Min. ?
				if (!Kontur3[i].Del)
				{
					// Ja  =>  ist Platz von Min 2 frei?
					if (Kontur2[i].Del)
					{
						// Ja, Platz von Min 2 ist frei

						// Ist auch Platz von Min 1 frei?
						if (Kontur1[i].Del)
						{
							// Ja, auch Platz von Min 1 ist frei
							// =>  Min 3 nach Min 1

							if (Kontur3[i].KetteNr > 1)
							{
								// Wenn es nicht das erste Ketten-Element ist
								// => In Vorzeile den 'Platzwechsel' setzen
								if (Kontur3[i].KetteGruppeVor == 1)
									Kontur1[i - 1].KetteGruppeNach = 1;
								else if (Kontur3[i].KetteGruppeVor == 2)
									Kontur2[i - 1].KetteGruppeNach = 1;
								else if (Kontur3[i].KetteGruppeVor == 3)
									Kontur3[i - 1].KetteGruppeNach = 1;
								else
								{
									printf("Fehler in NoSeamFind: Kontur3-KetteVor ist %d in Zeile %d (1)\n", Kontur3[i].KetteGruppeVor, i);
								}
							}

							// In Folgezeile den 'Platzwechsel' setzen
							if (Kontur3[i].KetteGruppeNach == 1)
								Kontur1[i + 1].KetteGruppeVor = 1;
							else if (Kontur3[i].KetteGruppeNach == 2)
								Kontur2[i + 1].KetteGruppeVor = 1;
							else if (Kontur3[i].KetteGruppeNach == 3)
								Kontur3[i + 1].KetteGruppeVor = 1;
							// else   Kontur3[i].KetteGruppeNach == 0  = letztes Ketten-Element!

							// Min verschieben
							MoveMin3NachMin1(i);
							Kontur3[i].Del = true;
						}
						else
						{
							// Platz Min 1 ist besetzt
							// => Min 3 nach Min 2

							if (Kontur3[i].KetteNr > 1)
							{
								// Wenn es nicht das erste Ketten-Element ist
								// => In Vorzeile den 'Platzwechsel' setzen
								if (Kontur3[i].KetteGruppeVor == 1)
									Kontur1[i - 1].KetteGruppeNach = 2;
								else if (Kontur3[i].KetteGruppeVor == 2)
									Kontur2[i - 1].KetteGruppeNach = 2;
								else if (Kontur3[i].KetteGruppeVor == 3)
									Kontur3[i - 1].KetteGruppeNach = 2;
								else
								{
									printf("Fehler in NoSeamFind: Kontur3-KetteVor ist %d in Zeile %d (2)\n", Kontur3[i].KetteGruppeVor, i);
								}
							}

							// In Folgezeile den 'Platzwechsel' setzen
							if (Kontur3[i].KetteGruppeNach == 1)
								Kontur1[i + 1].KetteGruppeVor = 2;
							else if (Kontur3[i].KetteGruppeNach == 2)
								Kontur2[i + 1].KetteGruppeVor = 2;
							else if (Kontur3[i].KetteGruppeNach == 3)
								Kontur3[i + 1].KetteGruppeVor = 2;
							// else   Kontur3[i].KetteGruppeNach == 0  = letztes Ketten-Element!

							// Min verschieben
							MoveMin3NachMin2(i);
							Kontur3[i].Del = true;
						}
					}
					else
					{
						// Platz Min 2 ist besetzt

						// Ist Platz von Min 1 frei?
						if (Kontur1[i].Del)
						{
							// Ja, Platz von Min 1 ist frei
							// =>  Min 2 nach Min 1

							if (Kontur2[i].KetteNr > 1)
							{
								// Wenn es nicht das erste Ketten-Element ist
								// => In Vorzeile den 'Platzwechsel' setzen
								if (Kontur2[i].KetteGruppeVor == 1)
									Kontur1[i - 1].KetteGruppeNach = 1;
								else if (Kontur2[i].KetteGruppeVor == 2)
									Kontur2[i - 1].KetteGruppeNach = 1;
								else if (Kontur2[i].KetteGruppeVor == 3)
									Kontur3[i - 1].KetteGruppeNach = 1;
								else
								{
									printf("Fehler in NoSeamFind: Kontur2-KetteVor ist %d in Zeile %d (3)\n", Kontur2[i].KetteGruppeVor, i);
								}
							}

							// In Folgezeile den 'Platzwechsel' setzen
							if (Kontur2[i].KetteGruppeNach == 1)
								Kontur1[i + 1].KetteGruppeVor = 1;
							else if (Kontur2[i].KetteGruppeNach == 2)
								Kontur2[i + 1].KetteGruppeVor = 1;
							else if (Kontur2[i].KetteGruppeNach == 3)
								Kontur3[i + 1].KetteGruppeVor = 1;
							// else   Kontur2[i].KetteGruppeNach == 0  = letztes Ketten-Element!

							// Min verschieben
							MoveMin2NachMin1(i);

							// => Min 3 nach Min 2

							if (Kontur3[i].KetteNr > 1)
							{
								// Wenn es nicht das erste Ketten-Element ist
								// => In Vorzeile den 'Platzwechsel' setzen
								if (Kontur3[i].KetteGruppeVor == 1)
									Kontur1[i - 1].KetteGruppeNach = 2;
								else if (Kontur3[i].KetteGruppeVor == 2)
									Kontur2[i - 1].KetteGruppeNach = 2;
								else if (Kontur3[i].KetteGruppeVor == 3)
									Kontur3[i - 1].KetteGruppeNach = 2;
								else
								{
									printf("Fehler in NoSeamFind: Kontur3-KetteVor ist %d in Zeile %d (4)\n", Kontur3[i].KetteGruppeVor, i);
								}
							}

							// In Folgezeile den 'Platzwechsel' setzen
							if (Kontur3[i].KetteGruppeNach == 1)
								Kontur1[i + 1].KetteGruppeVor = 2;
							else if (Kontur3[i].KetteGruppeNach == 2)
								Kontur2[i + 1].KetteGruppeVor = 2;
							else if (Kontur3[i].KetteGruppeNach == 3)
								Kontur3[i + 1].KetteGruppeVor = 2;
							// else   Kontur3[i].KetteGruppeNach == 0  = letztes Ketten-Element!

							// Min verschieben
							MoveMin3NachMin2(i);
							Kontur3[i].Del = true;
						}
						// else   Auch Min 1 ist besetzt  =>  Do nothing!
					}
				}  // if ( !Kontur3[i].Del )

				// Existiert 2. Min. ?
				else if (!Kontur2[i].Del)
				{
					// Ja  =>  ist Platz von Min 1 frei?
					if (Kontur1[i].Del)
					{
						// Ja, Platz von Min 1 ist frei
						// =>  Min 2 nach Min 1

						if (Kontur2[i].KetteNr > 1)
						{
							// Wenn es nicht das erste Ketten-Element ist
							// => In Vorzeile den 'Platzwechsel' setzen
							if (Kontur2[i].KetteGruppeVor == 1)
								Kontur1[i - 1].KetteGruppeNach = 1;
							else if (Kontur2[i].KetteGruppeVor == 2)
								Kontur2[i - 1].KetteGruppeNach = 1;
							else if (Kontur2[i].KetteGruppeVor == 3)
								Kontur3[i - 1].KetteGruppeNach = 1;
							else
							{
								printf("Fehler in NoSeamFind: Kontur2-KetteVor ist %d in Zeile %d (5)\n", Kontur2[i].KetteGruppeVor, i);
							}
						}

						// In Folgezeile den 'Platzwechsel' setzen
						if (Kontur2[i].KetteGruppeNach == 1)
							Kontur1[i + 1].KetteGruppeVor = 1;
						else if (Kontur2[i].KetteGruppeNach == 2)
							Kontur2[i + 1].KetteGruppeVor = 1;
						else if (Kontur2[i].KetteGruppeNach == 3)
							Kontur3[i + 1].KetteGruppeVor = 1;
						// else   Kontur2[i].KetteGruppeNach == 0  = letztes Ketten-Element!

						// Min verschieben
						MoveMin2NachMin1(i);
						Kontur2[i].Del = true;
					}
					// else   Min 1 ist besetzt  =>  Do nothing!
				}
			}  // for (int i = 1; i < iKonturAnzahl - 1; i++)


			// Jetzt noch die letzte Zeile separat, da 'unterhalb' nix ist
			// -----------------------------------------------------------

			int iEnde = 0;

			if (iKonturAnzahl)
			{
				iEnde = iKonturAnzahl - 1;
			}

			// Existiert 3. Min. ?
			if (!Kontur3[iEnde].Del)
			{
				// Ja  =>  ist Platz von Min 2 frei?
				if (Kontur2[iEnde].Del)
				{
					// Ja, Platz von Min 2 ist frei

					// Ist auch Platz von Min 1 frei?
					if (Kontur1[iEnde].Del)
					{
						// Ja, auch Platz von Min 1 ist frei
						// =>  Min 3 nach Min 1

						// Kann nicht das erste Ketten-Element sein
						// => In Vorzeile den 'Platzwechsel' setzen
						if (Kontur3[iEnde].KetteGruppeVor == 1)
							Kontur1[iEnde - 1].KetteGruppeNach = 1;
						else if (Kontur3[iEnde].KetteGruppeVor == 2)
							Kontur2[iEnde - 1].KetteGruppeNach = 1;
						else if (Kontur3[iEnde].KetteGruppeVor == 3)
							Kontur3[iEnde - 1].KetteGruppeNach = 1;
						else
						{
							printf("Fehler in NoSeamFind: Kontur3-KetteVor ist %d in Zeile %d (6)\n", Kontur3[iEnde].KetteGruppeVor, iEnde);
						}

						// Min verschieben
						MoveMin3NachMin1(iEnde);
						Kontur3[iEnde].Del = true;
					}
					else
					{
						// Platz Min 1 ist besetzt
						// => Min 3 nach Min 2

						// Kann nicht das erste Ketten-Element sein
						// => In Vorzeile den 'Platzwechsel' setzen
						if (Kontur3[iEnde].KetteGruppeVor == 1)
							Kontur1[iEnde - 1].KetteGruppeNach = 2;
						else if (Kontur3[iEnde].KetteGruppeVor == 2)
							Kontur2[iEnde - 1].KetteGruppeNach = 2;
						else if (Kontur3[iEnde].KetteGruppeVor == 3)
							Kontur3[iEnde - 1].KetteGruppeNach = 2;
						else
						{
							printf("Fehler in NoSeamFind: Kontur3-KetteVor ist %d in Zeile %d (7)\n", Kontur3[iEnde].KetteGruppeVor, iEnde);
						}

						// Min verschieben
						MoveMin3NachMin2(iEnde);
						Kontur3[iEnde].Del = true;
					}
				}
				else
				{
					// Platz Min 2 ist besetzt

					// Ist Platz von Min 1 frei?
					if (Kontur1[iEnde].Del)
					{
						// Ja, Platz von Min 1 ist frei
						// =>  Min 2 nach Min 1

						// Kann nicht das erste Ketten-Element sein
						// => In Vorzeile den 'Platzwechsel' setzen
						if (Kontur2[iEnde].KetteGruppeVor == 1)
							Kontur1[iEnde - 1].KetteGruppeNach = 1;
						else if (Kontur2[iEnde].KetteGruppeVor == 2)
							Kontur2[iEnde - 1].KetteGruppeNach = 1;
						else if (Kontur2[iEnde].KetteGruppeVor == 3)
							Kontur3[iEnde - 1].KetteGruppeNach = 1;
						else
						{
							printf("Fehler in NoSeamFind: Kontur2-KetteVor ist %d in Zeile %d (8)\n", Kontur2[iEnde].KetteGruppeVor, iEnde);
						}

						// Min verschieben
						MoveMin2NachMin1(iEnde);

						// => Min 3 nach Min 2

						// Kann nicht das erste Ketten-Element sein
						// => In Vorzeile den 'Platzwechsel' setzen
						if (Kontur3[iEnde].KetteGruppeVor == 1)
							Kontur1[iEnde - 1].KetteGruppeNach = 2;
						else if (Kontur3[iEnde].KetteGruppeVor == 2)
							Kontur2[iEnde - 1].KetteGruppeNach = 2;
						else if (Kontur3[iEnde].KetteGruppeVor == 3)
							Kontur3[iEnde - 1].KetteGruppeNach = 2;
						else
						{
							printf("Fehler in NoSeamFind: Kontur3-KetteVor ist %d in Zeile %d (9)\n", Kontur3[iEnde].KetteGruppeVor, iEnde);
						}

						// Min verschieben
						MoveMin3NachMin2(iEnde);
						Kontur3[iEnde].Del = true;
					}
					// else   Auch Min 1 ist besetzt  =>  Do nothing!
				}
			}  // if ( !Kontur3[iEnde].Del )

			// Existiert 2. Min. ?
			else if (!Kontur2[iEnde].Del)
			{
				// Ja  =>  ist Platz von Min 1 frei?
				if (Kontur1[iEnde].Del)
				{
					// Ja, Platz von Min 1 ist frei
					// =>  Min 2 nach Min 1

					// Kann nicht das erste Ketten-Element sein
					// => In Vorzeile den 'Platzwechsel' setzen
					if (Kontur2[iEnde].KetteGruppeVor == 1)
						Kontur1[iEnde - 1].KetteGruppeNach = 1;
					else if (Kontur2[iEnde].KetteGruppeVor == 2)
						Kontur2[iEnde - 1].KetteGruppeNach = 1;
					else if (Kontur2[iEnde].KetteGruppeVor == 3)
						Kontur3[iEnde - 1].KetteGruppeNach = 1;
					else
					{
						printf("Fehler in NoSeamFind: Kontur2-KetteVor ist %d in Zeile %d (10)\n", Kontur2[iEnde].KetteGruppeVor, iEnde);
					}

					// Min verschieben
					MoveMin2NachMin1(iEnde);
					Kontur2[iEnde].Del = true;
				}
				// else   Min 1 ist besetzt  =>  Do nothing!
			}
		}


		/**********************************************************************
		* Description:  Loescht Zeilen mit mehreren Minimas.                 *
		*               Wenn                                                 *
		*               -  Min 2 oder                                        *
		*               -  Min 2 und Min 3                                   *
		*               existieren: Min's dieser Zeile loeschen, Verknuep-   *
		*               fungen nach oben/unten loeschen (wenn zu Min 1!).    *
		//// *               Nochmals von oben durch: wenn Kettenstueck zu kurz   *
		//// *               oder 'falscher Anfang'  =>  Kettenstueck weg         *
		*                                                                    *
		* Parameter:    maxDiff:      max. Pixeldiff. fuer Check bei Projekt *
		*                             Yudigar                                *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::EliminateMultiMinima(int iMaxDiff)
		{
			//	const int MIN_FOR_MINIMA_KETTE = 3;
			//	int       iTmpLength = 0;


			// 1. Durchgang: mehrere Min's in Zeile  =>  loeschen
			// --------------------------------------------------

			for (int i = 0; i < iKonturAnzahl; i++)
			{
				// Min 3 (und Min 2) vorhanden?
				if (!Kontur3[i].Del)
				{
					// Min 3 und Verknuepfungen loeschen

					// Verknuepfung nach oben zu Min 1 ?
					if (Kontur3[i].KetteGruppeVor == 1)
					{
						if (i == 0)
						{
							printf("Fehler FDS, ElimMultiMin: Kontur3-KetteVor ist 1 in Zeile 0 (20)\n");
						}
						else
						{
							// In Vorzeile Eintrag loeschen
							Kontur1[i - 1].KetteGruppeNach = 0;
						}
					}

					// Verknuepfung nach unten zu Min 1 ?
					if (Kontur3[i].KetteGruppeNach == 1)
					{
						if (i == (iKonturAnzahl - 1))
						{
							printf("Fehler FDS, ElimMultiMin: Kontur3-KetteNach ist 1 in Zeile %d (21)\n", i);
						}
						else
						{
							// In Folgezeile Eintrag loeschen
							Kontur1[i + 1].KetteGruppeVor = 0;
						}
					}

					// Min 3 loeschen
					Kontur3[i].Del = true;

					// Min 2 und Verknuepfungen loeschen

					// Verknuepfung nach oben zu Min 1 ?
					if (Kontur2[i].KetteGruppeVor == 1)
					{
						if (i == 0)
						{
							printf("Fehler FDS, ElimMultiMin: Kontur2-KetteVor ist 1 in Zeile 0 (22)\n");
						}
						else
						{
							// In Vorzeile Eintrag loeschen
							Kontur1[i - 1].KetteGruppeNach = 0;
						}
					}

					// Verknuepfung nach unten zu Min 1 ?
					if (Kontur2[i].KetteGruppeNach == 1)
					{
						if (i == (iKonturAnzahl - 1))
						{
							printf("Fehler FDS, ElimMultiMin: Kontur2-KetteNach ist 1 in Zeile %d (23)\n", i);
						}
						else
						{
							// In Folgezeile Eintrag loeschen
							Kontur1[i + 1].KetteGruppeVor = 0;
						}
					}

					// Min 2 loeschen
					Kontur2[i].Del = true;

					// Min 1 und Verknuepfungen loeschen

					// Verknuepfung nach oben zu Min 1 ?
					if (Kontur1[i].KetteGruppeVor == 1)
					{
						if (i == 0)
						{
							printf("Fehler FDS, ElimMultiMin: Kontur1-KetteVor ist 1 in Zeile 0 (24)\n");
						}
						else
						{
							// In Vorzeile Eintrag loeschen
							Kontur1[i - 1].KetteGruppeNach = 0;
						}
					}

					// Verknuepfung nach unten zu Min 1 ?
					if (Kontur1[i].KetteGruppeNach == 1)
					{
						if (i == (iKonturAnzahl - 1))
						{
							printf("Fehler FDS, ElimMultiMin: Kontur1-KetteNach ist 1 in Zeile %d (25)\n", i);
						}
						else
						{
							// In Folgezeile Eintrag loeschen
							Kontur1[i + 1].KetteGruppeVor = 0;
						}
					}

					// Min 1 loeschen
					Kontur1[i].Del = true;
				}  // Min 3 vorhanden

				// Min 2 vorhanden?
				else if (!Kontur2[i].Del)
				{
					// Min 2 und Verknuepfungen loeschen

					// Verknuepfung nach oben zu Min 1 ?
					if (Kontur2[i].KetteGruppeVor == 1)
					{
						if (i == 0)
						{
							printf("Fehler FDS, ElimMultiMin: Kontur2-KetteVor ist 1 in Zeile 0 (26)\n");
						}
						else
						{
							// In Vorzeile Eintrag loeschen
							Kontur1[i - 1].KetteGruppeNach = 0;
						}
					}

					// Verknuepfung nach unten zu Min 1 ?
					if (Kontur2[i].KetteGruppeNach == 1)
					{
						if (i == (iKonturAnzahl - 1))
						{
							printf("Fehler FDS, ElimMultiMin: Kontur2-KetteNach ist 1 in Zeile %d (27)\n", i);
						}
						else
						{
							// In Folgezeile Eintrag loeschen
							Kontur1[i + 1].KetteGruppeVor = 0;
						}
					}

					// Min 2 loeschen
					Kontur2[i].Del = true;

					// Min 1 und Verknuepfungen loeschen

					// Verknuepfung nach oben zu Min 1 ?
					if (Kontur1[i].KetteGruppeVor == 1)
					{
						if (i == 0)
						{
							printf("Fehler FDS, ElimMultiMin: Kontur1-KetteVor ist 1 in Zeile 0 (28)\n");
						}
						else
						{
							// In Vorzeile Eintrag loeschen
							Kontur1[i - 1].KetteGruppeNach = 0;
						}
					}

					// Verknuepfung nach unten zu Min 1 ?
					if (Kontur1[i].KetteGruppeNach == 1)
					{
						// In Folgezeile Eintrag loeschen
						Kontur1[i + 1].KetteGruppeVor = 0;
					}

					// Min 1 loeschen
					Kontur1[i].Del = true;
				}  // Min 2 vorhanden

				//else  =>  nur Min 1 oder gar kein Min da
			}



		}


		/**********************************************************************
		* Description:  Min 3 an Platz von Min 1 kopieren.                   *
		*                                                                    *
		* Parameter:    iKonturElement:    Element der Kontur-Liste          *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::MoveMin3NachMin1(int iKonturElement)
		{
			Kontur1[iKonturElement].XL = Kontur3[iKonturElement].XL;
			Kontur1[iKonturElement].XR = Kontur3[iKonturElement].XR;
			Kontur1[iKonturElement].xPos = Kontur3[iKonturElement].xPos;
			Kontur1[iKonturElement].zeile = Kontur3[iKonturElement].zeile;
			Kontur1[iKonturElement].GW = Kontur3[iKonturElement].GW;
			Kontur1[iKonturElement].DispCol = Kontur3[iKonturElement].DispCol;
			Kontur1[iKonturElement].KetteGruppeVor = Kontur3[iKonturElement].KetteGruppeVor;
			Kontur1[iKonturElement].KetteNr = Kontur3[iKonturElement].KetteNr;
			Kontur1[iKonturElement].KetteGruppeNach = Kontur3[iKonturElement].KetteGruppeNach;
			Kontur1[iKonturElement].Del = Kontur3[iKonturElement].Del;
		}


		/**********************************************************************
		* Description:  Min 3 an Platz von Min 2 kopieren.                   *
		*                                                                    *
		* Parameter:    iKonturElement:    Element der Kontur-Liste          *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::MoveMin3NachMin2(int iKonturElement)
		{
			Kontur2[iKonturElement].XL = Kontur3[iKonturElement].XL;
			Kontur2[iKonturElement].XR = Kontur3[iKonturElement].XR;
			Kontur2[iKonturElement].xPos = Kontur3[iKonturElement].xPos;
			Kontur2[iKonturElement].zeile = Kontur3[iKonturElement].zeile;
			Kontur2[iKonturElement].GW = Kontur3[iKonturElement].GW;
			Kontur2[iKonturElement].DispCol = Kontur3[iKonturElement].DispCol;
			Kontur2[iKonturElement].KetteGruppeVor = Kontur3[iKonturElement].KetteGruppeVor;
			Kontur2[iKonturElement].KetteNr = Kontur3[iKonturElement].KetteNr;
			Kontur2[iKonturElement].KetteGruppeNach = Kontur3[iKonturElement].KetteGruppeNach;
			Kontur2[iKonturElement].Del = Kontur3[iKonturElement].Del;
		}


		/**********************************************************************
		* Description:  Min 2 an Platz von Min 1 kopieren.                   *
		*                                                                    *
		* Parameter:    iKonturElement:    Element der Kontur-Liste          *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::MoveMin2NachMin1(int iKonturElement)
		{
			Kontur1[iKonturElement].XL = Kontur2[iKonturElement].XL;
			Kontur1[iKonturElement].XR = Kontur2[iKonturElement].XR;
			Kontur1[iKonturElement].xPos = Kontur2[iKonturElement].xPos;
			Kontur1[iKonturElement].zeile = Kontur2[iKonturElement].zeile;
			Kontur1[iKonturElement].GW = Kontur2[iKonturElement].GW;
			Kontur1[iKonturElement].DispCol = Kontur2[iKonturElement].DispCol;
			Kontur1[iKonturElement].KetteGruppeVor = Kontur2[iKonturElement].KetteGruppeVor;
			Kontur1[iKonturElement].KetteNr = Kontur2[iKonturElement].KetteNr;
			Kontur1[iKonturElement].KetteGruppeNach = Kontur2[iKonturElement].KetteGruppeNach;
			Kontur1[iKonturElement].Del = Kontur2[iKonturElement].Del;
		}

		bool NoSeamFind::noBadMinimum(const SNoSeamKontur& contour1, const SNoSeamKontur& contour2, const int maxDiff)
        {
            const bool bothEdgesOk = (std::abs(contour1.XL - contour2.XL) < maxDiff) && (std::abs(contour1.XR - contour2.XR) < maxDiff);
            const bool leftEdgeAndMinOk = (std::abs(contour1.XL - contour2.XL) < maxDiff) && (std::abs(contour1.xPos - contour2.xPos) < maxDiff);
            const bool rightEdgeAndMinOk = (std::abs(contour1.XR - contour2.XR) < maxDiff) && (std::abs(contour1.xPos - contour2.xPos) < maxDiff);
            return bothEdgesOk || leftEdgeAndMinOk || rightEdgeAndMinOk;
        }

		void NoSeamFind::SearchForChainsCont(int iMaxDiff)
		{
			// Distanz von Min(Kante) zu next Min(Kante)
			const int  CHECK_DIST_MIN = 5;  // 7;
			int        aktstart = 0;
			int        aktlaenge = 0;
			int        iKettenStatus = 0;

			for (int zaehler = 0; zaehler < iKonturAnzahl; zaehler++)
			{
                switch (iKettenStatus)
                {
                    // Noch kein KP (Kettenpunkt) gefunden, jetzt der 1-te?
                    case 0:
                    {
                        if (!Kontur1[zaehler].Del)
                        {
                            aktstart = zaehler;
                            aktlaenge = 1;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // 1 KP gefunden, jetzt der 2-te?
                    case 1:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, fertig  =>  1. KP loeschen
                            Kontur1[zaehler - 1].Del = true;
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 2;
                        }
                        else
                        {
                            // Min's passen nicht  =>  1. KP loeschen
                            Kontur1[zaehler - 1].Del = true;
                            aktstart++;
                        }
                        break;
                    }
                    // 2 KP gefunden, jetzt der 3-te?
                    case 2:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  jetzt min. 3 folgende KP, damit
                            // noch Kette
                            aktlaenge++;
                            iKettenStatus = 4;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        // Kanten um Min 1 gut passend?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 3;
                        }
                        else
                        {
                            // Min's passen nicht
                            aktlaenge++;
                            iKettenStatus = 17;
                        }
                        break;
                    }
                    // 3 KP gefunden, jetzt der 4-te?
                    case 3:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  jetzt min. 2 folgende KP, damit
                            // noch Kette
                            aktlaenge++;
                            iKettenStatus = 7;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        // Kanten um Min 1 gut passend?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 9;
                        }
                        else
                        {
                            // Min's passen nicht
                            aktlaenge++;
                            iKettenStatus = 18;
                        }
                        break;
                    }
                    // 2 KP + Luecke, jetzt der 3-te?
                    case 4:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  die 2 KP weg
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        // Kanten um Min 1 gut passend?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-2], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 5;
                        }
                        else
                        {
                            // Min's passen nicht  =>  die 2 KP weg
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // 2 KP + Luecke + KP, jetzt der 4-te?
                    case 5:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  die 3 KP weg
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 6;
                        }
                        else
                        {
                            // Min's passen nicht  =>  die 3 Konturpunkte weg
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // 2 KP + Luecke + 2 KP
                    case 6:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  die ersten 2 KP weg
                            Kontur1[zaehler - 5].Del = true;
                            Kontur1[zaehler - 4].Del = true;
                            // Zweite 2 KP behalten
                            aktlaenge = 3;
                            aktstart = zaehler - 2;
                            iKettenStatus = 4;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            // Ist Kette  =>  Kanten Luecke fuellen!
                            Kontur1[zaehler - 3].XL = (Kontur1[zaehler - 4].XL + Kontur1[zaehler - 2].XL) / 2;
                            Kontur1[zaehler - 3].XR = (Kontur1[zaehler - 4].XR + Kontur1[zaehler - 2].XR) / 2;

                            aktlaenge++;
                            iKettenStatus = 9;
                        }
                        else
                        {
                            // Min's passen nicht  =>  die ersten 2 KP weg
                            Kontur1[zaehler - 5].Del = true;
                            Kontur1[zaehler - 4].Del = true;
                            // Zweite 2 KP behalten
                            aktlaenge = 3;
                            aktstart = zaehler - 2;
                            iKettenStatus = 17;
                        }
                        break;
                    }
                    // 3 KP + Luecke
                    case 7:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  die ersten 3 KP weg
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-2], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 8;
                        }
                        else
                        {
                            // Min's passen nicht  =>  die 3 Konturpunkte weg
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // 3 KP + Luecke + KP
                    case 8:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  alle 4 KP weg
                            Kontur1[zaehler - 5].Del = true;
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        // Kanten um Min 1 gut passend?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 16;
                        }
                        else
                        {
                            // Min's passen nicht  =>  die 4 Konturpunkte weg
                            Kontur1[zaehler - 5].Del = true;
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // Ist Kette
                    case 9:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke
                            aktlaenge++;
                            iKettenStatus = 10;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                        }
                        else
                        {
                            // Min's passen nicht
                            aktlaenge++;
                            iKettenStatus = 19;
                        }
                        break;
                    }
                    // Ist Kette + Luecke
                    case 10:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  Kette sichern
                            // Luecke weg
                            aktlaenge--;
                            AddNeueKette(aktstart, aktlaenge);
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-2], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 11;
                        }
                        else
                        {
                            // Min's passen nicht
                            // Luecke weg
                            aktlaenge--;
                            AddNeueKette(aktstart, aktlaenge);
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // Ist Kette + Luecke + KP
                    case 11:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  Kette sichern
                            // Luecke + KP weg
                            aktlaenge -= 2;
                            AddNeueKette(aktstart, aktlaenge);
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 12;
                        }
                        else
                        {
                            // Min's passen nicht
                            // KP 1 weg
                            Kontur1[zaehler - 1].Del = true;
                            // Luecke + KP weg
                            aktlaenge -= 2;
                            AddNeueKette(aktstart, aktlaenge);
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // Ist Kette + Luecke + 2 KP
                    case 12:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke
                            aktlaenge++;
                            iKettenStatus = 13;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            // Ist Kette  =>  Kanten Luecke fuellen!
                            Kontur1[zaehler - 3].XL = (Kontur1[zaehler - 4].XL + Kontur1[zaehler - 2].XL) / 2;
                            Kontur1[zaehler - 3].XR = (Kontur1[zaehler - 4].XR + Kontur1[zaehler - 2].XR) / 2;

                            aktlaenge++;
                            iKettenStatus = 9;
                        }
                        else
                        {
                            // Min's passen nicht  =>  Kette
                            AddNeueKette(aktstart, aktlaenge);
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // Ist Kette + Luecke + 2 KP + Luecke
                    case 13:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  Kette sichern
                            // Luecke weg
                            aktlaenge--;
                            AddNeueKette(aktstart, aktlaenge);
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-2], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 14;
                        }
                        else
                        {
                            // Min's passen nicht
                            // Luecke weg
                            aktlaenge--;
                            AddNeueKette(aktstart, aktlaenge);
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // Ist Kette + Luecke + 2 KP + Luecke + 1 KP
                    case 14:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  Kette sichern
                            // KP 1 weg
                            Kontur1[zaehler - 1].Del = true;
                            // Luecke + KP weg
                            aktlaenge -= 2;
                            AddNeueKette(aktstart, aktlaenge);
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 15;
                        }
                        else
                        {
                            // Min's passen nicht
                            // KP 1 weg
                            Kontur1[zaehler - 1].Del = true;
                            // Luecke + KP weg
                            aktlaenge -= 2;
                            AddNeueKette(aktstart, aktlaenge);
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // Ist Kette + Luecke + 2 KP + Luecke + 2 KP
                    case 15:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  Kette sichern
                            // Luecke + 2 KP weg
                            aktlaenge -= 3;
                            AddNeueKette(aktstart, aktlaenge);
                            // 2 KP + neue Luecke
                            aktlaenge = 3;
                            aktstart = zaehler - 2;
                            iKettenStatus = 4;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            // Ist Kette  =>  Kanten Luecke fuellen!
                            Kontur1[zaehler - 3].XL = (Kontur1[zaehler - 4].XL + Kontur1[zaehler - 2].XL) / 2;
                            Kontur1[zaehler - 3].XR = (Kontur1[zaehler - 4].XR + Kontur1[zaehler - 2].XR) / 2;
                            Kontur1[zaehler - 6].XL = (Kontur1[zaehler - 7].XL + Kontur1[zaehler - 5].XL) / 2;
                            Kontur1[zaehler - 6].XR = (Kontur1[zaehler - 7].XR + Kontur1[zaehler - 5].XR) / 2;

                            aktlaenge++;
                            iKettenStatus = 9;
                        }
                        else
                        {
                            // Min's passen nicht
                            // Luecke + 2 KP weg
                            aktlaenge -= 3;
                            AddNeueKette(aktstart, aktlaenge);
                            // 2 KP + BadMin
                            aktlaenge = 3;
                            aktstart = zaehler - 2;
                            iKettenStatus = 17;
                        }
                        break;
                    }
                    // 3 KP + Luecke + 2 KP
                    case 16:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke
                            aktlaenge++;
                            iKettenStatus = 13;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            // Ist Kette  =>  Kanten Luecke fuellen!
                            Kontur1[zaehler - 3].XL = (Kontur1[zaehler - 4].XL + Kontur1[zaehler - 2].XL) / 2;
                            Kontur1[zaehler - 3].XR = (Kontur1[zaehler - 4].XR + Kontur1[zaehler - 2].XR) / 2;

                            aktlaenge++;
                            iKettenStatus = 9;
                        }
                        else
                        {
                            // Min's passen nicht  =>  Kette sichern
                            AddNeueKette(aktstart, aktlaenge);
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                        break;
                    }
                    // 2 KP + BadMin
                    case 17:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke
                            // 2 KP + BadMin weg
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-2], CHECK_DIST_MIN))
                        {
                            // BadMin weg
                            Kontur1[zaehler - 1].Del = true;
                            aktlaenge++;
                            iKettenStatus = 5;
                        }
                        else
                        {
                            // Min's passen nicht
                            // 2 KP weg
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            // Passt BadMin?
                            // Passt das Min? Oder passen die beiden Kanten?
                            // Kanten um Min 1 gut passend?
                            if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                            {
                                // BadMin + neues Min als 2 KP
                                aktlaenge = 2;
                                aktstart = zaehler - 1;
                                iKettenStatus = 2;
                            }
                            else
                            {
                                // Nee, passen nicht  => BadMin weg
                                Kontur1[zaehler - 1].Del = true;
                                // das Min als neuer 1-ter KP
                                aktlaenge = 1;
                                aktstart = zaehler;
                                iKettenStatus = 1;
                            }
                        }
                        break;
                    }
                    // 3 KP + BadMin
                    case 18:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke
                            // 3 KP + BadMin weg
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-2], CHECK_DIST_MIN))
                        {
                            // BadMin weg
                            Kontur1[zaehler - 1].Del = true;
                            aktlaenge++;
                            iKettenStatus = 8;
                        }
                        else
                        {
                            // Min's passen nicht
                            // 3 KP weg
                            Kontur1[zaehler - 4].Del = true;
                            Kontur1[zaehler - 3].Del = true;
                            Kontur1[zaehler - 2].Del = true;
                            // Passt BadMin?
                            // Passt das Min? Oder passen die beiden Kanten?
                            if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                            {
                                // BadMin + neues Min als 2 KP
                                aktlaenge = 2;
                                aktstart = zaehler - 1;
                                iKettenStatus = 2;
                            }
                            else
                            {
                                // Nee, passen nicht  => BadMin weg
                                Kontur1[zaehler - 1].Del = true;
                                // das Min als neuer 1-ter KP
                                aktlaenge = 1;
                                aktstart = zaehler;
                                iKettenStatus = 1;
                            }
                        }
                        break;
                    }
                    // Kette + BadMin
                    case 19:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  Kette speichern
                            // BadMin weg
                            Kontur1[zaehler - 1].Del = true;
                            aktlaenge--;
                            AddNeueKette(aktstart, aktlaenge);
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-2], CHECK_DIST_MIN))
                        {
                            aktlaenge++;
                            iKettenStatus = 20;
                        }
                        else
                        {
                            // Min's passen nicht
                            // BadMin weg
                            aktlaenge--;
                            AddNeueKette(aktstart, aktlaenge);
                            // Passt BadMin?
                            // Passt das Min? Oder passen die beiden Kanten?
                            if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                            {
                                // BadMin + neues Min als 2 KP
                                aktlaenge = 2;
                                aktstart = zaehler - 1;
                                iKettenStatus = 2;
                            }
                            else
                            {
                                // Nee, passen nicht  => BadMin weg
                                Kontur1[zaehler - 1].Del = true;
                                // das Min als neuer 1-ter KP
                                aktlaenge = 1;
                                aktstart = zaehler;
                                iKettenStatus = 1;
                            }
                        }
                        break;
                    }
                    // Kette + BadMin + KP
                    case 20:
                    {
                        if (Kontur1[zaehler].Del)
                        {
                            // Nee, eine Luecke  =>  Kette speichern
                            // BadMin + KP weg
                            Kontur1[zaehler - 2].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            aktlaenge -= 2;
                            AddNeueKette(aktstart, aktlaenge);
                            iKettenStatus = 0;
                        }
                        // Passt das Min? Oder passen die beiden Kanten?
                        else if (noBadMinimum(Kontur1[zaehler], Kontur1[zaehler-1], CHECK_DIST_MIN))
                        {
                            // BadMin weg
                            Kontur1[zaehler - 2].Del = true;
                            aktlaenge++;
                            iKettenStatus = 12;
                        }
                        else
                        {
                            // Min's passen nicht
                            // BadMin + KP weg
                            Kontur1[zaehler - 2].Del = true;
                            Kontur1[zaehler - 1].Del = true;
                            aktlaenge -= 2;
                            AddNeueKette(aktstart, aktlaenge);
                            // das Min als neuer 1-ter KP
                            aktlaenge = 1;
                            aktstart = zaehler;
                            iKettenStatus = 1;
                        }
                    }
                    break;
                }

#if DEBUGV || DEBUGV_GLOBAL
				printf("Kette %d: aktstart %d aktlen %d status %d\n",
					zaehler,
					aktstart,
					aktlaenge,
					iKettenStatus
					);
#endif

			}  // for (zaehler=0; zaehler<iKonturAnzahl; zaehler++)


#if DEBUGV || DEBUGV_GLOBAL
			printf("Kette end: aktstart %d aktlen %d status %d\n",
				aktstart,
				aktlaenge,
				iKettenStatus
				);
#endif


			// End-Status pruefen
			// iKettenStatus = 0, 1, 2, 3, 4, 5, 6, 7, 8
			if (iKettenStatus == 9)  // Kette
			{
				AddNeueKette(aktstart, aktlaenge);
			}
			else if (iKettenStatus == 10)  // Kette + Luecke
			{
				aktlaenge--;
				AddNeueKette(aktstart, aktlaenge);
			}
			else if (iKettenStatus == 11)  // Kette + Luecke + 1 KP
			{
				aktlaenge -= 2;
				AddNeueKette(aktstart, aktlaenge);
				// KP 1 weg
				Kontur1[iKonturAnzahl - 1].Del = true;
			}
			else if (iKettenStatus == 12)  // Kette + Luecke + 2 KP
			{
				// Kanten Luecke fuellen!
				Kontur1[iKonturAnzahl - 3].XL = (Kontur1[iKonturAnzahl - 4].XL + Kontur1[iKonturAnzahl - 2].XL) / 2;
				Kontur1[iKonturAnzahl - 3].XR = (Kontur1[iKonturAnzahl - 4].XR + Kontur1[iKonturAnzahl - 2].XR) / 2;

				AddNeueKette(aktstart, aktlaenge);
			}
			else if (iKettenStatus == 13)  // Kette + Luecke + 2 KP + Luecke
			{
				// Kanten Luecke 2 fuellen!
				Kontur1[iKonturAnzahl - 4].XL = (Kontur1[iKonturAnzahl - 5].XL + Kontur1[iKonturAnzahl - 3].XL) / 2;
				Kontur1[iKonturAnzahl - 4].XR = (Kontur1[iKonturAnzahl - 5].XR + Kontur1[iKonturAnzahl - 3].XR) / 2;

				aktlaenge--;
				AddNeueKette(aktstart, aktlaenge);
			}
			else if (iKettenStatus == 14)  // Kette + Luecke + 2 KP + Luecke + 1 KP
			{
				// Kanten Luecke 2 fuellen!
				Kontur1[iKonturAnzahl - 5].XL = (Kontur1[iKonturAnzahl - 6].XL + Kontur1[iKonturAnzahl - 4].XL) / 2;
				Kontur1[iKonturAnzahl - 5].XR = (Kontur1[iKonturAnzahl - 6].XR + Kontur1[iKonturAnzahl - 4].XR) / 2;

				aktlaenge -= 2;
				AddNeueKette(aktstart, aktlaenge);
				// KP 1 weg
				Kontur1[iKonturAnzahl - 1].Del = true;
			}
			else if (iKettenStatus == 15)  // Kette + Luecke + 2 KP + Luecke + 2 KP
			{
				// Kanten Luecke 2 fuellen!
				Kontur1[iKonturAnzahl - 6].XL = (Kontur1[iKonturAnzahl - 7].XL + Kontur1[iKonturAnzahl - 5].XL) / 2;
				Kontur1[iKonturAnzahl - 6].XR = (Kontur1[iKonturAnzahl - 7].XR + Kontur1[iKonturAnzahl - 5].XR) / 2;

				aktlaenge -= 3;
				AddNeueKette(aktstart, aktlaenge);
				// KP 1 und 2 weg
				Kontur1[iKonturAnzahl - 2].Del = true;
				Kontur1[iKonturAnzahl - 1].Del = true;
			}
			else if (iKettenStatus == 16)  // 3 KP + Luecke + 2 KP
			{
				// Kanten Luecke fuellen!
				Kontur1[iKonturAnzahl - 3].XL = (Kontur1[iKonturAnzahl - 4].XL + Kontur1[iKonturAnzahl - 2].XL) / 2;
				Kontur1[iKonturAnzahl - 3].XR = (Kontur1[iKonturAnzahl - 4].XR + Kontur1[iKonturAnzahl - 2].XR) / 2;

				AddNeueKette(aktstart, aktlaenge);
			}
			// iKettenStatus = 17, 18
			else if (iKettenStatus == 19)  // Kette + BadMin
			{
				aktlaenge--;
				AddNeueKette(aktstart, aktlaenge);
				// BadMin weg
				Kontur1[iKonturAnzahl - 1].Del = true;
			}
			else if (iKettenStatus == 20)  // Kette + BadMin + KP
			{
				aktlaenge -= 2;
				AddNeueKette(aktstart, aktlaenge);
				// BadMin und KP 1 weg
				Kontur1[iKonturAnzahl - 2].Del = true;
				Kontur1[iKonturAnzahl - 1].Del = true;
			}

		} // SearchforChainsCont


		void NoSeamFind::CalcChainsKanten(int iMaxKantenDelta)
		{
			int end;
			int leftMin = 0;  ///< Linke MaxXPos der linken Kanten
			int leftMax = 0;  ///< Rechte MaxXPos der linken Kanten
			int rightMin = 0;  ///< Linke MaxXPos der rechten Kanten
			int rightMax = 0;  ///< Rechte MaxXPos der rechten Kanten


			// Nur laengste Kette behalten
			if ((LK[1].laenge > LK[0].laenge)
				&& (LK[2].laenge > LK[1].laenge)
				)
			{
				// LK3 ist groesste

				// Erst LK1 loeschen
				if (LK[0].laenge)
				{
					end = LK[0].start + LK[0].laenge;

					for (int i = LK[0].start; i < end; i++)
					{
						Kontur1[i].Del = true;
					}
				}

				// Jetzt LK2 loeschen
				if (LK[1].laenge)
				{
					end = LK[1].start + LK[1].laenge;

					for (int i = LK[1].start; i < end; i++)
					{
						Kontur1[i].Del = true;
					}

					LK[1].laenge = 0;
				}

				// Jetzt Daten von LK3 in LK1 kopieren
				LK[0].laenge = LK[2].laenge;
				LK[0].start = LK[2].start;
				LK[0].m = LK[2].m;
				LK[0].b = LK[2].b;
				LK[0].mGW = LK[2].mGW;
				LK[0].mX = LK[2].mX;
				LK[0].minX = LK[2].minX;
				LK[0].maxX = LK[2].maxX;
				LK[0].x1 = LK[2].x1;
				LK[0].y1 = LK[2].y1;
				LK[0].x2 = LK[2].x2;
				LK[0].y2 = LK[2].y2;

				// Jetzt LK3 loeschen
				end = LK[2].start + LK[2].laenge;

				for (int i = LK[2].start; i < end; i++)
				{
					Kontur1[i].Del = true;
				}

				LK[2].laenge = 0;
			}

			else if ((LK[1].laenge > LK[0].laenge)
				&& (LK[1].laenge > LK[2].laenge)
				)
			{
				// LK2 ist groesste

				// Erst LK1 loeschen
				if (LK[0].laenge)
				{
					end = LK[0].start + LK[0].laenge;

					for (int i = LK[0].start; i < end; i++)
					{
						Kontur1[i].Del = true;
					}
				}

				// Jetzt LK3 loeschen
				if (LK[2].laenge)
				{
					end = LK[2].start + LK[2].laenge;

					for (int i = LK[2].start; i < end; i++)
					{
						Kontur1[i].Del = true;
					}

					LK[2].laenge = 0;
				}

				// Jetzt Daten von LK2 in LK1 kopieren
				LK[0].laenge = LK[1].laenge;
				LK[0].start = LK[1].start;
				LK[0].m = LK[1].m;
				LK[0].b = LK[1].b;
				LK[0].mGW = LK[1].mGW;
				LK[0].mX = LK[1].mX;
				LK[0].minX = LK[1].minX;
				LK[0].maxX = LK[1].maxX;
				LK[0].x1 = LK[1].x1;
				LK[0].y1 = LK[1].y1;
				LK[0].x2 = LK[1].x2;
				LK[0].y2 = LK[1].y2;

				// Jetzt LK2 loeschen
				end = LK[1].start + LK[1].laenge;

				for (int i = LK[1].start; i < end; i++)
				{
					Kontur1[i].Del = true;
				}

				LK[1].laenge = 0;
			}

			else
			{
				// LK1 ist groesste oder gar keine Kette !!

				// Erst LK2 loeschen
				if (LK[1].laenge)
				{
					end = LK[1].start + LK[1].laenge;

					for (int i = LK[1].start; i < end; i++)
					{
						Kontur1[i].Del = true;
					}

					LK[1].laenge = 0;
				}

				// Jetzt LK3 loeschen
				if (LK[2].laenge)
				{
					end = LK[2].start + LK[2].laenge;

					for (int i = LK[2].start; i < end; i++)
					{
						Kontur1[i].Del = true;
					}

					LK[2].laenge = 0;
				}
			}

			// Jetzt das Gezappel der langen Kette bestimmen
			// Gibt's ueberhaupt eine Kette ?
			if (LK[0].laenge > 0)
			{
				    leftMin = leftMax = Kontur1[LK[0].start].XL;
				    rightMin = rightMax = Kontur1[LK[0].start].XR;

				int end = LK[0].start + LK[0].laenge;

				for (int i = LK[0].start + 1; i < end; i++)
				{
					if (Kontur1[i].XL < leftMin)
						          leftMin = Kontur1[i].XL;
					if (Kontur1[i].XL > leftMax)
						          leftMax = Kontur1[i].XL;
					if (Kontur1[i].XR < rightMin)
						          rightMin = Kontur1[i].XR;
					if (Kontur1[i].XR > rightMax)
						          rightMax = Kontur1[i].XR;
				}

				// Max - Min > Delta ?
				if (((leftMax - leftMin) > iMaxKantenDelta)
					&& ((rightMax - rightMin) > iMaxKantenDelta)
					)
				{
					LK[0].KantenIO = true;
				}
				else
					LK[0].KantenIO = false;
			}
		}


		void NoSeamFind::CalcBoxMulti(int nr)
			// Berechnet fuer die Kette 'nr' die Daten/Parameter.
			// nr = Nummer der Kette !!
		{
			int counter = 0;

			NoSeamResult[nr].bFilled = false;

			if (LK[nr].laenge == 0) return; // Box nicht existent, also i.O.

			NoSeamResult[nr].bFilled = true; // Kette da, nu wird gerechnet

			//if (LK[nr].mGW>80) return true;
			//if (LK[nr].mGW<27) return true;

			// Boxparameter
			int x1 = LK[nr].x1;
			int y1 = LK[nr].y1;
			int x2 = LK[nr].x2;
			int y2 = LK[nr].y2;

			int BoxMitteX = (x1 + x2) / 2;

			if ((BoxMitteX < 20) || ((_width - BoxMitteX) < 20) || (x1>_width) || (x2<5) || (y1<5) || (y1>_height) || (y2<5) || (y2>_height))
			{
				NoSeamResult[nr].bFilled = false;
				return;
			}
			else
			{
				if (x1<5)
				{
					x1 = 5;
				}
				if (x2>_width)
				{
					x2 = _width - 5;
				}
			}
			//min/max-Distanz Kette in X
			int iDistKetteX = LK[nr].maxX - LK[nr].minX + 1;
			int iSummengroesseX;

			// Gardienten Bereich in X je nach Breite Kette anpassen
			if (iDistKetteX <= 2) iSummengroesseX = 3;
			else if (iDistKetteX < 9) iSummengroesseX = 5;
			else iSummengroesseX = 15;

			//printf("CalcBoxMulti: iDistKetteX: %d iSummengroesseX: %d\n", iDistKetteX, iSummengroesseX);

			//Box-Summer bereitstellen;
			BoxSummer Sum1(_image);
			BoxSummer Sum2(_image);
			BoxSummer SumGW(_image);

			NoSeamResult[nr].box_x1 = x1;
			NoSeamResult[nr].box_y1 = y1;
			NoSeamResult[nr].box_x2 = x2;
			NoSeamResult[nr].box_y2 = y2;

			NoSeamResult[nr].greyvalChain = (int)(LK[nr].mGW + 0.5);


			// Gradienten entlang jeder Zeile berechnen
			int ws_grad = 0, ws_grad_max = 0;
			double ws_grad_sum = 0, sw_grad_sum = 0;
			int grad_sum1 = 0, grad_sum2 = 0;
			int ws_grad_max_GW = 9999, sw_grad_max_GW = 9999;
			int sw_grad = 0, sw_grad_max = 0;
			int sw_grad_max_index = 0, ws_grad_max_index = 0;
			int iBreite;
			//double dBreitenmittelwert=0;
			double SummXX = 0;
			double SummX = 0;
			//double SummXX1 = 0;
			//double SummX1=0;
			//double SummXX1=0;
			//double SummX2=0;
			//double SummXX2=0;


			double mittel_sw = 0, mittel_ws = 0, mittel_sw_ws;

			int iNoProblem = 0;
			int iMaxNoProblem = 0;

			double lauflaenge_links = 0, lauflaenge_rechts = 0;
			int lastleft = 0, lastright = 0, iDiffL = 0, iDiffR = 0;

			const int NoOfBuckets = 500;
			const double PercentInBuckets = (100.0 - __PER_CENT_KILL) / 100.0;
			int BucketL[NoOfBuckets];
			int BucketR[NoOfBuckets];
			for (int q = 0; q<NoOfBuckets; q++) BucketL[q] = BucketR[q] = 0;

			int ws[1000];
			int sw[1000];
			int sw_ws[1000];
			int iArrayAnzahl = 0;

			bool bZeileOK;
			int iZeileOKZaehler = 0;


			const int iMinBreite = 3;
			const int iMaxBreite = 30;

			bool bErsteZeile = true;

			int mGWsum = 0, mGWsum_count = 0;


			// Gradienten-Berechnung
			// ---------------------

            const int speeder = 2;
			for (int zeile = y1 + 2; zeile <= y2 - 2; zeile += speeder)
			{
				counter++;
				ws_grad_max = sw_grad_max = -10000; // sollte beim ersten Mal ueberschrieben werden
				ws_grad_max_GW = sw_grad_max_GW = 9999;

				//printf("**3**\n");

				for (int spalte = x1 + iSummengroesseX - 1; spalte<x2 - iSummengroesseX; spalte += 2)
				{
					// Gradient Weiss-Schwarz behandeln
					grad_sum1 = Sum1.getSum(spalte - iSummengroesseX - 1, zeile - 1, spalte, zeile + 1);
					grad_sum2 = Sum2.getSum(spalte + 1, zeile - 1, spalte + iSummengroesseX, zeile + 1);
					ws_grad = grad_sum1 - grad_sum2;
					//ws_grad = Sum1.getSum(spalte-iSummengroesseX-1, zeile-1, spalte, zeile+1)
					//  - Sum2.getSum(spalte+1, zeile-1, spalte+iSummengroesseX, zeile+1);

					// nur bei breiter Kette
					if (iDistKetteX > 5)
					{
						if (ws_grad>ws_grad_max) // groesseren Gradienten gefunden
						{
							ws_grad_max = ws_grad;
							ws_grad_max_index = spalte;
							ws_grad_max_GW = grad_sum2;
						}
					}
					// nur bei schmaler Kette
					else
					{
						if (ws_grad > ws_grad_max && grad_sum2 < ws_grad_max_GW) // groesseren  und niedrigsten Gradienten gefunden
						{
							ws_grad_max = ws_grad;
							ws_grad_max_index = spalte;
							ws_grad_max_GW = grad_sum2;
						}
						// findet noch ein dunkleren Gradient im aehnlichen Level
						else if (ws_grad >((int)0.8 * ws_grad_max) && ws_grad > 2000 && grad_sum2 < ws_grad_max_GW)
						{
							ws_grad_max = ws_grad;
							ws_grad_max_index = spalte;
							ws_grad_max_GW = grad_sum2;
							//printf("Z %d S %d Sum1: %d Sum2: %d ws_grad %d ws_grad_max %d ws_grad_max_GW %d\n", zeile, spalte, grad_sum1, grad_sum2, ws_grad,ws_grad_max, ws_grad_max_GW);
						}
					}

					// Gradient Schwarz-Weiss behandeln
					sw_grad = -ws_grad;

					// nur bei breiter Kette
					if (iDistKetteX > 5)
					{

						if (sw_grad>sw_grad_max) // groesseren Gradienten gefunden
						{
							sw_grad_max = sw_grad;
							sw_grad_max_index = spalte;
							sw_grad_max_GW = grad_sum1;
						}

					}
					// nur bei schmaler Kette
					else
					{
						if (sw_grad > sw_grad_max && grad_sum1 < sw_grad_max_GW) // groesseren  und niedrigsten Gradienten gefunden
						{
							sw_grad_max = sw_grad;
							sw_grad_max_index = spalte;
							sw_grad_max_GW = grad_sum1;
						}
						// findet noch ein dunkleren Gradient im aehnlichen Level
						else if (sw_grad >((int)0.8 * sw_grad_max) && sw_grad > 2000 && grad_sum1 < sw_grad_max_GW)
						{
							sw_grad_max = ws_grad;
							sw_grad_max_index = spalte;
							sw_grad_max_GW = grad_sum2;
							//printf("WWZ %d S %d Sum1: %d Sum2: %d sw_grad %d sw_grad_max %d sw_grad_max_GW %d\n", zeile, spalte, grad_sum1, grad_sum2, sw_grad,sw_grad_max, sw_grad_max_GW);
						}
					}

				} // for ueber spalten

				//putPixel (nXStart + iLoop, nYStart, nColor);
				//printf("**a1**\n");

				int div = iSummengroesseX * 3;
				ws_grad_max = ws_grad_max / div; // Bugfix aus Souvis
				sw_grad_max = sw_grad_max / div; // uebernommen

				if (bErsteZeile) // erste Zeile
				{
					bErsteZeile = false;
					lastleft = ws_grad_max_index;
					lastright = sw_grad_max_index;
				}
				else
				{
					iDiffL = absInt(lastleft - ws_grad_max_index); //Diff. zum Vorherigen
					lauflaenge_links += iDiffL;
					BucketL[iDiffL]++;
					lastleft = ws_grad_max_index;

					iDiffR = absInt(lastright - sw_grad_max_index);
					lauflaenge_rechts += iDiffR;
					BucketR[iDiffR]++;
					lastright = sw_grad_max_index;
				}

				//ws_grad_max_index += 2;
				//sw_grad_max_index += 2;

				// Grauwert Spalt Berechnung ueber gefundene Gardkanten NEU
				if (lastleft > lastright)
				{
					mGWsum += SumGW.getSum(lastright, zeile, lastleft, zeile);
					mGWsum_count += (lastleft - lastright + 1);
				}
				else
				{
					mGWsum += SumGW.getSum(lastleft, zeile, lastright, zeile);
					mGWsum_count += (lastright - lastleft + 1);
				}

				if (_displayParameter == 16)
				{
					_overlay.addPoint(ws_grad_max_index, zeile, Color::Red());
					_overlay.addPoint(sw_grad_max_index, zeile, Color::Green());

				}

				iBreite = sw_grad_max_index - ws_grad_max_index;
				iBreite = std::abs(iBreite); 	// Neu auf Wunsch von VOEST

				bZeileOK = (iBreite >= iMinBreite) && (iBreite <= iMaxBreite); // ist Breite im Rahmen?

				// Testen auf gute Zeilen hintereinander:
				if (bZeileOK)
				{
					iNoProblem++;
				}
				else
				{
					if (iNoProblem>iMaxNoProblem) iMaxNoProblem = iNoProblem;
					iNoProblem = 0;
				}

				SummX += iBreite;
				SummXX += iBreite * iBreite;

				ws_grad_sum += ws_grad_max;
				sw_grad_sum += sw_grad_max;

				sw[iArrayAnzahl] = sw_grad_max_index;
				ws[iArrayAnzahl] = ws_grad_max_index;

				//		if (bZeileOK) // if evtl. raus
				//		{
				mittel_sw += sw_grad_max_index;
				mittel_ws += ws_grad_max_index;
				iZeileOKZaehler++;
				//		}

				sw_ws[iArrayAnzahl] = iBreite;
				iArrayAnzahl++;

			} // for ueber zeilen
			//printf("**2**\n");

			// Grauwert Spart auswerten
			NoSeamResult[nr].greyvalAreaInEdge = mGWsum / mGWsum_count;
			//printf("CalcBoxMulti: GrauwertMittel %.2f\n", NoSeamResult[nr].greyvalAreaInEdge);


			// BucketSort
			// ----------
			// Hier wird ein BucketSort durchgefuehrt. BucketSort sortiert, indem fuer jeden vorkommenen Wert ein "Eimer" angelegt wird. Jedes Element wandert dann in einen dieser Eimer
			// indem die Anzahl darin eins hochgezaehlt wird.

			int iCurNo = 0;
			double BucketSumL = 0, BucketSumR = 0;
			int iMustNo;
			iMustNo = (int)((counter)*PercentInBuckets + 0.5);
			int iCurBucket = -1;
			int tooMuch;

			// BucketL
			while (iCurNo<iMustNo)
			{
				iCurBucket++;
				BucketSumL += iCurBucket * BucketL[iCurBucket];
				iCurNo += BucketL[iCurBucket];
			}
			tooMuch = iCurNo - iMustNo; // zuviele fuer exakten Prozentwert
			BucketSumL -= tooMuch * iCurBucket; // die zuviel sind wieder raus

			// BucketR
			iCurNo = 0; iCurBucket = -1;
			while (iCurNo<iMustNo)
			{
				iCurBucket++;
				BucketSumR += iCurBucket * BucketR[iCurBucket];
				iCurNo += BucketR[iCurBucket];
			}
			tooMuch = iCurNo - iMustNo; // zuviele fuer exakten Prozentwert
			BucketSumR -= tooMuch * iCurBucket; // die zuviel sind wieder raus

			BucketSumL /= iMustNo;
			BucketSumR /= iMustNo;
			// Buckets ausgewertet
			//printf("**1**\n");

			//if (SeamPar.IRob.iDisplay==16) printf("Problemlose Zeilen am Stueck: %d\n", iMaxNoProblem);

			lauflaenge_links = lauflaenge_links / (counter - 1);
			lauflaenge_rechts = lauflaenge_rechts / (counter - 1);
			mittel_sw_ws = SummX / (counter);
			mittel_sw /= iZeileOKZaehler;
			mittel_ws /= iZeileOKZaehler;

			//	if (SeamPar.IRob.iDisplay==16)
			//	{
			//		printf("LL li %f, LL re %f\n", lauflaenge_links, lauflaenge_rechts);
			//		printf("LL li %f, LL re %f (bereinigt)\n", BucketSumL, BucketSumR);
			//	}

			NoSeamResult[nr].LauflaengeLinks = BucketSumL;
			NoSeamResult[nr].LauflaengeRechts = BucketSumR;

			//printf("**Zeilen im Rahmen: %d\n", iZeileOKZaehler);

			//printf("WS-Grad %f\n", ws_grad_sum/(y2-y1+1));
			//printf("SW-Grad %f\n", sw_grad_sum/(y2-y1+1));
			//printf("Breite %f, MWlinks %f, MWrechts %f\n", mittel_sw_ws, mittel_ws, mittel_sw);

			ws_grad_sum = ws_grad_sum / (counter);
			sw_grad_sum = sw_grad_sum / (counter);

			NoSeamResult[nr].Gradient = (ws_grad_sum + sw_grad_sum)/2.0; //mittlerer Gradient aus links und recht ... damit ist das Max<=255


			//	// Standard-Abweichung provisorisch
			//	double dStdAbw;
			//	dStdAbw = sqrt(
			//						(mittel_sw_ws*mittel_sw_ws*(counter) - 2*mittel_sw_ws*SummX	+ SummXX)
			//								 / (counter) );
			//	//printf("StdAbw %f\n", dStdAbw);


			// Nach-Kalkulation
			// ----------------

			// erneute Statistik ohne Ausreisser ueber die linke und rechte Kante
			// sowie ueber die Breite

			double SummX1 = 0;
			double SummXX1 = 0;
			//  double StdAbw1;
			double Mittel1;
			int Anzahl1 = 0;

			double SummX2 = 0;
			double SummXX2 = 0;
			//  double StdAbw2;
			double Mittel2;
			int Anzahl2 = 0;

			double SummX3 = 0;
			double SummXX3 = 0;
			//  double StdAbw3;
			double Mittel3;
			int Anzahl3 = 0;

			for (int z = 0; z<iArrayAnzahl; z++)
			{
				if (fabs(ws[z] - mittel_ws)<25)
				{
					Anzahl1++;
					SummX1 += ws[z];
					SummXX1 += ws[z] * ws[z];
				}

				if (fabs(sw[z] - mittel_sw)<25)
				{
					Anzahl2++;
					SummX2 += sw[z];
					SummXX2 += sw[z] * sw[z];
				}

				if (fabs(sw_ws[z] - mittel_sw_ws)<25)
				{
					Anzahl3++;
					SummX3 += sw_ws[z];
					SummXX3 += sw_ws[z] * sw_ws[z];
				}
			} // for ueber Anzahl im Array

			if (!Anzahl1) Anzahl1 = 1;
			if (!Anzahl2) Anzahl2 = 1;
			if (!Anzahl3) Anzahl3 = 1;

			Mittel1 = SummX1 / Anzahl1;
			Mittel2 = SummX2 / Anzahl2;
			Mittel3 = SummX3 / Anzahl3;

			NoSeamResult[nr].left_x = (int)(Mittel1 + 0.5);
			NoSeamResult[nr].right_x = (int)(Mittel2 + 0.5);
			NoSeamResult[nr].width = (int)(Mittel3 + 0.5);

			//CB Wenn Gradientensuche fehlerhaft, dann Gradientenergebniss aus Minima-Suche
			int GradPosDiff = NoSeamResult[nr].right_x - NoSeamResult[nr].left_x;

			if (GradPosDiff <= 0)
			{
				//		int XLmax = 0, XRmin = 9999;
				int XLsum = 0, XRsum = 0;

				//printf("ClacBoxMulti: Kette Start %d Laenge %d\n", LK[0].start,LK[0].laenge);

				for (int x = 0; x < LK[0].laenge; x++)
				{
					//if (Kontur1[LK[0].start + x].XL > XLmax )  XLmax = Kontur1[LK[0].start+x].XL;
					//if (Kontur1[LK[0].start + x].XR < XRmin )  XRmin = Kontur1[LK[0].start+x].XR;
					XLsum += Kontur1[LK[0].start + x].XL;
					XRsum += Kontur1[LK[0].start + x].XR;
					//printf("ClacBoxMulti: Kontur Index %d XL %d XR %d\n",LK[0].start + x ,Kontur1[LK[0].start + x].XL, Kontur1[LK[0].start + x].XR );
					//printf("ClacBoxMulti: Left - XLsum %d XLmax %d -- Right - XRsum %d XRmin %d\n", XLsum, XLmax, XRsum, XRmin);
				}
				XLsum = (int)XLsum / LK[0].laenge;
				XRsum = (int)XRsum / LK[0].laenge;

				//printf("ClacBoxMulti: Left - XLsum %d XLmax %d \n", XLsum, XLmax);
				//printf("ClacBoxMulti: Right - XRsum %d XRmin %d \n", XRsum, XRmin);

				NoSeamResult[nr].left_x = XLsum;
				NoSeamResult[nr].right_x = XRsum;

			}


			// Standard-Abweichung
			// -------------------

			// GUR auskommentiert
			double StdAbw1 = sqrt( (Mittel1*Mittel1*Anzahl1 - 2*Mittel1*SummX1	+ SummXX1) / Anzahl1 );
			double StdAbw2 = sqrt( (Mittel2*Mittel2*Anzahl2 - 2*Mittel2*SummX2	+ SummXX2) / Anzahl2 );
			double StdAbw3 = sqrt( (Mittel3*Mittel3*Anzahl3 - 2*Mittel3*SummX3	+ SummXX3) / Anzahl3 );
			//
			NoSeamResult[nr].StdAbwLinks = StdAbw1;
			NoSeamResult[nr].StdAbwRechts= StdAbw2;
			NoSeamResult[nr].StdAbwBreite= StdAbw3;



			// GUR neu

			int   iStart, iEnd;
			int   iMeanXPosL = 0;
			int   iMeanXPosR = 0;
			int   iMeanXWidth = 0;
			int   iAnzMean = 0;
			int   iDeltaXL = 0;
			int   iSumDeltaXL = 0;
			int   iDeltaXR = 0;
			int   iSumDeltaXR = 0;
			int   iDeltaWid = 0;
			int   iSumDeltaWid = 0;

			int   iMeanLaufLi = 0;
			int   iMeanLaufRe = 0;


			iStart = LK[nr].start;
			iEnd = iStart + LK[nr].laenge;

			// Mittlere X-Pos der linken bzw. rechten Kante
			for (int i = iStart; i < iEnd; i++)
			{
				if (!Kontur1[i].Del)
				{
					iMeanXPosL += Kontur1[i].XL;
					iMeanXPosR += Kontur1[i].XR;
					iMeanXWidth += Kontur1[i].XR - Kontur1[i].XL;
					iAnzMean++;
				}
			}

			iMeanXPosL /= iAnzMean;
			iMeanXPosR /= iAnzMean;
			iMeanXWidth /= iAnzMean;

#if DEBUGV || DEBUGV_GLOBAL
			printf("FDS %d StdAbw:  meanL %d meanR %d meanW %d\n",
				nr,
				iMeanXPosL,
				iMeanXPosR,
				iMeanXWidth
				);
			printf("FDS  start %d end %d\n", iStart, iEnd);
#endif

			// Abweichung von den mittleren Kanten-Positionen
			for (int i = iStart; i < iEnd; i++)
			{
				if (!Kontur1[i].Del)
				{
					//			iDeltaXL  += std::abs ( iKontur1_XL[i] - iMeanXPosL );
					//			iDeltaXR  += std::abs ( iKontur1_XR[i] - iMeanXPosR );
					//			iDeltaWid += std::abs ( iKontur1_XR[i] - iKontur1_XL[i] - iMeanXWidth );
					iDeltaXL = Kontur1[i].XL - iMeanXPosL;
					iMeanLaufLi += std::abs(iDeltaXL);
					iSumDeltaXL += iDeltaXL * iDeltaXL;
					iDeltaXR = Kontur1[i].XR - iMeanXPosR;
					iMeanLaufRe += std::abs(iDeltaXR);
					iSumDeltaXR += iDeltaXR * iDeltaXR;
					iDeltaWid = Kontur1[i].XR - Kontur1[i].XL - iMeanXWidth;
					iSumDeltaWid += iDeltaWid * iDeltaWid;
				}
			}



#if DEBUGV || DEBUGV_GLOBAL
			printf("FDS %d StdAbw:  StdAbwL %8.3f StdAbwR %8.3f StdAbwW %8.3f\n",
				nr,
				NoSeamResult[nr].StdAbwLinks,
				NoSeamResult[nr].StdAbwRechts,
				NoSeamResult[nr].StdAbwBreite
				);
#endif

			//NoSeamResult[nr].LauflaengeLinks = (double)iMeanLaufLi / 1000.0;
			//NoSeamResult[nr].LauflaengeRechts = (double)iMeanLaufRe / 1000.0;
			// Zuweisung ist im Souvis-Algo drin. Meines Erachtens ist die "alte" Verion hier absolut korrekt (OS)


			// Jetzt die gefundenen Lauflaengen nach-skalieren:  wenn Kette kurz  =>  Lauflaenge "verlaengern"
			// Messw_aktuell = Messw  +    Anz_Pte_ROI - Anz_Pte_FDS   x  2  x  Messw
			//                             -------------------------
			//                                    Anz_Pte_ROI
			double  dLLFaktor = 3.0 - 2.0 * (double)LK[nr].laenge / (double)(y2 - y1) * 10.0;
			dLLFaktor = 1.0;
			NoSeamResult[nr].LauflaengeLinks *= dLLFaktor;
			NoSeamResult[nr].LauflaengeRechts *= dLLFaktor;

#if DEBUGV || DEBUGV_GLOBAL
			printf("NSF: Kette %d, LaufLi %d, LaufRe %d, FaktorLaufl %8.5f\n",
				nr,
				iMeanLaufLi,
				iMeanLaufRe,
				dLLFaktor
				);
#endif

			// Ende GUR neu

		}


		void NoSeamFind::CheckBoxMulti(int nr)
		{
			SNoSeamResult noSeamResult = NoSeamResult[nr];

			CalcGreyVals(noSeamResult.box_x1, noSeamResult.box_y1, noSeamResult.box_x2, noSeamResult.box_y2, noSeamResult.left_x, noSeamResult.right_x,
				             noSeamResult.greyvalAreaIn, noSeamResult.greyvalAreaOut);

			NoSeamResult[nr] = noSeamResult;
		}

		void NoSeamFind::DrawBoxes(bool drawIO)
			// Malt die umschreibenden Rechtecke. Ist drawIO==false wird nichts gemalt.
		{
			for (int i = 0; i<3; i++)
			{
				if (LK[i].laenge && drawIO)
				{
					_overlay.addRectangle(LK[i].x1, LK[i].y1, LK[i].x2 - LK[i].x1, LK[i].y2 - LK[i].y1, Color::Green());
					//drawRectangle(LK[i].x1, LK[i].y1, LK[i].x2, LK[i].y2, INT_DBG_COLOR_GREEN);
				}
			}
		}


		void NoSeamFind::DrawKontur_1()
			// Gibt die aktuellen Konturpunkte der Gruppe-1-Minima aus (kleine Rechtecke).
			// Natuerlich nur die, die das Del-Flag nicht gesetzt haben.
		{
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				if (!Kontur1[i].Del)
				{
					if (Kontur1[i].DispCol == 0)
					{
						_overlay.addRectangle(Kontur1[i].xPos - 2, Kontur1[i].zeile - 2, 4, 4, Color::Red());
						//drawRectangle(Kontur1[i].xPos - 2, Kontur1[i].zeile - 2, Kontur1[i].xPos + 2, Kontur1[i].zeile + 2, INT_DBG_COLOR_RED);
					}
					else
					{
						_overlay.addRectangle(Kontur1[i].xPos - 2, Kontur1[i].zeile - 2, 4, 4, Color::Cyan());
						//drawRectangle(Kontur1[i].xPos - 2, Kontur1[i].zeile - 2, Kontur1[i].xPos + 2, Kontur1[i].zeile + 2, INT_DBG_COLOR_CYAN);
					}
				}
			}
		}


		void NoSeamFind::DrawKontur_2()
			// Gibt die aktuellen Konturpunkte der Gruppe-2-Minima aus (kleine Rechtecke).
			// Natuerlich nur die, die das Del-Flag nicht gesetzt haben.
		{
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				if (!Kontur2[i].Del)
				{
					if (Kontur2[i].DispCol == 0)
					{
						_overlay.addRectangle(Kontur2[i].xPos - 2, Kontur2[i].zeile - 2, 4, 4, Color::Red());
						//drawRectangle(Kontur2[i].xPos - 2, Kontur2[i].zeile - 2, Kontur2[i].xPos + 2, Kontur2[i].zeile + 2, INT_DBG_COLOR_RED);
					}
					else
					{
						_overlay.addRectangle(Kontur2[i].xPos - 2, Kontur2[i].zeile - 2, 4, 4, Color::Cyan());
						//drawRectangle(Kontur2[i].xPos - 2, Kontur2[i].zeile - 2, Kontur2[i].xPos + 2, Kontur2[i].zeile + 2, INT_DBG_COLOR_CYAN);
					}
				}
			}
		}


		void NoSeamFind::DrawKontur_3()
			// Gibt die aktuellen Konturpunkte der Gruppe-3-Minima aus (kleine Rechtecke).
			// Natuerlich nur die, die das Del-Flag nicht gesetzt haben.
		{
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				if (!Kontur3[i].Del)
				{
					if (Kontur3[i].DispCol == 0)
					{
						_overlay.addRectangle(Kontur3[i].xPos - 2, Kontur3[i].zeile - 2, 4, 4, Color::Red());
						//drawRectangle(Kontur3[i].xPos - 2, Kontur3[i].zeile - 2, Kontur3[i].xPos + 2, Kontur3[i].zeile + 2, INT_DBG_COLOR_RED);
					}
					else
					{
						_overlay.addRectangle(Kontur3[i].xPos - 2, Kontur3[i].zeile - 2, 4, 4, Color::Cyan());
						//drawRectangle(Kontur3[i].xPos - 2, Kontur3[i].zeile - 2, Kontur3[i].xPos + 2, Kontur3[i].zeile + 2, INT_DBG_COLOR_CYAN);
					}
				}
			}
		}


		void NoSeamFind::DrawXPos()
			// Gibt die akteullen Konturpunkte aus (kleine Rechtecke). Natuerlich nur die, die
			// das Del-Flag nicht gesetzt haben.
		{
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				// Minima Gruppe 1
				if (!Kontur1[i].Del)
				{
					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile, Color::Green());
					//putPixel(Kontur1[i].xPos, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+1, Color::Green());
					//putPixel(Kontur1[i].xPos, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+2, Color::Green());
					//putPixel(Kontur1[i].xPos, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+3, Color::Green());
					//putPixel(Kontur1[i].xPos, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile, Color::Green());
					//putPixel(Kontur1[i].xPos + 1, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile+1, Color::Green());
					//putPixel(Kontur1[i].xPos + 1, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile+2, Color::Green());
					//putPixel(Kontur1[i].xPos + 1, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile+3, Color::Green());
					//putPixel(Kontur1[i].xPos + 1, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);
				}
				// Minima Gruppe 2
				if (!Kontur2[i].Del)
				{
					_overlay.addPoint(Kontur2[i].xPos, Kontur2[i].zeile, Color::Red());
					//putPixel(Kontur2[i].xPos, Kontur2[i].zeile, INT_DBG_COLOR_RED);

					_overlay.addPoint(Kontur2[i].xPos, Kontur2[i].zeile+1, Color::Red());
					//putPixel(Kontur2[i].xPos, Kontur2[i].zeile + 1, INT_DBG_COLOR_RED);

					_overlay.addPoint(Kontur2[i].xPos, Kontur2[i].zeile+2, Color::Red());
					//putPixel(Kontur2[i].xPos, Kontur2[i].zeile + 2, INT_DBG_COLOR_RED);

					_overlay.addPoint(Kontur2[i].xPos, Kontur2[i].zeile+3, Color::Red());
					//putPixel(Kontur2[i].xPos, Kontur2[i].zeile + 3, INT_DBG_COLOR_RED);

					_overlay.addPoint(Kontur2[i].xPos+1, Kontur2[i].zeile, Color::Red());
					//putPixel(Kontur2[i].xPos + 1, Kontur2[i].zeile, INT_DBG_COLOR_RED);

					_overlay.addPoint(Kontur2[i].xPos+1, Kontur2[i].zeile+1, Color::Red());
					//putPixel(Kontur2[i].xPos + 1, Kontur2[i].zeile + 1, INT_DBG_COLOR_RED);

					_overlay.addPoint(Kontur2[i].xPos+1, Kontur2[i].zeile+2, Color::Red());
					//putPixel(Kontur2[i].xPos + 1, Kontur2[i].zeile + 2, INT_DBG_COLOR_RED);

					_overlay.addPoint(Kontur2[i].xPos+1, Kontur2[i].zeile+3, Color::Red());
					//putPixel(Kontur2[i].xPos + 1, Kontur2[i].zeile + 3, INT_DBG_COLOR_RED);
				}
				// Minima Gruppe 3
				if (!Kontur3[i].Del)
				{
					_overlay.addPoint(Kontur3[i].xPos, Kontur3[i].zeile, Color::Cyan());
					//putPixel(Kontur3[i].xPos, Kontur3[i].zeile, INT_DBG_COLOR_CYAN);

					_overlay.addPoint(Kontur3[i].xPos, Kontur3[i].zeile+1, Color::Cyan());
					//putPixel(Kontur3[i].xPos, Kontur3[i].zeile + 1, INT_DBG_COLOR_CYAN);

					_overlay.addPoint(Kontur3[i].xPos, Kontur3[i].zeile+2, Color::Cyan());
					//putPixel(Kontur3[i].xPos, Kontur3[i].zeile + 2, INT_DBG_COLOR_CYAN);

					_overlay.addPoint(Kontur3[i].xPos, Kontur3[i].zeile+3, Color::Cyan());
					//putPixel(Kontur3[i].xPos, Kontur3[i].zeile + 3, INT_DBG_COLOR_CYAN);

					_overlay.addPoint(Kontur3[i].xPos+1, Kontur3[i].zeile, Color::Cyan());
					//putPixel(Kontur3[i].xPos + 1, Kontur3[i].zeile, INT_DBG_COLOR_CYAN);

					_overlay.addPoint(Kontur3[i].xPos+1, Kontur3[i].zeile+1, Color::Cyan());
					//putPixel(Kontur3[i].xPos + 1, Kontur3[i].zeile + 1, INT_DBG_COLOR_CYAN);

					_overlay.addPoint(Kontur3[i].xPos+1, Kontur3[i].zeile+2, Color::Cyan());
					//putPixel(Kontur3[i].xPos + 1, Kontur3[i].zeile + 2, INT_DBG_COLOR_CYAN);

					_overlay.addPoint(Kontur3[i].xPos+1, Kontur3[i].zeile+3, Color::Cyan());
					//putPixel(Kontur3[i].xPos + 1, Kontur3[i].zeile + 3, INT_DBG_COLOR_CYAN);
				}
			}
		}


		void NoSeamFind::DrawXPosKanten()
			// Gibt die linke und rechte Kante der aktuellen Konturpunkte aus. Natuerlich nur die, die
			// das Del-Flag nicht gesetzt haben.
		{
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				// Minima Gruppe 1
				if (!Kontur1[i].Del)
				{
					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile, Color::Green());
					//putPixel(Kontur1[i].XL, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile+1, Color::Green());
					//putPixel(Kontur1[i].XL, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile+2, Color::Green());
					//putPixel(Kontur1[i].XL, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile+3, Color::Green());
					//putPixel(Kontur1[i].XL, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile, Color::Green());
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile+1, Color::Green());
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile+2, Color::Green());
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile+3, Color::Green());
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);


					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile, Color::Green());
					//putPixel(Kontur1[i].XR, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile+1, Color::Green());
					//putPixel(Kontur1[i].XR, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile+2, Color::Green());
					//putPixel(Kontur1[i].XR, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile+3, Color::Green());
					//putPixel(Kontur1[i].XR, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XR+1, Kontur1[i].zeile, Color::Green());
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XR+1, Kontur1[i].zeile+1, Color::Green());
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XR+1, Kontur1[i].zeile+2, Color::Green());
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);

					_overlay.addPoint(Kontur1[i].XR+1, Kontur1[i].zeile+3, Color::Green());
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);
				}
				// Minima Gruppe 2
				if (!Kontur2[i].Del)
				{
					_overlay.addPoint(Kontur2[i].XL, Kontur2[i].zeile, Color::Red());
					_overlay.addPoint(Kontur2[i].XL, Kontur2[i].zeile + 1, Color::Red());
					_overlay.addPoint(Kontur2[i].XL, Kontur2[i].zeile + 2, Color::Red());
					_overlay.addPoint(Kontur2[i].XL, Kontur2[i].zeile + 3, Color::Red());
					_overlay.addPoint(Kontur2[i].XL + 1, Kontur2[i].zeile, Color::Red());
					_overlay.addPoint(Kontur2[i].XL + 1, Kontur2[i].zeile + 1, Color::Red());
					_overlay.addPoint(Kontur2[i].XL + 1, Kontur2[i].zeile + 2, Color::Red());
					_overlay.addPoint(Kontur2[i].XL + 1, Kontur2[i].zeile + 3, Color::Red());
					_overlay.addPoint(Kontur2[i].XR, Kontur2[i].zeile, Color::Red());
					_overlay.addPoint(Kontur2[i].XR, Kontur2[i].zeile + 1, Color::Red());
					_overlay.addPoint(Kontur2[i].XR, Kontur2[i].zeile + 2, Color::Red());
					_overlay.addPoint(Kontur2[i].XR, Kontur2[i].zeile + 3, Color::Red());
					_overlay.addPoint(Kontur2[i].XR + 1, Kontur2[i].zeile, Color::Red());
					_overlay.addPoint(Kontur2[i].XR + 1, Kontur2[i].zeile + 1, Color::Red());
					_overlay.addPoint(Kontur2[i].XR + 1, Kontur2[i].zeile + 2, Color::Red());
					_overlay.addPoint(Kontur2[i].XR + 1, Kontur2[i].zeile + 3, Color::Red());


				}
				// Minima Gruppe 3
				if (!Kontur3[i].Del)
				{
					_overlay.addPoint(Kontur3[i].XL, Kontur3[i].zeile, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XL, Kontur3[i].zeile + 1, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XL, Kontur3[i].zeile + 2, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XL, Kontur3[i].zeile + 3, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XL + 1, Kontur3[i].zeile, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XL + 1, Kontur3[i].zeile + 1, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XL + 1, Kontur3[i].zeile + 2, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XL + 1, Kontur3[i].zeile + 3, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR, Kontur3[i].zeile, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR, Kontur3[i].zeile + 1, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR, Kontur3[i].zeile + 2, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR, Kontur3[i].zeile + 3, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR + 1, Kontur3[i].zeile, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR + 1, Kontur3[i].zeile + 1, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR + 1, Kontur3[i].zeile + 2, Color::Cyan());
					_overlay.addPoint(Kontur3[i].XR + 1, Kontur3[i].zeile + 3, Color::Cyan());


				}
			}
		}


		void NoSeamFind::DrawXPosMin1()
			// Gibt die Konturpunkte der Min 1 aus (als rote Striche). Natuerlich nur die, die
			// das Del-Flag nicht gesetzt haben.
		{
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				// Minima Gruppe 1
				if (!Kontur1[i].Del)
				{
					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile, Color::Red());
					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+1, Color::Red());
					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+2, Color::Red());
					_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+3, Color::Red());

					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile, Color::Red());
					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile + 1, Color::Red());
					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile + 2, Color::Red());
					_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile + 3, Color::Red());


				}
			}
		}


		void NoSeamFind::DrawXPosMin1Kanten()
			// Gibt die Kanten der Konturpunkte der Min 1 aus. Natuerlich nur die, die
			// das Del-Flag nicht gesetzt haben.
		{
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				// Minima Gruppe 1
				if (!Kontur1[i].Del)
				{
					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile, Color::Green());
					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile+1, Color::Green());
					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile+2, Color::Green());
					_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile+3, Color::Green());
					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile, Color::Green());
					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile + 1, Color::Green());
					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile + 2, Color::Green());
					_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile + 3, Color::Green());

					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile, Color::Green());
					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 1, Color::Green());
					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 2, Color::Green());
					_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 3, Color::Green());
					_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile, Color::Green());
					_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 1, Color::Green());
					_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 2, Color::Green());
					_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 3, Color::Green());

					//putPixel(Kontur1[i].XL, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);
					//putPixel(Kontur1[i].XL, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);
					//putPixel(Kontur1[i].XL, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);
					//putPixel(Kontur1[i].XL, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile, INT_DBG_COLOR_GREEN);
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile + 1, INT_DBG_COLOR_GREEN);
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile + 2, INT_DBG_COLOR_GREEN);
					//putPixel(Kontur1[i].XL + 1, Kontur1[i].zeile + 3, INT_DBG_COLOR_GREEN);

					//putPixel(Kontur1[i].XR, Kontur1[i].zeile, INT_DBG_COLOR_BLUE);
					//putPixel(Kontur1[i].XR, Kontur1[i].zeile + 1, INT_DBG_COLOR_BLUE);
					//putPixel(Kontur1[i].XR, Kontur1[i].zeile + 2, INT_DBG_COLOR_BLUE);
					//putPixel(Kontur1[i].XR, Kontur1[i].zeile + 3, INT_DBG_COLOR_BLUE);
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile, INT_DBG_COLOR_BLUE);
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile + 1, INT_DBG_COLOR_BLUE);
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile + 2, INT_DBG_COLOR_BLUE);
					//putPixel(Kontur1[i].XR + 1, Kontur1[i].zeile + 3, INT_DBG_COLOR_BLUE);
				}
			}
		}


		void NoSeamFind::DrawKettePos(void)
			// Gibt die Konturpunkte der Kette aus.
			// Nur die laengste ist noch existent !!
		{
			// Gibt's Kette ?
			if (LK[0].laenge > 0)
			{
				int end = LK[0].start + LK[0].laenge;

				for (int i = LK[0].start; i < end; i++)
				{
					if (!Kontur1[i].Del)
					{
						if (LK[0].KantenIO)
						{
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile, Color::Green());
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+1, Color::Green());
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+2, Color::Green());
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile+3, Color::Green());
							_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile, Color::Green());
							_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile+1, Color::Green());
							_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile+2, Color::Green());
							_overlay.addPoint(Kontur1[i].xPos+1, Kontur1[i].zeile+3, Color::Green());


						}
						else
						{
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile, Color::Red());
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile + 1, Color::Red());
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile + 2, Color::Red());
							_overlay.addPoint(Kontur1[i].xPos, Kontur1[i].zeile + 3, Color::Red());
							_overlay.addPoint(Kontur1[i].xPos + 1, Kontur1[i].zeile, Color::Red());
							_overlay.addPoint(Kontur1[i].xPos + 1, Kontur1[i].zeile + 1, Color::Red());
							_overlay.addPoint(Kontur1[i].xPos + 1, Kontur1[i].zeile + 2, Color::Red());
							_overlay.addPoint(Kontur1[i].xPos + 1, Kontur1[i].zeile + 3, Color::Red());


						}
					}
				}
			}
		}


		void NoSeamFind::DrawKetteKante(void)
			// Gibt die Kantenpunkte der Kette aus.
			// Nur die laengste Kette ist noch existent !!
		{
			// Gibt's Kette ?
			if (LK[0].laenge > 0)
			{
				int end = LK[0].start + LK[0].laenge;

				for (int i = LK[0].start; i < end; i++)
				{
					if (!Kontur1[i].Del)
					{
						if (LK[0].KantenIO)
						{
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile, Color::Green());
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile + 1, Color::Green());
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile + 2, Color::Green());
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile + 3, Color::Green());
							_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile, Color::Green());
							_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile + 1, Color::Green());
							_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile + 2, Color::Green());
							_overlay.addPoint(Kontur1[i].XL+1, Kontur1[i].zeile + 3, Color::Green());



							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile, Color::Green());
							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 1, Color::Green());
							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 2, Color::Green());
							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 3, Color::Green());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile, Color::Green());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 1, Color::Green());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 2, Color::Green());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 3, Color::Green());


						}
						else
						{
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile, Color::Red());
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile + 1, Color::Red());
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile + 2, Color::Red());
							_overlay.addPoint(Kontur1[i].XL, Kontur1[i].zeile + 3, Color::Red());
							_overlay.addPoint(Kontur1[i].XL + 1, Kontur1[i].zeile, Color::Red());
							_overlay.addPoint(Kontur1[i].XL + 1, Kontur1[i].zeile + 1, Color::Red());
							_overlay.addPoint(Kontur1[i].XL + 1, Kontur1[i].zeile + 2, Color::Red());
							_overlay.addPoint(Kontur1[i].XL + 1, Kontur1[i].zeile + 3, Color::Red());



							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile, Color::Red());
							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 1, Color::Red());
							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 2, Color::Red());
							_overlay.addPoint(Kontur1[i].XR, Kontur1[i].zeile + 3, Color::Red());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile, Color::Red());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 1, Color::Red());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 2, Color::Red());
							_overlay.addPoint(Kontur1[i].XR + 1, Kontur1[i].zeile + 3, Color::Red());


						}
					}
				}
			}
		}







		/**********************************************************************
		* Description:  Loescht Punkte mit zu wenig Nachbarn.                *
		*                                                                    *
		* Parameter:    maxDiff1:     max. Pixeldiff. fuer Check bei Projekt *
		*                             Yudigar                                *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		void NoSeamFind::KillSingles2Rect(int maxDiff1)
		{
			bool tmp[ANZAHL_KONTUR];

			for (int k = 0; k<ANZAHL_KONTUR; k++)
				tmp[k] = Kontur1[k].Del;

			for (int i = 0; i<iKonturAnzahl; i++)
			{
				if (Kontur1[i].Del) continue; //bereits draussen

				if (CountNeighborsRect(Kontur1[i].xPos, Kontur1[i].zeile, 5, 100) > 15) continue;

				if (CountNeighborsRect(Kontur1[i].xPos, Kontur1[i].zeile, 5, 300) > 25) continue;

				if (CountNeighborsRect(Kontur1[i].xPos, Kontur1[i].zeile, 5, 40)  < 10) tmp[i] = true;
			}

			for (int k = 0; k<ANZAHL_KONTUR; k++)
			{
				//printf("ipNSF:  killkont %d\n", k);
				Kontur1[k].Del = tmp[k];
			}
		}


		// Zaehlt zu einem Konturpunkt die Anzahl der Nachbarn, die in einem Rechteck-Bereich nicht weiter als maxDiff entfernt sind.
		// Fuer jeden konturpunkt wird geschaut, wie viele der anderen
		//  - innerhalb einer gewissen Distanz maxDiff1 in xRichtung UND
		//  - innerhalb einer gewissen Distanz maxDiff2 in yRichtung sind
		// co1 = xPos Grauwertmin
		// co2 = Zeile des Mins
		int NoSeamFind::CountNeighborsRect(int co1, int co2, int maxDiff1, int maxDiff2)
		{
			int anzahl = 0;
			int diff1, diff2;
			//double diff;
			for (int i = 0; i<iKonturAnzahl; i++)
			{
				diff1 = absInt(co1 - Kontur1[i].xPos);
				diff2 = absInt(co2 - Kontur1[i].zeile);
				//diff = sqrt((double)(diffX*diffX+diffY*diffY));
				if (diff1>maxDiff1) continue;
				if (diff2>maxDiff2) continue;
				anzahl++;
			}
			return anzahl;
		}


		void NoSeamFind::SearchForChains2()
			// Neue Hauptfunktion fuer die Suche nach zusammenhaengenden Ketten.
		{
			const int  maxDiff1 = 20,
				maxDiff2 = 80;
			int        last1 = -10,
				last2 = -10;
			//double     diff;
			int        diff1, diff2;
			const int  maxbreak = 5;  // <-------- Parameter ?
			int        breakcounter = 0;
			bool       neueKette = true;
			int        aktstart = 0;
			int        aktlaenge = 0;
			int        zaehler;

			//LK[0].laenge = LK[1].laenge = LK[2].laenge = 0;

			for (zaehler = 0; zaehler<iKonturAnzahl; zaehler++)
			{
				//if (zaehler == iKonturAnzahl-1) breakcounter = maxbreak+1;
				if (!neueKette && (zaehler == iKonturAnzahl - 1))
				{
					if (4 * aktlaenge >= 40)
						AddNeueKette(aktstart, aktlaenge);

					break;
				}

				if (neueKette) // neue Kette suchen
				{
					if (Kontur1[zaehler].Del) // aktuelles schon geloescht
					{
						continue;
					}
					else //aktueller Punkt vorhanden
					{
						aktstart = zaehler;
						neueKette = false;
						breakcounter = 0;
						aktlaenge = 1;
						last1 = Kontur1[zaehler].xPos;
						last2 = Kontur1[zaehler].zeile;
					}
				}
				else // keine neue Kette suchen
				{
					if (Kontur1[zaehler].Del)
					{
						breakcounter++;

						if (breakcounter > maxbreak) // Ende der aktuellen Kette
						{
							if (4 * aktlaenge >= 40)
								AddNeueKette(aktstart, aktlaenge);

							neueKette = true;
						}
					}
					else // akt. Punkt nicht geloescht
					{
						diff1 = absInt(last1 - Kontur1[zaehler].xPos);
						diff2 = absInt(last2 - Kontur1[zaehler].zeile);
						//last1 = iKontur1[zaehler];
						//last2 = iKontur2[zaehler];

						if ((diff1 > maxDiff1) || (diff2 > maxDiff2))
						{
							breakcounter++;
							breakcounter++;
							Kontur1[zaehler].Del = true;
							if (breakcounter > maxbreak) // Ende der aktuellen Kette
							{
								if (4 * aktlaenge >= 40)
									AddNeueKette(aktstart, aktlaenge);

								neueKette = true;
							}
						}
						else
						{
							last1 = Kontur1[zaehler].xPos;
							last2 = Kontur1[zaehler].zeile;

							aktlaenge += breakcounter;
							breakcounter = 0;
							aktlaenge++;
						}
					}  // else // akt. Punkt nicht geloescht
				}  // else // keine neue Kette suchen
			}  // for (zaehler=0; zaehler<iKonturAnzahl; zaehler++)
		} // SearchforChains2





		// Aus s2insp.cpp aufgerufen: Initialisiert alles und setzt eine gespeicherte Kontur zurueck
		void NoSeamFind::Reset()
		{
			Kontur1.clear();
			Kontur2.clear();
			Kontur3.clear();

			SNoSeamKontur s;
			for (int i = 0; i < ANZAHL_KONTUR; i++)
			{
				Kontur1.push_back(s);
				Kontur2.push_back(s);
				Kontur3.push_back(s);
			}

			InitKontur();

			// Variablen fuer Kettensuche
			for (int i = 0; i<3; i++)
			{
				LK[i].start = 0;
				LK[i].laenge = 0;
				LK[i].m = 0;
				LK[i].b = 0;
				LK[i].mGW = 0;
				LK[i].mX = 0;
				LK[i].x1 = 0;
				LK[i].y1 = 0;
				LK[i].x2 = 0;
				LK[i].y2 = 0;
			}

			x1old_ = -2;
			x2old_ = -2;
			y1old_ = -2;
			y2old_ = -2;
		}


		// Packt die Parameter aus dem shared-Memory in eine Member-Struktur
		void NoSeamFind::SetParams(
			int                   iSeamPos,
			int                   iSeamWidth,
			int                   iGuete,
			//ImageInfo             imageInfo,
			int                   StartY,
			int                   EndY,
			int                   direction,
			int                   iKonkav,
			int                   iKonvex,
			int                   iCalcMode,
			int                   iSeamQuality
			)
		{
			MinY_ = 0;
			MaxY_ = 999;
			iCalcMode_ = iCalcMode;   // Wird in s2insp.cpp gesetzt
			//SeamPar = seamPar;
			//roiGreyDim_ = roiGrey;
			NoSeamPara.SeamWidth = iSeamWidth;
			NoSeamPara.Konkavitaet = iKonkav;
			NoSeamPara.Konvexitaet = iKonvex;
			NoSeamPara.SeamQuality = iSeamQuality;
		}

		// Weisst der Member-Variablen den Bildpointer zu.
		// Wird von s2insp.cpp aufgerufen
		void NoSeamFind::SetBildPointer(unsigned char * pBild)
		{
			//vermutlich ueberfluessig
			//m_pBild = pBild;
		}

		void NoSeamFind::InitKontur()
		{
			for (int i = 0; i < ANZAHL_KONTUR; i++)
			{
				Kontur1[i].xPos = Kontur1[i].zeile = Kontur1[i].GW = Kontur1[i].DispCol = 0;
				Kontur1[i].Del = true;
			}

			iKonturAnzahl = 0;
			bKonturXY = true;
		}

		void NoSeamFind::AddNeueKette(int start, int laenge)
			// Neue Kette gefunden. Ist sie laenger als die bisherigen?
		{
			//printf("To Add: start=%d, laenge=%d\n", start, laenge);
			if (laenge > LK[0].laenge)
			{
				LK[2] = LK[1];
				LK[1] = LK[0];
				LK[0].laenge = laenge;
				LK[0].start = start;
				return;
			}

			if (laenge > LK[1].laenge)
			{
				LK[2] = LK[1];
				LK[1].laenge = laenge;
				LK[1].start = start;
				return;
			}

			if (laenge > LK[2].laenge)
			{
				LK[2].laenge = laenge;
				LK[2].start = start;
			}
		}


		void NoSeamFind::CalcChainsMB()
			// MB steht fuer m&b in y = m * x + b
			// Berechnet zu einer Kette diese beiden Werte
		{
			GeradenRegression geradenRegression;
			geradenRegression.reset();

			int end, begin;

			for (int j = 0; j < ANZAHL_KETTEN; j++)
			{
				if (LK[j].laenge > 0)
				{
					begin = LK[j].start;
					end = begin + LK[j].laenge - 1;

					for (int i = begin; i <= end; i++)
					{
						if (!Kontur1[i].Del)
							// Minimum i               Zeile = Y         xPos
							geradenRegression.addPoint(Kontur1[i].zeile, Kontur1[i].xPos);
					}
					geradenRegression.calcGerade(LK[j].m, LK[j].b);
					geradenRegression.reset();
				}
			}
		}


		void NoSeamFind::DeleteAllButChains()
			// Setzt bei allen Konturpunkten das Del-Flag, wenn sie nicht in einer Kette liegen.
		{
			int zaehler;
			int Anz[ANZAHL_KETTEN];
			int iCurChain;

			for (int i = 0; i < ANZAHL_KETTEN; i++)
			{
				LK[i].mGW = 0;
				LK[i].mX = 0;
				LK[i].minX = 9999;
				LK[i].maxX = 0;
				Anz[i] = 0;
			}

			for (zaehler = 0; zaehler < iKonturAnzahl; zaehler++)
			{
				iCurChain = -1;

				// Gehoert der Kontur-Punkt 'zaehler' zu einer Kette?
				for (int i = 0; i < ANZAHL_KETTEN; i++)
				{
					if ((zaehler >= LK[i].start)
						&& (zaehler < (LK[i].start + LK[i].laenge))
						)
					{
						iCurChain = i;
					}
				}

				// Wenn iCurChain = 0, 1 oder 2  =>  Kontur[zaehler] gehoert
				// zu einer der Ketten
				if (iCurChain + 1) // Punkt in irgendeiner Kette
				{
					if (Kontur1[zaehler].Del) // Punkt schon geloescht = Luecke
					{
						// Mit Dummy-Punkt (x-Pos) aus der Geraden-Berechnung fuellen
						Kontur1[zaehler].xPos = (int)(LK[iCurChain].m * Kontur1[zaehler].zeile + LK[iCurChain].b + 0.5); // Punkt begradigen
						Kontur1[zaehler].Del = false; // Punkt wieder reinnehmen
						// Jetzt noch die Kanten-Punkte re-aktivieren = Mittelwert
						// aus Punkt oberhalb und Punkt unterhalb (diese beiden muessen existie-
						// ren!)
						Kontur1[zaehler].XL = (Kontur1[zaehler - 1].XL + Kontur1[zaehler + 1].XL) / 2;
						Kontur1[zaehler].XR = (Kontur1[zaehler - 1].XR + Kontur1[zaehler + 1].XR) / 2;
					}
					else // Punkt ist vorhanden
					{
						// Wenn Grauwert des Min's nicht zu gross
						// => in Intensitaets-Mittelwert-Summe uebernehmen
						if (Kontur1[zaehler].GW < 190)
						{
							LK[iCurChain].mGW += Kontur1[zaehler].GW;
							Anz[iCurChain]++;
						}
					}

					// x-Pos des Kontur-Punktes
					LK[iCurChain].mX += Kontur1[zaehler].xPos;
					// minX / maxX  der Kette speichern
					if (LK[iCurChain].minX > Kontur1[zaehler].xPos)
						LK[iCurChain].minX = Kontur1[zaehler].xPos;
					if (LK[iCurChain].maxX < Kontur1[zaehler].xPos)
						LK[iCurChain].maxX = Kontur1[zaehler].xPos;
					//printf("DeleteAllButChains:%d minX: %d maxX: %d Kontour1: %d\n", zaehler, LK[iCurChain].minX ,LK[iCurChain].maxX, iKontur1[zaehler]);

					continue;
				}
				else  // Kontur-Punkt gehoert zu keiner Kette
				{
					Kontur1[zaehler].Del = true;
				}
			} // for (zaehler=0; zaehler<iKonturAnzahl; zaehler++)

			for (int i = 0; i < ANZAHL_KETTEN; i++)
			{
				if (Anz[i])
					LK[i].mGW /= Anz[i];
				else
					LK[i].mGW = 200;

				if (LK[i].laenge)
					LK[i].mX /= LK[i].laenge;
				else
					LK[i].mX = 0;
			}
		}


		void NoSeamFind::CalcBoxes(bool draw)
			// Berechnet fuer die Ketten die umschreibenden Rechtecke.
			// Falls draw==true wird's auch gleich gemalt.
		{
			// Max. 3 Ketten
			for (int i = 0; i<ANZAHL_KETTEN; i++)
			{
				if (LK[i].laenge == 0)
				{
					LK[i].x1 = LK[i].y1 = LK[i].x2 = LK[i].y2 = 0;
				}
				else
				{
					LK[i].x1 = (int)(LK[i].mX - 59.5 + 25);
					LK[i].y1 = Kontur1[LK[i].start].zeile;
					LK[i].x2 = (int)(LK[i].mX + 60.5 - 25);
					//LK[i].y2 = Kontur1[ LK[i].start ].zeile + 4*LK[i].laenge-4;		// urspruenglich
					LK[i].y2 = Kontur1[LK[i].start].zeile + 10 * LK[i].laenge - 10;  //neu

					// Abgestellt! Es werden spaeter allenfalls gruene Boxen gezeichnet
					//			if (draw)
					//				drawRectangle ( LK[i].x1, LK[i].y1,
					//								LK[i].x2, LK[i].y2,
					//								INT_DBG_COLOR_CYAN
					//							  );
				}
			}
		}




		int NoSeamFind::GetNextOben(int ort)
			// Sucht in der Kontur den naechsten Punkt nach oben
		{
			if (ort == 0) return -1; // keiner drueber
			for (int i = ort - 1; i >= 0; i--)
			{
				if (!Kontur1[i].Del) return i;
			}
			return -1;
		}

		int NoSeamFind::GetNextUnten(int ort)
			// Sucht in der Konur den naechsten Punkt nach unten
		{
			if (ort >= iKonturAnzahl - 1) return -1; // keiner mehr drunter
			for (int i = ort + 1; i<iKonturAnzahl; i++)
			{
				if (!Kontur1[i].Del) return i;
			}
			return -1;
		}


		void NoSeamFind::ChainInfo()
			// Debug-Ausgabe. Gibt die Infos zu gefundenen Ketten aus.
		{
			for (int i = 0; i<3; i++)
				printf("%d. Kette -> Start %d, Laenge %d, mittl. GW %d\n", i + 1, LK[i].start, LK[i].laenge, (int)LK[i].mGW);
		}


		int NoSeamFind::GetRealInKontur()
			// Zaehlt wieviele Punkte wirklich in der Kontur stecken (Koennten schon einige geloescht sein).
		{
			int zaehler = 0;
			for (int i = 0; i<ANZAHL_KONTUR; i++)
				if (!Kontur1[i].Del) zaehler++;
			return zaehler;
		}

		double NoSeamFind::CalcSurfaceY(int x1, int y1, int x2, int y2)
			// Berechnet in dem angegebenen Bereich die Oberflaeche in Y
		{
			const int maxgrad = 50;
			double diff;
			double surfy = 0;
			int zeile, spalte;

			for (spalte = x1; spalte<x2; spalte++)
			{
				for (zeile = y1; zeile<y2; zeile++)
				{
					//diff = std::abs(m_pBild[zeile*m_AnzX + spalte] - m_pBild[(zeile + 1)*m_AnzX + spalte]);
					diff = std::abs(_image[zeile][spalte] - _image[zeile + 1][spalte]);
					//if (diff<0) diff=-diff;
					if (diff>maxgrad) diff = 0;
					surfy += diff;
				}
			}
			return surfy / ((y2 - y1 + 1)*(x2 - x1));
		}

		double NoSeamFind::GetSurfaceY(int lk)
			// Berechnet zu einer Kette die Oberflaeche in Y. ACHTUNG: Bereich muss angepasst werden.
		{
			double tmpsurf = 0;
			lk -= 1;
			for (int i = 0; i<3; i++)
			{
				if ((lk == i) && (LK[i].laenge))
					tmpsurf = CalcSurfaceY((int)LK[i].mX - 30, Kontur1[LK[i].start].zeile, (int)LK[i].mX + 30, Kontur1[LK[i].start + LK[i].laenge - 1].zeile);
			}

			return tmpsurf;
		}


		/*
		void CNoSeamFind::CheckBox(int nr)
		{
		nr-=1;
		if (LK[nr].laenge)
		{
		LK[nr].isOK=CheckBox( (int)LK[nr].mX-30, iKontur2[LK[nr].start], (int)LK[nr].mX+30, iKontur2[LK[nr].start+LK[nr].laenge-1]	);
		}
		}
		*/

		//int NoSeamFind::SumCandidates()
		//	// derzeit nicht in Benutzung
		//{
		//	if (m_MaxIndex == 0)    return 0;
		//	if (m_MaxIndex == 1024) return 0;
		//	return (AnzahlArray[m_MaxIndex] + AnzahlArray[m_MaxIndex - 1] + AnzahlArray[m_MaxIndex + 1]);
		//	//return (AnzahlArray[m_MaxIndex]);
		//}

		long NoSeamFind::getSum(int x1, int y1, int x2, int y2)
		{
			long sum = 0;
			int index = 0;
			int zeile, spalte;
			int diff = (x2 - x1 + 1);
			if ((x1old_ == x1 - 1) && (x2old_ == x2 - 1) && (y1old_ == y1) && (y2old_ == y2))
			{ // Kasten eins weiter => Sonderfall! Spalte links raus, Spalte rechts rein...
				spalte = x1old_;
				sum = oldSum_;
				for (zeile = y1; zeile <= y2; zeile++)
				{
					index = zeile * _width + spalte;
					//sum -= m_pBild[index]; //links raus
					//sum += m_pBild[index + diff]; //rechts rein
					//sum -= m_pBild[index]; //links raus
					//sum += m_pBild[index + diff]; //rechts rein
					sum -= _image[zeile][spalte]; //links raus
					sum += _image[zeile][spalte + diff]; //rechts rein
				}
			}
			else
			{ // Summe komplett neu berechnen
				for (zeile = y1; zeile <= y2; zeile++)
				{
					index = zeile * _width + x1;
					for (spalte = x1; spalte <= x2; spalte++)
					{
						sum += _image[zeile][spalte];
						index++;
					}
				}
			}

			x1old_ = x1;
			x2old_ = x2;
			y1old_ = y1;
			y2old_ = y2;
			oldSum_ = sum;
			return sum;
		}




		PoorPenetrationOverlay NoSeamFind::getOverlay()
		{
			return _overlay;
		}

		std::vector<PoorPenetrationCandidate> NoSeamFind::getCandidates()
		{
			std::vector<PoorPenetrationCandidate> returnVector;
			returnVector.reserve(ANZAHL_PARAMETER_GRUPPEN);

			bool hasOne = false;

			for (int i = 0; i < ANZAHL_PARAMETER_GRUPPEN; i++)
			{
				SNoSeamResult result = NoSeamResult[i];

				if (result.bFilled)
				{
					hasOne = true;
					PoorPenetrationCandidate candidate;
					candidate.xmin = result.box_x1;
					candidate.ymin = result.box_y1;
					candidate.xmax = result.box_x2;
					candidate.ymax = result.box_y2;
					candidate.m_oLength = result.box_y2 - result.box_y1;
					candidate.m_oWidth = result.width;
					candidate.m_oGradient = (int)(0.5+result.Gradient);
					candidate.m_oGreyvalGap = (int)(0.5+result.greyvalAreaInEdge);
					candidate.m_oGreyvalInner = (int)(0.5+result.greyvalAreaIn);
					candidate.m_oGreyvalOuter = (int)(0.5 + result.greyvalAreaOut);
					candidate.m_oStandardDeviation = (int)(0.5+10*(result.StdAbwBreite + result.StdAbwLinks + result.StdAbwRechts));
					candidate.m_oDevelopedLengthLeft = (int)(0.5 + result.LauflaengeLinks*1000);
					candidate.m_oDevelopedLengthRight = (int)(0.5 + result.LauflaengeRechts*1000);

					returnVector.push_back(candidate);
				}
			} // for ueber alle candidates

			if (!hasOne) // keiner da? Dummy rein
			{
				PoorPenetrationCandidate candidate;
				returnVector.push_back(candidate);
			}

			return returnVector;
		}

		void NoSeamFind::clamp(int & var, int lower, int upper)
		{
			if (var<lower) var = lower;
			if (var>upper) var = upper;
		}

		void NoSeamFind::change(int & var1, int & var2)
		{
			int tmp = var1;
			var1 = var2;
			var2 = tmp;
		}


		double NoSeamFind::GetMeanGreyVal(int x1, int y1, int x2, int y2)
		{
			//clamp (x1, 5, m_AnzX-5);
			//clamp (y1, 5, m_AnzY-5);
			//clamp (x2, 5, m_AnzX-5);
			//clamp (y2, 5, m_AnzY-5);

			//if (x1>x2) change(x1, x2);
			//if (y1>y2) change(y1, y2);

			double sum = 0;
			int index;

			for (int zeile = y1; zeile <= y2; zeile++)
			{
				index = zeile*_width + x1;
				for (int spalte = x1; spalte <= x2; spalte++)
				{
					//sum += m_pBild[index];
					sum += _image[zeile][spalte];
					index++;
				}
			}
			return (sum / ((y2 - y1 + 1) * (x2 - x1 + 1)));
		}

		int NoSeamFind::CalcGreyVals(int x1, int y1, int x2, int y2, int left, int right,
			double & ValIn, double & ValOut)
			// Berechnet die mittl. Grauwerte innerhalb und ausserhalb einer
			// evtl. fehlenden Durchschweissung
		{
			ValIn = ValOut = 128;

			clamp(x1, 5, _width - 5);
			clamp(y1, 5, _height - 5);
			clamp(x2, 5, _width - 5);
			clamp(y2, 5, _height - 5);

			if (x1>x2) change(x1, x2);
			if (y1>y2) change(y1, y2);

			if (left>right) change(left, right);

			if (left<x1) return -1;
			if (left>x2) return -2;
			if (right<x1) return -3;
			if (right>x2) return -4;

			//links+rechts neben der FDS
			ValOut = (GetMeanGreyVal(x1, y1, left - 4, y2) +
				GetMeanGreyVal(right + 4, y1, x2, y2)) / 2;
			//FDS
			if (right - left > 2)	ValIn = GetMeanGreyVal(left + 1, y1, right - 1, y2);
			else              ValIn = 128;

			return 0;
		}

		int NoSeamFind::getErrorPosition(int chainNo)
		{

			int Pos = (NoSeamResult[chainNo].box_x1 + NoSeamResult[chainNo].box_x2) / 2;

			return Pos;
		}


		int NoSeamFind::absInt(int i)
		{
			return (i > 0) ? i : -i;
		}



		PoorPenetrationOverlay::PoorPenetrationOverlay()
		{
			reset();
		}

		void PoorPenetrationOverlay::reset()
		{
			_pointContainer.clear();
			_lineContainer.clear();
			_rectangleContainer.clear();
		}

		const std::vector<PoorPenetrationPoint> & PoorPenetrationOverlay::getPointContainer() const
		{
			return _pointContainer;
		}

		const std::vector<PoorPenetrationLine> & PoorPenetrationOverlay::getLineContainer() const
		{
			return _lineContainer;
		}

		const std::vector<PoorPenetrationRectangle> & PoorPenetrationOverlay::getRectangleContainer() const
		{
			return _rectangleContainer;
		}

		void PoorPenetrationOverlay::addPoint(int x, int y, Color color)
		{
			_pointContainer.push_back(PoorPenetrationPoint(x, y, color));
		}

		void PoorPenetrationOverlay::addLine(int x1, int y1, int x2, int y2, Color color)
		{
			_lineContainer.push_back(PoorPenetrationLine(x1, y1, x2, y2, color));
		}

		void PoorPenetrationOverlay::addRectangle(int x, int y, int width, int height, Color color)
		{
			_rectangleContainer.push_back(PoorPenetrationRectangle(x, y, width, height, color));
		}

		//int PoorPenetrationOverlay::getPointCount()
		//{
		//	return _overlayPointContainer.size();
		//}

		//int PoorPenetrationOverlay::getLineCount()
		//{
		//	return _overlayLineContainer.size();
		//}

		//int PoorPenetrationOverlay::getRectangleCount()
		//{
		//	return _overlayRectangleContainer.size();
		//}

		PoorPenetrationPoint::PoorPenetrationPoint()
		{
			x = 0;
			y = 0;
			color = Color::Black();
		}

		PoorPenetrationPoint::PoorPenetrationPoint(int x, int y, Color color)
		{
			this->x = x;
			this->y = y;
			this->color = color;
		}

		PoorPenetrationLine::PoorPenetrationLine()
		{
			x1 = 0;
			y1 = 0;
			x2 = 0;
			y2 = 0;
			color = Color::Black();
		}

		PoorPenetrationLine::PoorPenetrationLine(int x1, int y1, int x2, int y2, Color color)
		{
			this->x1 = x1;
			this->y1 = y1;
			this->x2 = x2;
			this->y2 = y2;
			this->color = color;
		}

		PoorPenetrationRectangle::PoorPenetrationRectangle()
		{
			x = 0;
			y = 0;
			width = 0;
			height = 0;
			color = Color::Black();
		}

		PoorPenetrationRectangle::PoorPenetrationRectangle(int x, int y, int width, int height, Color color)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
			this->color = color;
		}

		SNoSeamKontur::SNoSeamKontur()
		{
			XL=0;
			XR = 0;
			xPos = 0;
			zeile = 0;
			GW = 0;
			Del = true;
			DispCol = 0;

			KetteNr = 0;
			KetteGruppeVor = 0;
			KetteGruppeNach = 0;
		}


	} // namespace precitec
} // namespace filter
