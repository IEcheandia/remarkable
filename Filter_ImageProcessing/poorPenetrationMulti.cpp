/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		Js
* 	@date		2017
* 	@brief 		This filter tries to detect a poor penetration.
*/

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <algorithm>
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "poorPenetrationMulti.h"

// Konstante, wie viel Prozent von Ausreissern eliminiert werden
#define __PER_CENT_KILL 10

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string PoorPenetrationMulti::m_oFilterName = std::string("PoorPenetrationMulti");
		const std::string PoorPenetrationMulti::PIPENAME_RESULT_OUT = std::string("ResultOut");


		PoorPenetrationMulti::PoorPenetrationMulti() :
			TransformFilter(PoorPenetrationMulti::m_oFilterName, Poco::UUID{"9926f4e6-b2c2-4035-a79b-f6c42248ca3c"}),
			m_pPipeInImageFrame(NULL),
			m_oMode(0),
			m_oDisplay(0),
			m_oThresholdUpper(0),
	        m_oThresholdLower(255),
			m_oCountOverThreshold(0),
			m_oCountUnderThreshold(0)
		{

			m_pPipeOutResult = new SynchronePipe< interface::GeoPoorPenetrationCandidatearray >(this, PoorPenetrationMulti::PIPENAME_RESULT_OUT);


			// Set default values of the parameters of the filter
			parameters_.add("Mode", Parameter::TYPE_int, m_oMode);
			parameters_.add("Display", Parameter::TYPE_int, m_oDisplay);
			parameters_.add("ThresholdUpper", Parameter::TYPE_int, m_oThresholdUpper);
			parameters_.add("ThresholdLower", Parameter::TYPE_int, m_oThresholdLower);
			parameters_.add("GreyDistance", Parameter::TYPE_int, m_oGreyDistance);
			parameters_.add("CountOverThreshold", Parameter::TYPE_int, m_oCountOverThreshold);
			parameters_.add("CountUnderThreshold", Parameter::TYPE_int, m_oCountUnderThreshold);
			parameters_.add("FilterSizeX", Parameter::TYPE_int, m_oFilterSizeX);
			parameters_.add("FilterSizeY", Parameter::TYPE_int, m_oFilterSizeY);
			parameters_.add("MaxNumberMinima", Parameter::TYPE_int, m_oNumberOfMinima);
			parameters_.add("Resolution", Parameter::TYPE_int, m_oResolution);
			parameters_.add("ColumnDeviation", Parameter::TYPE_int, m_oColumnDeviation);
			parameters_.add("MaxInterruption", Parameter::TYPE_int, m_oMaxInterruption);

            setInPipeConnectors({{Poco::UUID("0886537D-0446-4B12-8166-17E64F900A9B"), m_pPipeInImageFrame, "ImageFrameIn", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("D50B4A3C-97CF-4797-B340-CF051193796C"), m_pPipeOutResult, PIPENAME_RESULT_OUT, 0, ""}});
            setVariantID(Poco::UUID("93d8f815-839a-4bec-bdfb-d6b6c4a9ff25"));
		}

		PoorPenetrationMulti::~PoorPenetrationMulti()
		{

			delete m_pPipeOutResult;

		}

		void PoorPenetrationMulti::setParameter()
		{
			TransformFilter::setParameter();
			m_oMode = parameters_.getParameter("Mode").convert<int>();
			m_oDisplay = parameters_.getParameter("Display").convert<int>();
			m_oThresholdUpper = parameters_.getParameter("ThresholdUpper").convert<int>();
			m_oThresholdLower = parameters_.getParameter("ThresholdLower").convert<int>();
			m_oGreyDistance   = parameters_.getParameter("GreyDistance").convert<int>();
			m_oCountOverThreshold = parameters_.getParameter("CountOverThreshold").convert<int>();
			m_oCountUnderThreshold = parameters_.getParameter("CountUnderThreshold").convert<int>();
			m_oFilterSizeX = parameters_.getParameter("FilterSizeX").convert<int>();
			m_oFilterSizeY = parameters_.getParameter("FilterSizeY").convert<int>();
			m_oNumberOfMinima = parameters_.getParameter("MaxNumberMinima").convert<int>();
			m_oResolution     = parameters_.getParameter("Resolution").convert<int>();
			m_oColumnDeviation = parameters_.getParameter("ColumnDeviation").convert<int>();
			m_oMaxInterruption = parameters_.getParameter("MaxInterruption").convert<int>();


		} // setParameter

		bool PoorPenetrationMulti::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);


			return BaseFilter::subscribe(p_rPipe, p_oGroup);

		} // subscribe

		void PoorPenetrationMulti::paint()
		{
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
			{
				return;
			} // if

			if (!m_hasPainting)
			{
				return;
			}

			try
			{

				const Trafo		&rTrafo(*m_oSpTrafo);
				OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
				OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

				// paint here


				for (unsigned int i = 0; i < m_oDrawPoints.size(); ++i)
				{
					for (int j = 0;j<5;++j)
					rLayerContour.add(new  OverlayPoint(rTrafo(Point(m_oDrawPoints[i].x, m_oDrawPoints[i].y+j)), m_oDrawPoints[i].color));
				}

				rLayerContour.add(new OverlayRectangle(rTrafo(Rect(m_oRectangle.x, m_oRectangle.y, m_oRectangle.width, m_oRectangle.height)), m_oRectangle.color));


			}
			catch (...)
			{
				return;
			}
		} // paint


		void PoorPenetrationMulti::proceed(const void* sender, fliplib::PipeEventArgs& e)
		{

			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

			geo2d::PoorPenetrationCandidatearray oOutCandidate;

			// Read out image frame from pipe
			const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);

			try
			{
				m_oSpTrafo = rFrameIn.context().trafo();
				// Extract actual image and size
				const BImage &rImageIn = rFrameIn.data();

				m_oSpTrafo = rFrameIn.context().trafo();
				//_overlay.setTrafo(*m_oSpTrafo);


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

				m_oDrawPoints.clear();
				m_hasPainting = true;

				FdFind noSeam(rImageIn, m_oFilterSizeX, m_oFilterSizeY, m_oNumberOfMinima, m_oThresholdLower,m_oThresholdUpper,m_oGreyDistance,m_oCountOverThreshold,m_oCountUnderThreshold,m_oResolution,m_oColumnDeviation);

				std::vector<PointGrey> minimaPos;
				std::vector<PointGrey> resultPosition;
				std::vector<PointGrey> gradPosLeft;
				std::vector<PointGrey> gradPosRight;
				std::vector<PointGrey> lastPointVec;


				noSeam.CheckForNoSeam(m_oDisplay, m_oMode, minimaPos, resultPosition);

				if (m_oDisplay == 1)
				{
					for (unsigned int i = 0; i < minimaPos.size(); ++i)
					{
						m_oDrawPoints.push_back(minimaPos[i]);
					}
				}
				else if (m_oDisplay == 2)
				{
					for (unsigned int i = 0; i < resultPosition.size(); ++i)
					{
						m_oDrawPoints.push_back(resultPosition[i]);
					}
				}

				int maxInterrupt = m_oMaxInterruption;
				int maxLength = 0;
				maxLength = noSeam.InterruptPoints(resultPosition, lastPointVec, maxInterrupt);

				if (m_oDisplay == 3)
				{
					for (unsigned int i = 0; i < lastPointVec.size(); ++i)
					{
						m_oDrawPoints.push_back(lastPointVec[i]);
					}
                }


				//rectangle for the  remaining points
				int mean = 0;
				int meanGrey = 0;
				noSeam.CheckForMean(mean,meanGrey, lastPointVec);


				if (lastPointVec.size() > 1)
				{
					m_oRectangle.x      = mean - m_oColumnDeviation - 10;
					m_oRectangle.y      = lastPointVec[0].y;
					m_oRectangle.width = (mean + m_oColumnDeviation + 10) - (mean - m_oColumnDeviation - 10);
					m_oRectangle.height = maxLength;
					m_oRectangle.color = Color::Green();
				}



				int meanGrad = 0;
				int meanWidth = 0;
				noSeam.CheckForGradient(lastPointVec,gradPosLeft,gradPosRight,meanGrad,meanWidth);

				if (m_oDisplay == 4)
				{
					for (unsigned int i = 0; i < gradPosLeft.size(); ++i)
					{
						m_oDrawPoints.push_back(gradPosLeft[i]);
					}
					for (unsigned int i = 0; i < gradPosRight.size(); ++i)
					{
						m_oDrawPoints.push_back(gradPosRight[i]);
					}
			   }

				int     greyValInside = 0;
				int     greyValOutside = 0;
				double  meanDev = 0;
				double	lenL = 0;
				double  lenR =0;

				noSeam.CheckForGradientFeatures(gradPosLeft, gradPosRight, greyValInside, greyValOutside, meanDev,lenL,lenR);



				int meanGapWidth = meanWidth;
				int meanGapLength = maxLength;
				int meanGradient = meanGrad;
				int meanGreyValGap = meanGrey;

				int meanGreyValInner = greyValInside;
				if (meanWidth <= 3) //test
				{
					meanGreyValInner = meanGrey;
				}

				int meanGreyValOutGap = greyValOutside;
				int stdDeviation = static_cast<int>(meanDev);
				int lengthLeft =   static_cast<int>(lenL);
				int lengthRight = static_cast<int>(lenR);





				PoorPenetrationCandidate noSeamCandidate(meanGapWidth, meanGapLength, meanGradient, meanGreyValGap,meanGreyValInner, meanGreyValOutGap,
					stdDeviation, lengthLeft, lengthRight);

				//bounding box in the candidate
				if (lastPointVec.size() > 0)
				{
					noSeamCandidate.xmin = mean - m_oColumnDeviation - 10; //
					noSeamCandidate.ymin = lastPointVec[0].y;
					noSeamCandidate.xmax = mean + m_oColumnDeviation + 10;//
					noSeamCandidate.ymax = lastPointVec[0].y + maxLength;
				}
				else
				{
					noSeamCandidate.xmin = 0;
					noSeamCandidate.ymin = 0;
					noSeamCandidate.xmax = 1;
					noSeamCandidate.ymax = 1; //in case of a division late
				}


			    // to fullfil the outpipe condition - at the moment there is only one noSeamCandidate per image
				// we have no candidate vector
				oOutCandidate.getData().push_back(noSeamCandidate);
				oOutCandidate.getRank().push_back(255);


				const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type

				const GeoPoorPenetrationCandidatearray &rCandidate = GeoPoorPenetrationCandidatearray(rFrameIn.context(), oOutCandidate, oAnalysisResult, filter::eRankMax);

				preSignalAction();


				m_pPipeOutResult->signal(rCandidate);


			}
			catch (...)
			{
				PoorPenetrationCandidate cand;
				oOutCandidate.getData().push_back(cand);
				oOutCandidate.getRank().push_back(0);


				//const GeoVecDoublearray &rGeoLaserLine = GeoVecDoublearray(rFrameIn.context(), m_oLaserLineOut, rFrameIn.analysisResult(), interface::NotPresent);
				const GeoPoorPenetrationCandidatearray &rCandidate = GeoPoorPenetrationCandidatearray(rFrameIn.context(), oOutCandidate, rFrameIn.analysisResult(), interface::NotPresent);

				preSignalAction();


				m_pPipeOutResult->signal(rCandidate);

				return;
			}
		} // proceedGroup




		//----------------------------------------------------------------------------
		// Class:  FdFind
		//----------------------------------------------------------------------------
		FdFind::FdFind()
		{

		}

		FdFind::FdFind(BImage image,int sizeX, int sizeY, int numberOfMinima, int lowerThreshold,int upperThreshold,int greyDistance,int ctrUpper,int ctrUnder,int resolution,int deviation):
			m_oIimage(image),
			m_oSizeX(sizeX),
		    m_oSizeY(sizeY),
			m_oMaxNumberMinima(numberOfMinima),
			m_oLowerThreshold(lowerThreshold),
			m_oUpperThreshold(upperThreshold),
			m_oDistance(greyDistance),
			m_oCtrUpper(ctrUpper),
			m_oCtrUnder(ctrUnder),
			m_oLineResolution(resolution),
			m_oLineDeviation(deviation)
		{

			m_oWidth = image.width();
			m_oHeight = image.height();

		}



		FdFind::~FdFind()
		{
			// Do nothing
		}

		void FdFind::setImage(BImage image)
		{
			m_oIimage = image;
			m_oWidth  = image.width();
			m_oHeight = image.height();
		}

		// search for minima in a grey levelprofile
		// search for the most occuring x position and eliminate points outside
		int FdFind::CheckForNoSeam(int display, int mode, std::vector<PointGrey> &minimapos, std::vector<PointGrey> &resultPosition)
		{

			int y1 = 5;
			int y2 = m_oHeight - 5;
			int x1 = 5;
			int x2 = m_oWidth - 5;
			int xMaxPos=0;
			std::vector<PointGrey> gapPositions;

			// Kontur bestimmen mittels dunkler Stellen
			FindKontur(mode,x1, x2, y1, y2,minimapos); // kostet Zeit (etwa die Haelfte)

			xMaxPos = HistogramSort(minimapos);

			gapPositions = ErasePoints(minimapos,xMaxPos);

			for (unsigned int i = 0; i < gapPositions.size(); ++i)
			{
				resultPosition.push_back(gapPositions[i]);
			}


			return (1);

		}



		int FdFind::FindKontur(int mode, int x1, int x2, int y1, int y2, std::vector<PointGrey> &minimapos)
		{

			int length = 0;

			const int sizeX = m_oSizeX;
			const int sizeY = m_oSizeY;
			std::vector<PointGrey> positionsPerLine;

			for (int zeile = y1; zeile < (y2 - (sizeY - 1)); zeile+=m_oLineResolution)
			{

				//we get n minima per line
				positionsPerLine = ScanLine(zeile, sizeX, sizeY, x1, x2,mode);

				length = positionsPerLine.size();

				//limiting to  m_oMaxNumberMinima
				if (length > m_oMaxNumberMinima)
				{
					positionsPerLine.erase(positionsPerLine.begin() + m_oMaxNumberMinima, positionsPerLine.begin() + length);
				}
				length = positionsPerLine.size();
				for (int i = 0; i < length; ++i)
					minimapos.push_back(positionsPerLine[i]);

				/*
				if (minimapos.size() > m_oMaxNumberMinima)
				{
					minimapos.erase(minimapos.begin() + m_oMaxNumberMinima, minimapos.begin() + length);
				}
				*/
				//length = minimapos.size();

			}


			return(1);
		}


		int FdFind::HistogramSort(std::vector<PointGrey> &minimapos)
		{
			//Histogramcalculation to get the biggest amount of minima per x region:
			// x range inside the x positions has to be
			int xRange = 10;
			int ranges = m_oWidth / xRange + 1;
			int histoVecSize = ranges;

			std::vector<int> histoVec(ranges, 0);

			int val = 0;
			int index = 0;
			for (unsigned int i = 0; i < minimapos.size(); ++i)
			{
				val = minimapos[i].x;
				index = val / xRange;
				histoVec[index]++;
			}

			//histomax
			int maxVal = 0;
			int maxIndex = 0;
			for (int i = 0; i < histoVecSize; ++i)
			{
				if (histoVec[i]>maxVal)
				{
					maxVal = histoVec[i];
					maxIndex = i;
				}
			}
			int xMaxPos = (maxIndex*xRange + xRange / 2);

			return(xMaxPos);
		}


		std::vector<PointGrey> FdFind::ErasePoints(std::vector<PointGrey> &minimapos,int xMaxPos)
		{

			std::vector<PointGrey> resultPositions;
			PointGrey val;
			int oldLinePos = -1; //so the first position can not be at the old pos

			for (unsigned int i = 0; i < minimapos.size(); ++i)
			{
				if ((minimapos[i].x >= xMaxPos - m_oLineDeviation) && (minimapos[i].x <= xMaxPos + m_oLineDeviation))
				{

					val.x = minimapos[i].x;
					val.y = minimapos[i].y;
					val.grey = minimapos[i].grey;
					val.color = Color::Red();
					if (val.y!=oldLinePos)
					    resultPositions.push_back(val);
					oldLinePos = val.y;
				}
			}
			return(resultPositions);
		}

		int FdFind::InterruptPoints(std::vector<PointGrey> &pointVec, std::vector<PointGrey> &lastPointVec, int &maxInterrupt)
		{


			int maxdist = maxInterrupt;// 10; //mu

			int distance = 0;
			int length = 0;
			int startY = 0;
			int endY = 0;
			int labelStart = 0;
			int labelEnd = 0;
			int maxLength = 0;
			bool label = false;

			for (unsigned int i = 1; i < pointVec.size(); ++i)
			{
				distance = pointVec[i].y - pointVec[i - 1].y;

				if ((distance <= maxdist) && !label)
				{
					startY = pointVec[i-1].y;
					label = true;

				}
				else if ((distance > maxdist) && label)
				{
					label = false;
					endY = pointVec[i-1].y;
					length = endY - startY;

					if (length > maxLength)
					{
						maxLength = length;
						labelStart = startY;
						labelEnd = endY;
					}

				}
				else if ( label && (i==pointVec.size() -1) ) // ended at the last point with no or no more more interruption
				{
					endY = pointVec[i].y;
					length = endY - startY;
					if (length > maxLength)
					{
						maxLength = length;
						labelStart = startY;
						labelEnd = endY;
					}
					i = pointVec.size(); // beende loop

				}
			}//for

			if (length == 0)
			{
				//there are no interruptions, take all points
				if (pointVec.size() > 0)
				{
					int lastIndex = pointVec.size() - 1;
					int firstIndex = 0;
					maxLength = pointVec[lastIndex].y - pointVec[firstIndex].y;
					lastPointVec = pointVec;
				}
            }
			else //take only the points inside the biggest "chain"
			{

				if (pointVec.size() > 1)
				{
					for (unsigned int i = 0; i < pointVec.size(); ++i) //??? start
					{

						if ((pointVec[i].y >= labelStart) && (pointVec[i].y <= labelEnd))
							lastPointVec.push_back(pointVec[i]);
					}

				}
				else if (pointVec.size() == 1)
				{
					maxLength = 1;
					lastPointVec = pointVec;

				}
				else
				{
					maxLength = 0;

				}


			}


			return(maxLength);

		}


		int FdFind::CheckForGradient(std::vector<PointGrey> &resultPosition, std::vector<PointGrey> &gradPosLeft, std::vector<PointGrey> &gradPosRight, int &meanGrad, int  &width)
		{

			PointGrey val;
			int line = 0;
			int grad = 0;
			int leftPos = 0;
			int rightPos = 0;


			if (resultPosition.size() > 1)
			{
				for (unsigned int i = 0; i < resultPosition.size(); ++i)
				{
					line = resultPosition[i].y;

					leftPos  = 0;
					rightPos = 0;
					//go left
					for (int x = resultPosition[i].x; x>0; --x)
					{

						x = std::max(0, x);
						if (m_oIimage[line][x] > m_oLowerThreshold)
						{
							val.x = x;
							val.y = line;
							val.grey = m_oIimage[line][x];
							val.color = Color::Blue();
							gradPosLeft.push_back(val);
							leftPos = x;
							grad += (m_oIimage[line][x] - m_oIimage[line][x + 1]);
							//stop
							x = 0;
						}
					}

					//go right
					for (int x = resultPosition[i].x; x<m_oWidth; ++x)
					{
						x = std::min(m_oWidth, x);

						if (m_oIimage[line][x] > m_oLowerThreshold)
						{
							val.x = x;
							val.y = line;
							val.grey = m_oIimage[line][x];
							val.color = Color::Blue();
							gradPosRight.push_back(val);
							rightPos = x;
							grad += (m_oIimage[line][x] - m_oIimage[line][x - 1]);
							//stop
							x = m_oWidth;
						}
					}

					if((leftPos>0) && (rightPos>0))
					{
						width += rightPos - leftPos;
					}


				}//for
				grad = grad / resultPosition.size();
				grad /= 2;
				width /= resultPosition.size();
			}
			else
			{
				grad = 0;
			}
			meanGrad = grad;

            return(0);
		}


		int FdFind::CheckForMean(int &mean,int &meangrey, std::vector<PointGrey> & lastPointVec)
		{

			int sum = 0;
			int grey = 0;
			int k = 0;



			if (lastPointVec.size() > 0)
			{

				for (unsigned int i = 0; i < lastPointVec.size(); ++i)
				{
					sum += lastPointVec[i].x;
					grey += lastPointVec[i].grey;
					k++;
				}
				sum /= k;
				grey /= k;
			}
			else
			{
				sum = 0;
				grey = 0;
			}

			mean = sum;
			meangrey = grey;


			return 0;
		}


		int FdFind::CheckForGradientFeatures(std::vector<PointGrey> &gradPosLeft, std::vector<PointGrey> &gradPosRight, int &greyValInside ,int &greyValOutside, double &meanDev, double &lenL, double &lenR)
		{

			int line = 0;
			int startx = 0;
			int endx = 0;
			int sum = 0;
			int sumges = 0;
			int sumLeft = 0;
			int meanxLeft = 0;
			int sumRight = 0;
			int meanxRight =0;
			//left side of the dege

			//check size
			if ((gradPosLeft.size() < 1) || (gradPosRight.size() < 1))
			{
				greyValInside  = 0;
				greyValOutside = 0;
				meanDev = 0;
				lenL = 0;
				lenR = 0;
				return -1;
			}


			if (gradPosLeft.size() > 1)
			{

				for (const auto &point : gradPosLeft)
				{
					line = point.y;
					startx = point.x;
					meanxLeft += startx;
					endx = startx - 3;
					sumLeft = 0;
					for (int x = startx; x>endx; --x)
					{
						sumLeft += m_oIimage[line][x];
					}
					sumLeft /= 3;
					sumges += sumLeft;
				}


			}
			//right side of the edge
			if (gradPosRight.size() > 1)
			{
				for (unsigned int i = 0; i < gradPosRight.size(); ++i)
				{
					line = gradPosRight[i].y;
					startx = gradPosRight[i].x;
					meanxRight += startx;
					endx = startx + 3;
					sumRight = 0;
					for (int x = startx; x<endx; ++x)
					{
						sumRight += m_oIimage[line][x];
					}
					sumRight /= 3;
					sumges += sumRight;
				}


			}

			greyValOutside = (sumges/2) / gradPosLeft.size(); //dib by zero
			meanxLeft = meanxLeft / gradPosLeft.size();
			meanxRight = meanxRight / gradPosRight.size();

			//inner grey value
			sum = 0;
			//int dummy = 0; // Test *******************************************************
			if ((gradPosLeft.size() == gradPosRight.size()) && (gradPosLeft.size() > 1))
			{
                for (unsigned int i = 0; i < gradPosLeft.size(); ++i)
				{
					//to be shure to be inside of the gap
					startx = gradPosLeft[i].x +2;  //    gradPosLeft[i].x + 1;
					endx =   gradPosRight[i].x-2; //     gradPosRight[i].x - 1;
					line =   gradPosRight[i].y;
					int sumInner = 0;
					if ((endx - startx) > 0)
					{
						for (int x = startx; x < endx; ++x)
						{
							sumInner += m_oIimage[line][x];
						}
						sumInner /= (endx - startx);
					}
					else
					{

						//sumInner = m_oIimage[line][startx+1];
						sumInner = m_oIimage[line][startx];  // better inside the gap
					}
					sum += sumInner;
				}
				sum /= gradPosLeft.size();
				greyValInside = sum;
			}
			else
			{
				greyValInside = 255;
			}



			//swtdabw:
			sum = 0;
			for (unsigned int i = 0; i < gradPosLeft.size(); ++i)
			{
				sum += (gradPosLeft[i].x - meanxLeft)*(gradPosLeft[i].x - meanxLeft);
			}
			double stdLeft = std::sqrt( static_cast<double>(sum) / static_cast<double>(gradPosLeft.size()));

			sum = 0;
			for (unsigned int i = 0; i < gradPosRight.size(); ++i)
			{
				sum += (gradPosRight[i].x - meanxRight)*(gradPosRight[i].x - meanxRight);
			}
			double stdRight = std::sqrt(static_cast<double>(sum) / static_cast<double>(gradPosRight.size()));

			meanDev = (stdLeft + stdRight) / 2.0;


			//Curvelength
			for (unsigned int i = 1; i < gradPosLeft.size();++i)
			{

				lenL += std::sqrt(((gradPosLeft[i].x - gradPosLeft[i - 1].x) * (gradPosLeft[i].x - gradPosLeft[i - 1].x)
					+ (gradPosLeft[i].y - gradPosLeft[i - 1].y) * (gradPosLeft[i].y - gradPosLeft[i - 1].y)));

			}


			for (unsigned int i = 1; i < gradPosRight.size(); ++i)
			{
				lenR +=std::sqrt(  ((gradPosRight[i].x - gradPosRight[i - 1].x) * (gradPosRight[i].x - gradPosRight[i - 1].x)
					+ (gradPosRight[i].y - gradPosRight[i - 1].y) * (gradPosRight[i].y - gradPosRight[i - 1].y)));

			}


			return 0;
		}


		/**********************************************************************
		* Description:  Scannt eine Linie im Bild durch und sucht bis zu     *
		*               n Minima.                                            *
		*                                                                    *
		* Parameter:    line:         Y-Position des Streifens               *
		*               startx:       X-Position (absolut) Streifenanfang    *
		*               endx:         X-Position (absolut) Streifenende      *
		*               sizeX:        x Filtersize                           *
		*               sizeY:        y Filtersize                           *
		*               minimapos     Positionen der Minima                  *
		*               bDisplayStreifen:  Den Streifen graphisch anzeigen   *
		*                                                                    *
		* Returns:                                                           *
		**********************************************************************/

		std::vector<PointGrey> FdFind::ScanLine(int line, int sizeX, int sizeY, int startx, int endx,int mode)
		{

			std::vector<int> greyValue(m_oWidth,0);
			int summe = 0;
			int MinIndex = 0;
			std::vector<PointGrey> positionsInLine;
			int filtersize = sizeX * sizeY;
			// initial minimum
			int iMinScan = filtersize * 255;
			// initial maximum
			int iMaxScan = 0;

			if (filtersize <= 0)
			{
				filtersize = 1;
			}

			int lowThresh = filtersize*m_oLowerThreshold;
			int upperThresh = filtersize*m_oUpperThreshold;



			//check star values
			if (startx > endx)
			{
				int tmp;
				tmp = startx;
				startx = endx;
				endx = tmp;
			}

			if (startx < 0)
			{
				startx = 0;
			}
			if (endx > m_oWidth)
			{
				endx = m_oWidth;
			}
			if (line < 0)
			{
				line = 0;
			}
			if (line > m_oHeight - 10)
			{
				line = m_oHeight - 10;
			}


			int xStartIndex = 1 + startx + sizeX / 2;
			int xEndIndex = endx - sizeX / 2;

			int greyValSum = 0;
			int col =0;
			for (col = xStartIndex; col < xEndIndex; col++)
			{

				summe = 0;
				for (int m = 0; m < sizeY; ++m)
				{
					for (int n = col - sizeX / 2; n <= col + sizeX / 2; ++n)
					{
						summe += m_oIimage[line + m][n];
					}
				}
				greyValue[col] = summe;
				//greyValue.push_back(summe);
				greyValSum = summe;

				// Kleinste Intensitaet suchen
				if (greyValSum < iMinScan)
				{
					iMinScan = greyValSum;
					MinIndex = col;
				}
				// Groesste Intensitaet suchen
				if (greyValSum > iMaxScan)
				{
					iMaxScan = greyValSum;
				}
			} //for col


			int candidate = 0;
			int dummyxpos = 0;
			int dummyval = 0;
			PointGrey val;
			int greyLevelDistance = m_oDistance * filtersize;
			// contrast thresholds
			int ctrUpper = 0; // m_oCtrUpper;
			int ctrUnder = 0; // m_oCtrUnder;


			int conditionThresh = 0;
			if (mode == 1)
			{
				conditionThresh = 0;
			}
			else if (mode == 2)
			{
				conditionThresh = lowThresh;
			}


			// for computing the minima the algo compares values in the profile with are 2 pixels distance
			// and a grey level difference of "greyLevelDistance"

			int min    = 10000000; //Test
			int anzmin = 0;
			for (col = xStartIndex; col < xEndIndex; col++)
			{

                if ( (greyValue[col] < (greyValue[col - 2] - greyLevelDistance)) &&  (greyValue[col]<lowThresh) )
				{
					//mimima candidate:
					candidate = 1;
				//	dummyxpos = col; Test  -- Aenderung nimm die position der dunkelsten Stelle
					dummyval = greyValue[col];

					//Test ************************
					if (dummyval < min)
					{
						min = dummyval;
						//nimm hier die tiefste stelle
						dummyxpos = col; //Test
					}
					//*******************************


				}
				if (candidate)
				{

					if ((greyValue[col] > (dummyval + greyLevelDistance)) && (greyValue[col]>= conditionThresh))
					{
						//val.x = dummyxpos + (col - dummyxpos) / 2;
						//neu: Test
						val.x = dummyxpos;

						//val.grey = greyValue[val.x];
						val.grey = min / filtersize; // greyValue[val.x]; / filtersize
						min = 10000000;

						val.y = line;
						val.color = Color::Yellow();
						positionsInLine.push_back(val);
						candidate = 0;
						anzmin++;

					}
				}

			}  // for col

			// if there was no position under the lower threshold - take the deepest geylevel to get one value  -- mode switch ?
			if (anzmin == 0)
			{
				val.x = MinIndex;
				val.y = line;
				val.grey = iMinScan / filtersize;
				val.color = Color::Yellow();
				positionsInLine.push_back(val);

			}


			if (mode == 3) // check if there are enough point around a minim psoition under teh lower an over the upper threshold
			{
				std::vector<PointGrey> contrastPoints;
				int xpos = 0;
				bool leftUpperOK = false;
				bool rightUpperOK = false;
				bool highEdge = false;
				int dummyGrey = 0;


                for (unsigned int i = 0; i<positionsInLine.size(); ++i)
				{
					highEdge = false;
					ctrUnder = 0;
					ctrUpper = 0;
					xpos = positionsInLine[i].x;

					//left direction
					for (int l = xpos; l>4; --l)
					{

						if ((greyValue[l] < lowThresh) && !highEdge)
						{
							ctrUnder++;
						}
						dummyGrey = (greyValue[l] + greyValue[l - 1] + greyValue[l - 2] + greyValue[l - 3]) / 4;
						if (dummyGrey >= upperThresh)
						{
							ctrUpper++;
							highEdge = true;
						}

						if (ctrUpper > m_oCtrUpper)
						{
							l = 0;
							leftUpperOK = true;
						}
						if (ctrUpper > 1 && dummyGrey < upperThresh)
						{
							l = 0;
							leftUpperOK = false;
						}

					}

					//right direction
					ctrUpper = 0;
					highEdge = false;
					for (int l = xpos; l < m_oWidth - 4; ++l)
					{
						if ((greyValue[l] < lowThresh) && !highEdge)
						{
							ctrUnder++;
						}
						dummyGrey = (greyValue[l] + greyValue[l + 1] + greyValue[l + 2] + greyValue[l + 3]) / 4;
						if (dummyGrey >= upperThresh)
						{
							ctrUpper++;
							highEdge = true;
						}
						if (ctrUpper > m_oCtrUpper)
						{
							l = m_oWidth;
							rightUpperOK = true;
						}
						if (ctrUpper > 1 && dummyGrey < upperThresh)
						{
							l = m_oWidth;
							rightUpperOK = false;
						}
					}

					if (leftUpperOK && rightUpperOK && (ctrUnder > m_oCtrUnder))
					{
						//point has high contrast
						contrastPoints.push_back(positionsInLine[i]);

					}

				}

				std::sort(contrastPoints.begin(), contrastPoints.end(), [](const PointGrey& lhs, const PointGrey& rhs){return lhs.grey < rhs.grey; });
				return(contrastPoints);

			}//mode = 2



			//fast sorting
			//std::sort(minimapos.begin(), minimapos.end(), [](const PointGrey& lhs, const PointGrey& rhs){return lhs.grey < rhs.grey; });
			std::sort(positionsInLine.begin(), positionsInLine.end(), [](const PointGrey& lhs, const PointGrey& rhs){return lhs.grey < rhs.grey; });


			return(positionsInLine);
		}//ScanLine

	} // namespace precitec
} // namespace filter

