/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter calculates the displacement of two blanks at the beginning and at the end.
*/

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "edgeSkew.h"

#include <fliplib/TypeToDataTypeImpl.h>

#define INT_STRIPE_OFFSET 10

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {

		const std::string EdgeSkew::m_oFilterName = std::string("EdgeSkew");
		const std::string EdgeSkew::PIPENAME_DISPLACEMENT_OUT = std::string("Displacement");

		EdgeSkew::EdgeSkew() :
			TransformFilter(EdgeSkew::m_oFilterName, Poco::UUID{"dffb7b9a-19ff-4f07-87fb-11d832f25974"}),
			m_pPipeInImageFrame(NULL),
			m_pPipeInStartEndInfo(NULL),

			m_oWidth(100),
			m_oResolution(4)
		{
			m_pPipeOutDisplacement = new SynchronePipe< interface::GeoDoublearray >(this, EdgeSkew::PIPENAME_DISPLACEMENT_OUT);

			// Set default values of the parameters of the filter
			parameters_.add("Width", Parameter::TYPE_int, m_oWidth);
			parameters_.add("Resolution", Parameter::TYPE_int, m_oResolution);

            setInPipeConnectors({{Poco::UUID("6503e140-1a97-498c-9a3d-2b1376f1ec7e"), m_pPipeInImageFrame, "ImageFrame", 1, "Image"},
            {Poco::UUID("726e6686-cf69-4c55-b357-ba86ba9bd530"), m_pPipeInStartEndInfo, "StartEndInfo", 1, "StartEndInfo"}});
            setOutPipeConnectors({{Poco::UUID("0133e84c-c409-440c-8a30-901d6ce29628"), m_pPipeOutDisplacement, "Displacement", 0, ""}});
            setVariantID(Poco::UUID("4984b32a-ce77-4cbd-8537-6b3697ec49b7"));
		}

		EdgeSkew::~EdgeSkew()
		{
			delete m_pPipeOutDisplacement;
		}

		void EdgeSkew::setParameter()
		{
			TransformFilter::setParameter();
			m_oWidth = parameters_.getParameter("Width").convert<int>();
			m_oResolution = parameters_.getParameter("Resolution").convert<int>();
		} // setParameter

		bool EdgeSkew::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(ImageFrame))
				m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

			if (p_rPipe.type() == typeid(GeoStartEndInfoarray))
				m_pPipeInStartEndInfo = dynamic_cast< fliplib::SynchronePipe < GeoStartEndInfoarray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void EdgeSkew::paint()
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

				// Kanten einzeichnen
				if (meanLeft_ != 0)
				{
					rLayerContour.add(new OverlayLine(rTrafo(Point(0, meanLeft_ - 1)), rTrafo(Point(m_oWidth, meanLeft_ - 1)), Color::Blue()));
					rLayerContour.add(new OverlayLine(rTrafo(Point(0, meanLeft_)), rTrafo(Point(m_oWidth, meanLeft_)), Color::Blue()));
					rLayerContour.add(new OverlayLine(rTrafo(Point(0, meanLeft_ + 1)), rTrafo(Point(m_oWidth, meanLeft_ + 1)), Color::Blue()));
				}
				if (meanRight_ != 0)
				{
					rLayerContour.add(new OverlayLine(rTrafo(Point(imageWidth - m_oWidth - 1, meanRight_ - 1)), rTrafo(Point(imageWidth - 1, meanRight_ - 1)), Color::Blue()));
					rLayerContour.add(new OverlayLine(rTrafo(Point(imageWidth - m_oWidth - 1, meanRight_)), rTrafo(Point(imageWidth - 1, meanRight_)), Color::Blue()));
					rLayerContour.add(new OverlayLine(rTrafo(Point(imageWidth - m_oWidth - 1, meanRight_ + 1)), rTrafo(Point(imageWidth - 1, meanRight_ + 1)), Color::Blue()));
				}

				if (m_oVerbosity < eMedium) return;

				if ((meanLeft_ != 0) && (meanRight_ != 0))
				{
					int xPos = imageWidth / 2;
					rLayerContour.add(new OverlayLine(rTrafo(Point(xPos, meanLeft_)), rTrafo(Point(xPos, meanRight_)), Color::Blue()));
					rLayerContour.add(new OverlayLine(rTrafo(Point(xPos - 5, meanLeft_)), rTrafo(Point(xPos + 5, meanLeft_)), Color::Blue()));
					rLayerContour.add(new OverlayLine(rTrafo(Point(xPos - 5, meanRight_)), rTrafo(Point(xPos + 5, meanRight_)), Color::Blue()));

				}

				// Kantenpunkte einzeichnen
				for (int i = 0; i < (int)pointsLeft.size(); i++) if (pointsLeft[i].y > 0) rLayerContour.add(new OverlayPoint(rTrafo(Point(pointsLeft[i].x, pointsLeft[i].y)), Color::Red()));
				for (int i = 0; i < (int)pointsRight.size(); i++) if (pointsRight[i].y > 0) rLayerContour.add(new OverlayPoint(rTrafo(Point(pointsRight[i].x, pointsRight[i].y)), Color::Red()));
			}
			catch (...)
			{
				return;
			}
		} // paint


		void EdgeSkew::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
			poco_assert_dbg(m_pPipeInStartEndInfo != nullptr); // to be asserted by graph editor

			pointsLeft.clear();
			pointsRight.clear();

			meanLeft_ = meanRight_ = 0;

			geo2d::Doublearray oOutDisplacement;

			bool calcEdge = true;

			// Read out image frame from pipe
			const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
			const BImage &rImageIn = rFrameIn.data();

			imageWidth = rImageIn.width();
			imageHeight = rImageIn.height();

			m_image = rFrameIn.data();

			const interface::GeoStartEndInfoarray &rGeoStartEndInfoArrayIn = m_pPipeInStartEndInfo->read(m_oCounter);
			unsigned int oSizeOfArray = rGeoStartEndInfoArrayIn.ref().size();
			geo2d::StartEndInfo oInInfo;
			if (oSizeOfArray <= 0)
			{
				calcEdge = false;
				m_threshBackground = m_threshMaterial = 0;
			}
			else
			{
				oInInfo = std::get<eData>(rGeoStartEndInfoArrayIn.ref()[0]);
				m_threshMaterial = oInInfo.threshMaterial;
				if ((m_threshMaterial<0) || (m_threshMaterial>255)) m_threshMaterial = 40;
				m_threshBackground = oInInfo.threshBackground;
				if ((m_threshBackground<0) || (m_threshBackground>255)) m_threshBackground = 40;

				// calcEdge = oInInfo.isBottomDark || oInInfo.isTopDark;
				calcEdge = (oInInfo.isBottomDark && !oInInfo.isTopDark) || (!oInInfo.isBottomDark && oInInfo.isTopDark); // XOR
			}

			try
			{
				m_oSpTrafo = rFrameIn.context().trafo();

				// Read-out laserline
				//const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read();
				m_oSpTrafo = rFrameIn.context().trafo();
				// And extract byte-array
				//const VecDoublearray& rLaserarray = rLaserLineIn.ref();
				// input validity check

				/*
				if (inputIsInvalid(rFrameIn))
				{
				oOutDouble1.getData().push_back(0);
				oOutDouble1.getRank().push_back(0);
				oOutDouble2.getData().push_back(0);
				oOutDouble2.getRank().push_back(0);
				oOutDouble3.getData().push_back(0);
				oOutDouble3.getRank().push_back(0);

				const GeoVecDoublearray &rGeoLaserLine = GeoVecDoublearray( rFrameIn.context(), m_oLaserLineOut, rFrameIn.analysisResult(), interface::NotPresent );
				const GeoDoublearray &rDouble1 = GeoDoublearray(rFrameIn.context(), oOutDouble1, rLaserLineIn.analysisResult(), interface::NotPresent);
				const GeoDoublearray &rDouble2 = GeoDoublearray(rFrameIn.context(), oOutDouble2, rLaserLineIn.analysisResult(), interface::NotPresent);
				const GeoDoublearray &rDouble3 = GeoDoublearray(rFrameIn.context(), oOutDouble3, rLaserLineIn.analysisResult(), interface::NotPresent);

				logTiming();

				m_pPipeOutImageFrame->signal(rFrameIn);
				m_pPipeOutLaserLine->signal(rGeoLaserLine);
				m_pPipeOutDisplacement->signal(rDouble1);
				m_pPipeOutDouble2->signal(rDouble2);
				m_pPipeOutDouble3->signal(rDouble3);

				return; // RETURN
				}
				*/

				m_hasPainting = true;
				int stripeY = oInInfo.borderBgStripeY;

				if (calcEdge) // ueberhaupt etwas tun?
				{
					if (oInInfo.isTopDark)
					{
						stripeY -= INT_STRIPE_OFFSET;
						if (stripeY < 0) stripeY = 0;
						pointsLeft = calcEdgeMeanTopLeft(stripeY, imageHeight - 1, m_oWidth, m_oResolution);
						meanLeft_ = getMean(pointsLeft);
						pointsRight = calcEdgeMeanTopRight(stripeY, imageHeight - 1, m_oWidth, m_oResolution);
						meanRight_ = getMean(pointsRight);

						oOutDisplacement.getData().push_back(meanLeft_ - meanRight_);
						oOutDisplacement.getRank().push_back(255);
					}
					else
					{
						if (oInInfo.isBottomDark)
						{
							stripeY += INT_STRIPE_OFFSET;
							if (stripeY > imageHeight - 1) stripeY = imageHeight - 1;
							pointsLeft = calcEdgeMeanBottomLeft(0, stripeY, m_oWidth, m_oResolution);
							meanLeft_ = getMean(pointsLeft);
							pointsRight = calcEdgeMeanBottomRight(0, stripeY, m_oWidth, m_oResolution);
							meanRight_ = getMean(pointsRight);

							oOutDisplacement.getData().push_back(meanLeft_ - meanRight_);
							oOutDisplacement.getRank().push_back(255);
						}
						else // komischer Fall
						{
							m_hasPainting = false;
							oOutDisplacement.getData().push_back(0);
							oOutDisplacement.getRank().push_back(0);
						}
					}

				}
				else // nichts tun, kein Kantenbild
				{
					m_hasPainting = false;
					oOutDisplacement.getData().push_back(0);
					oOutDisplacement.getRank().push_back(128);
				}

				////////

				const GeoDoublearray &rDoubleOut = GeoDoublearray(rFrameIn.context(), oOutDisplacement, rFrameIn.analysisResult(), filter::eRankMax);

				preSignalAction();

				m_pPipeOutDisplacement->signal(rDoubleOut);

			}
			catch (...)
			{
				oOutDisplacement.getData().push_back(0);
				oOutDisplacement.getRank().push_back(0);

				const GeoDoublearray &rDisplacement = GeoDoublearray(rFrameIn.context(), oOutDisplacement, rFrameIn.analysisResult(), interface::NotPresent);

				preSignalAction();

				m_pPipeOutDisplacement->signal(rDisplacement);

				return;
			}
		} // proceedGroup

		std::vector<DisplacementPoint> EdgeSkew::calcEdgeMeanTopLeft(int minY, int maxY, int width, int resolution)
		{
			//bool first = true;
			std::vector<DisplacementPoint> edgePoints;

			if (minY < 0) minY = 0;
			if (maxY > imageHeight - 1) maxY = imageHeight - 1;

			for (int x = 0; x < width; x += resolution)
			{
				for (int y = minY; y < maxY - resolution; y++)
				{
					int mean = getMeanInArea(x, y, x + resolution - 1, y + resolution - 1);

					if (mean > m_threshMaterial)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
						break;
					}

					if (y == maxY - resolution - 1)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
					}
				}
			}

			int perCentKill = 15;
			int perCentKillNumber = (int)(0.5 + (perCentKill / 100.0) * edgePoints.size());

			for (int i = 0; i < perCentKillNumber; i++)
			{
				killSmallest(edgePoints);
				killBiggest(edgePoints);
			}

			return edgePoints;
		}

		std::vector<DisplacementPoint> EdgeSkew::calcEdgeMeanTopRight(int minY, int maxY, int width, int resolution)
		{
			//bool first = true;
			std::vector<DisplacementPoint> edgePoints;

			for (int x = imageWidth - (width+1); x < imageWidth; x += resolution)
			{
				for (int y = minY; y < maxY - resolution; y++)
				{
					int mean = getMeanInArea(x, y, x + resolution - 1, y + resolution - 1);

					if (mean > m_threshMaterial)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
						break;
					}

					if (y == maxY - resolution - 1)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
					}
				}
			}

			int perCentKill = 15;
			int perCentKillNumber = (int)(0.5 + (perCentKill / 100.0) * edgePoints.size());

			for (int i = 0; i < perCentKillNumber; i++)
			{
				killSmallest(edgePoints);
				killBiggest(edgePoints);
			}

			return edgePoints;
		}

		std::vector<DisplacementPoint> EdgeSkew::calcEdgeMeanBottomLeft(int minY, int maxY, int width, int resolution)
		{
			//bool first = true;
			std::vector<DisplacementPoint> edgePoints;

			for (int x = 0; x < width; x += resolution)
			{
				for (int y = maxY; y > minY + resolution; y--)
				{
					int mean = getMeanInArea(x, y - resolution - 1, x + resolution - 1, y);

					if (mean > m_threshMaterial)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
						break;
					}

					if (y == minY + resolution + 1)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
					}
				}
			}

			int perCentKill = 15;
			int perCentKillNumber = (int)(0.5 + (perCentKill / 100.0) * edgePoints.size());

			for (int i = 0; i < perCentKillNumber; i++)
			{
				killSmallest(edgePoints);
				killBiggest(edgePoints);
			}

			return edgePoints;
		}

		std::vector<DisplacementPoint> EdgeSkew::calcEdgeMeanBottomRight(int minY, int maxY, int width, int resolution)
		{
			//bool first = true;
			std::vector<DisplacementPoint> edgePoints;

			for (int x = imageWidth - (width + 1); x < imageWidth; x += resolution)
			{
				for (int y = maxY; y > minY + resolution; y--)
				{
					int mean = getMeanInArea(x, y - resolution - 1, x + resolution - 1, y);

					if (mean > m_threshMaterial)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
						break;
					}

					if (y == minY + resolution + 1)
					{
						edgePoints.push_back(DisplacementPoint(x, y));
					}
				}
			}

			int perCentKill = 15;
			int perCentKillNumber = (int)(0.5 + (perCentKill / 100.0) * edgePoints.size());

			for (int i = 0; i < perCentKillNumber; i++)
			{
				killSmallest(edgePoints);
				killBiggest(edgePoints);
			}

			return edgePoints;
		}

		int EdgeSkew::getMeanInArea(int x1, int y1, int x2, int y2)
		{
			double sum = 0;
			for (int x = x1; x <= x2; x++)
			{
				for (int y = y1; y <= y2; y++)
				{
					sum += m_image[y][x];
				}
			}
			int count = (x2 - x1 + 1)*(y2 - y1 + 1);
			return (int)(0.5 + sum / count);
		}

		void EdgeSkew::killSmallest(std::vector<DisplacementPoint> & points)
		{
			int size = points.size();

			if (size <= 0) return;
			if (size == 1)
			{
				points[0].y = -1;
				return;
			}

			int indexMin = 0;
			int tempMin = 100000;

			for (int i = 0; i < size; i++)
			{
				if (points[i].y == -1) continue;
				if (points[i].y < tempMin)
				{
					indexMin = i;
					tempMin = points[i].y;
				}
			}

			points[indexMin].y = -1;
		}

		void EdgeSkew::killBiggest(std::vector<DisplacementPoint> & points)
		{
			int size = points.size();

			if (size <= 0) return;
			if (size == 1)
			{
				points[0].y = -1;
				return;
			}

			int indexMax = 0;
			int tempMax = -10;

			for (int i = 0; i < size; i++)
			{
				if (points[i].y == -1) continue;
				if (points[i].y > tempMax)
				{
					indexMax = i;
					tempMax = points[i].y;
				}
			}

			points[indexMax].y = -1;
		}

		int EdgeSkew::getMean(std::vector<DisplacementPoint> & points)
		{
			int count = 0;
			int size = points.size();
			double sum = 0;

			for (int i = 0; i < size; i++)
			{
				if (points[i].y != -1)
				{
					count++;
					sum += points[i].y;
				}
			}

			if (count>0)
			{
				return (int)(0.5 + sum / count);
			}
			return 0;
		}

		DisplacementPoint::DisplacementPoint(int inX, int inY)
		{
			x = inX;
			y = inY;
		}

		//////////////////////////////////////////////
		/*********************************************
		CLASS ProjXStripe
		*********************************************/
		//////////////////////////////////////////////

		//ProjXStripe::ProjXStripe()
		//{
		//  reset();
		//}
		//
		//ProjXStripe::~ProjXStripe()
		//{
		//}
		//
		//void ProjXStripe::reset()
		//{
		//	for (int i=0; i<1000; i++) iValue_[i] = 0;
		//
		//	iStart_ = 0;
		//	iEnd_   = 999;
		//	x1_ = x2_ = y1_ = y2_ = 0;
		//}

		//void ProjXStripe::draw(unsigned char * pImage, int iResX, int iResY, int c)
		//{
		//	int X, Y, oldX=0, oldY=0;
		//	int color;
		//	if (c==0) color=INT_DBG_COLOR_RED;
		//	else      color=c;
		//	double ysize = y2_ - y1_;
		//
		//	for (X = iStart_; X<=iEnd_; X++)
		//	{
		//		Y = (int) (y2_ - (iValue_[X]/255.0) * ysize);
		//		if (X==iStart_) putPixel(X, Y, color);
		//		else            Line(oldX, oldY, X, Y, color);
		//		oldX=X;
		//		oldY=Y;
		//	}
		//}

		//int ProjXStripe::getMedian(int anz, int * pInt)
		//{
		//	int * itmp;
		//	int tmp;
		//
		//	itmp = new int[anz];
		//
		//	memcpy(itmp, pInt, anz*sizeof(int));
		//
		//	for (int i=1; i<anz; i++)
		//	{
		//		for (int j=0; j<anz-i; j++)
		//		{
		//			if ( itmp[j] < itmp[j+1] )
		//			{
		//				tmp       = itmp[j];
		//				itmp[j]   = itmp[j+1];
		//				itmp[j+1] = tmp;
		//			}
		//		}
		//	}
		//
		//	int erg = itmp[1+anz/2];
		//
		//	delete [] itmp;
		//
		//	return erg;
		//}
		//
		//int ProjXStripe::getMedian(std::vector<int> intVec)
		//{
		//	int tmp;
		//	int total = intVec.size();
		//
		//	if (total<=0) return 0;
		//	if (total==1) return intVec[0];
		//
		//	for (int i=1; i<total; i++)
		//	{
		//		for (int j=0; j<total-i; j++)
		//		{
		//			if ( intVec[j] < intVec[j+1] )
		//			{
		//				tmp         = intVec[j];
		//				intVec[j]   = intVec[j+1];
		//				intVec[j+1] = tmp;
		//			}
		//		}
		//	}
		//	int erg = intVec[1+total/2];
		//	return erg;
		//}
		//
		//void ProjXStripe::copyVal2Tmp()
		//{
		//	_values = _tempValues;
		//	memcpy(iValTmp_, iValue_, 1000*sizeof(int));
		//}
		//
		//void ProjXStripe::copyTmp2Val()
		//{
		//	_tempValues = _values;
		//	memcpy(iValue_, iValTmp_, 1000*sizeof(int));
		//}
		//
		//void ProjXStripe::smoothValues(int sizeMedian)
		//{
		//	int LiReWeg = sizeMedian / 2;
		//
		//	copyVal2Tmp();
		//
		//	for (int i = 0; i<LiReWeg; i++)
		//	{
		//		//Enden beschneiden
		//		iValTmp_[iStart_+i] = 0;
		//		iValTmp_[iEnd_-i]    = 0;
		//	}
		//
		//	iStart_ = iStart_ + LiReWeg;
		//	iEnd_   = iEnd_   - LiReWeg;
		//
		//	for (int i=iStart_; i<=iEnd_; i++)
		//	{
		//		iValTmp_[i] = getMedian(sizeMedian, &iValue_[i-LiReWeg]);
		//	}
		//
		//	copyTmp2Val();
		//}
		//
		//int ProjXStripe::getMin()
		//{
		//	int tempMin = iValue_[iStart_];
		//
		//	for (int i=iStart_+1; i<=iEnd_; i++)
		//	{
		//	  if (iValue_[i]<tempMin) tempMin=iValue_[i];
		//	}
		//
		//	return tempMin;
		//}
		//
		//int ProjXStripe::getMax()
		//{
		//	int tempMax = iValue_[iStart_];
		//
		//	for (int i=iStart_+1; i<=iEnd_; i++)
		//	{
		//	  if (iValue_[i]>tempMax) tempMax=iValue_[i];
		//	}
		//
		//	return tempMax;
		//}
		//
		//int ProjXStripe::countChanges()
		//{
		//	int Min = getMin();
		//	int Max = getMax();
		//	int Counter = 0;
		//	int Middle = (Min+Max) / 2;
		//	int ThreshDown = Middle-5;
		//	int ThreshUp   = Middle+5;
		//	bool goesDown = true;
		//
		//	for (int i=iStart_; i<=iEnd_; i++)
		//	{
		//		if (goesDown)
		//		{
		//			if (iValue_[i]<ThreshDown) //Grenze passiert
		//			{
		//				Counter++;
		//				goesDown = false;
		//			}
		//		}
		//		else
		//		{
		//			// goesUP
		//			if (iValue_[i]>ThreshUp) //Grenze passiert
		//			{
		//				Counter++;
		//				goesDown = true;
		//			}
		//		}
		//	}
		//
		//	return Counter;
		//}

		//////////////////////////////////////////////
		/*********************************************
		CLASS ProjAnalyser
		*********************************************/
		//////////////////////////////////////////////

		//ProjAnalyser::ProjAnalyser(precitec::image::BImage image)
		//{
		//  _image = image;
		//  iResX_ = image.width();
		//  iResY_ = image.height();
		//  isBad_ = false;
		//}
		//
		//ProjAnalyser::~ProjAnalyser()
		//{
		//}
		//
		//int ProjAnalyser::getBoxMean(int x1, int y1, int x2, int y2)
		//{
		//	int zeile, spalte;
		//	long sum=0;
		//	int total = (x2-x1+1)*(y2-y1+1);
		//
		//	if (total<=0) return 0;
		//
		//	for (zeile=y1; zeile<=y2; zeile++)
		//	{
		//		for (spalte=x1; spalte<=x2; spalte++)
		//		{
		//			sum += _image[zeile][spalte];
		//		}
		//	}
		//	return ( (int)(sum/total) );
		//}
		//
		//void ProjAnalyser::start()
		//{
		//	//printf("ProjAnalyser::start\n");
		//
		//	isBad_ = false;
		//	int boxNr;
		//	int x1=0, x2=0, y1=0, y2=0;
		//	int LiRe = 1;
		//	//int Gesamt = 2*LiRe +1;
		//	int spalte;
		//	int Mittel;
		//
		//	TotalStripe.iStart_ = x1;
		//	TotalStripe.iEnd_   = x2;
		//  //printf("start\n");
		//  //return;
		//
		//	for (boxNr = 0; boxNr<10; boxNr++)
		//	{
		//  		Stripe[boxNr].y1_ = y1 = 250 + boxNr*50;
		//  		Stripe[boxNr].y2_ = y2 = y1  + 49;
		//  		Stripe[boxNr].x1_ = x1 = 50  + LiRe;
		//  		Stripe[boxNr].x2_ = x2 = 750 - LiRe;
		//  		Stripe[boxNr].iStart_ = x1;
		//  		Stripe[boxNr].iEnd_   = x2;
		//
		//  		for (spalte = x1; spalte <= x2; spalte++)
		//  		{
		//			Mittel = getBoxMean(spalte-LiRe, y1, spalte+LiRe, y2);
		//			Stripe[boxNr].iValue_[spalte] = Mittel;
		//			TotalStripe.iValue_[spalte] += Mittel;
		//  		}
		//
		//  		Stripe[boxNr].smoothValues(15);
		//  		//if (DEBUG_SESSION) Stripe[boxNr].draw(pImage_, iResX_, iResY_);
		//  		//if (DEBUG_SESSION) printf("Streifen %d: Min=%d, Max=%d, Wechsel=%d\n",
		//  	    //    boxNr+1, Stripe[boxNr].getMin(), Stripe[boxNr].getMax(), Stripe[boxNr].countChanges()-2);
		//	}
		//
		//	for (int i=0; i<1000; i++) TotalStripe.iValue_[i] = TotalStripe.iValue_[i]/10;
		//
		//	TotalStripe.iStart_ = Stripe[0].iStart_;
		//	TotalStripe.iEnd_   = Stripe[0].iEnd_;
		//	TotalStripe.x1_ = 50  + LiRe;
		//	TotalStripe.x2_ = 750 - LiRe;
		//	TotalStripe.y1_ = 250;
		//	TotalStripe.y2_ = 749;
		//
		//	//if (DEBUG_SESSION) TotalStripe.draw(pImage_, iResX_, iResY_, INT_DBG_COLOR_GREEN);
		//
		//	int MinIndex = TotalStripe.iStart_;
		//	int MinGW = TotalStripe.iValue_[MinIndex];
		//
		//	for (int i=TotalStripe.iStart_+1; i<=TotalStripe.iEnd_; i++)
		//	{
		//		if (MinGW>TotalStripe.iValue_[i]) //kleineren gefunden
		//		{
		//			MinIndex = i;
		//			MinGW    = TotalStripe.iValue_[i];
		//		}
		//	}
		//
		//	//if (DEBUG_SESSION)
		//	//{
		//	//  Line(MinIndex,   400, MinIndex,   600, INT_DBG_COLOR_BLUE);
		//	//  Line(MinIndex+1, 400, MinIndex+1, 600, INT_DBG_COLOR_BLUE);
		//	//  Line(MinIndex-1, 400, MinIndex-1, 600, INT_DBG_COLOR_BLUE);
		//	//}
		//
		//	bool linksfix=false;
		//	bool rechtsfix=false;
		//	int GWThresh = (int)(MinGW*1.1);
		//	int indexL = MinIndex;
		//	int indexR = MinIndex;
		//
		//	for (int i=1; i<20; i++)
		//	{
		//		if (!linksfix && (MinIndex-i>=TotalStripe.iStart_)) //nicht links raus wandern!
		//		{
		//			if (TotalStripe.iValue_[MinIndex-i] < GWThresh) //links noch klein
		//			{
		//				indexL = MinIndex-i; //dann links verschieben
		//			}
		//			else // links nicht mehr klein
		//			{
		//				linksfix = true; // dann Suche nach links beenden
		//			}
		//		}
		//
		//		if (!rechtsfix && (MinIndex+i<=TotalStripe.iEnd_)) //nicht rechts raus wandern!
		//		{
		//			if (TotalStripe.iValue_[MinIndex+i] < GWThresh) //rechts noch klein
		//			{
		//				indexR = MinIndex+i; //dann rechts verschieben
		//			}
		//			else // rechts nicht mehr klein
		//			{
		//				rechtsfix = true; // dann Suche nach rechts beenden
		//			}
		//		}
		//	}
		//
		//	//if (DEBUG_SESSION)
		//	//{
		//	//  Line(indexL,    400, indexL,   600, INT_DBG_COLOR_BLUE);
		//	//  Line(indexL+1,  400, indexL,   600, INT_DBG_COLOR_BLUE);
		//	//  Line(indexL-1,  400, indexL,   600, INT_DBG_COLOR_BLUE);
		//
		//	//  Line(indexR,    400, indexR,   600, INT_DBG_COLOR_BLUE);
		//	//  Line(indexR+1,  400, indexR,   600, INT_DBG_COLOR_BLUE);
		//	//  Line(indexR-1,  400, indexR,   600, INT_DBG_COLOR_BLUE);
		//	//}
		//
		//  // jetzt hoch und runter laufen, laserlinie checken
		//  int Max=0;
		//  int Wert;
		//  bool obenOK = true;
		//  bool untenOK = true;
		//  int anzNOK=0;
		//  int anzNOKzsh=0;
		//
		//  //if (DEBUG_SESSION) printf("**oben**\n");
		//
		//  for (int i=indexL-5; i<=indexR+5; i++)
		//  {
		//  	Max=0;
		//
		//  	//zuerst nach oben, Max. suchen
		//  	for (int y=400; y>5; y--)
		//  	{
		//  		Wert = _image[y][i];
		//  		if (Wert>Max) Max=Wert;
		//  	}
		//
		//    //if (DEBUG_SESSION) printf("Spalte %d, Max = %d", i, Max);
		// 	//if (DEBUG_SESSION)
		// 	//{
		// 	//	if (Max>180)
		// 	//		printf(" \n");
		// 	//	else
		// 	//		printf(" *\n");
		// 	//}
		//
		//    if (Max>180) //OK
		//    {
		//    	anzNOKzsh = 0;
		//    }
		//    else //NOK
		//    {
		//    	anzNOKzsh++;
		//    	anzNOK++;
		//    	if (anzNOK   >=5) obenOK=false;
		//    	if (anzNOKzsh>=3) obenOK=false;
		//    }
		//   }
		//
		//  anzNOK=0;
		//  anzNOKzsh=0;
		//
		//  //if (DEBUG_SESSION) printf("**unten**\n");
		//
		//  for (int i=indexL-5; i<=indexR+5; i++)
		//  {
		//  	Max=0;
		//
		//  	for (int y=600; y<995; y++)
		//  	{
		//  		Wert = _image[y][i];
		//  		if (Wert>Max) Max=Wert;
		//  	}
		//
		//  	//if (DEBUG_SESSION) printf("Spalte %d, Max = %d", i, Max);
		//  	//if (DEBUG_SESSION)
		//  	//{
		//  	//	if (Max>180)
		//  	//		printf(" \n");
		//  	//	else
		//  	//		printf(" *\n");
		//  	//}
		//
		//  	if (Max>180) //OK
		//    {
		//    	anzNOKzsh = 0;
		//    }
		//    else //NOK
		//    {
		//    	anzNOKzsh++;
		//    	anzNOK++;
		//    	if (anzNOK   >=5) untenOK=false;
		//    	if (anzNOKzsh>=3) untenOK=false;
		//    }
		//  }
		//
		//	if (obenOK || untenOK)
		//	{
		//	}
		//	else
		//	{
		//	  isBad_ = true;
		//	}
		//
		//	//if (DEBUG_SESSION && isBad_)
		//	//{
		//	//	printf("#   #  ###  #   # ###\n");
		//	//	printf("##  # #   # #  #  ###\n");
		//	//	printf("# # # #   # ###    # \n");
		//	//	printf("#  ## #   # #  #     \n");
		//	//	printf("#   #  ###  #   #  # \n");
		//	//}
		//
		//	//if (DEBUG_SESSION && !isBad_)
		//	//{
		//	//	printf(" ***  *   * ***\n");
		//	//	printf("*   * *  *  ***\n");
		//	//	printf("*   * ***    * \n");
		//	//	printf("*   * *  *     \n");
		//	//	printf(" ***  *   *  * \n");
		//	//}
		//} // start - Hauptroutine
		//
		//bool ProjAnalyser::isBad()
		//{
		//	return isBad_;
		//}

		//////////////////////////////////////////////
		/*********************************************
		CLASS DiagStripe
		*********************************************/
		//////////////////////////////////////////////

		//DiagStripe::DiagStripe()
		//{
		//	mean_ = min_ = max_ = 0;
		//}
		//
		//DiagStripe::~DiagStripe()
		//{
		//}
		//
		//void DiagStripe::setImage(BImage image, bool direction)
		//{
		//  pImage_ = image;
		//  ResX_ = image.width();
		//  ResY_ = image.height();
		//  //pPar_ = pPar;
		//  bDirection_ = direction;
		//}
		//
		//int DiagStripe::getPixel(int x, int y)
		//// gibt den Grauwert an der aktuellen Position zurueck.
		//// sollte (x,y) ausserhalb liegen, gibt's -1 zurueck!
		//{
		//	if (x<0) return -1;
		//	if (y<0) return -1;
		//
		//	if (x>=ResX_) return -1;
		//	if (y>=ResY_) return -1;
		//
		//	return pImage_[y][x];
		//}
		//
		//void DiagStripe::drawLine(int color)
		//{
		//	/*
		//	if (!isValid_) return;
		//	//Endpunkte muessen berechnet werden
		//	int y, startx=0, starty=0, endx=0, endy=0;
		//	for (int x=0; x<ResX_; x++)
		//	{
		//	  // Fktwert berechnen
		//	  y = (int) (Steigung_ * x + AchsAbschnitt_ + 0.5);
		//	  if ( (y>=0) && (y<ResY_) )
		//	  {
		//	  	startx = x;
		//	  	starty = y;
		//	  	break;
		//	  }
		//	}
		//
		//	for (int x=ResX_-1; x>=0; x--)
		//	{
		//    y = (int) (Steigung_ * x + AchsAbschnitt_ + 0.5);
		//	  if ( (y>=0) && (y<ResY_) )
		//	  {
		//	  	endx = x;
		//	  	endy = y;
		//	  	break;
		//	  }
		//	}
		//
		//	Line(startx, starty, endx, endy, color);
		//	*/
		//}
		//
		//int DiagStripe::getLineY(int x)
		//{
		//	int y = (int) (Steigung_ * x + AchsAbschnitt_ + 0.5);
		//	return y;
		//}
		//
		//void DiagStripe::setData(double m, double pointX, double pointY)
		//{
		//	Steigung_ = m;
		//	//y-Achsenabschnitt berechnen
		//	AchsAbschnitt_ = pointY - m*pointX;
		//	checkValid();
		//}
		//
		//void DiagStripe::setData(double m, double n)
		//{
		//	Steigung_ = m;
		//	AchsAbschnitt_ = n;
		//	checkValid();
		//}
		//
		//void DiagStripe::checkValid()
		//{
		//  isValid_ = true;
		//  int j;
		//  for (int i=100; i<=700; i+=100)
		//  {
		//  	j = (int)(Steigung_ * i + AchsAbschnitt_ + 0.5);
		//  	if ( (j<ResY_) && (j>0) )
		//  	{
		//  		//printf("isValid: j = %d, Steigung: %d, AchsAbschnitt %d\n", j, Steigung_, AchsAbschnitt_);
		//  		return;
		//  	}
		//  }
		//  isValid_ = false;
		//}
		//
		//void DiagStripe::calcValues()
		//{
		//	if (!isValid_)
		//		return;
		//
		//	int zeile, spalte, ywert;
		//	const int speederx = 10;
		//	const int speedery = 5;
		//	const int rangey = 20;
		//	int gw, anzahl = 0, sum = 0;
		//	int curMin = 300, curMax = -1;
		//	int SchwelleVorneRohr;
		//	int SchwelleHintenRohr;
		//	int SchwelleVorneBG;
		//	int SchwelleHintenBG;
		//
		//	if (bDirection_)
		//	{
		//		SchwelleVorneRohr  = 10; //pPar_->StartEndDetect.ThreshTubeEnd;
		//		SchwelleHintenRohr = 10; //pPar_->StartEndDetect.ThreshTubeStart;
		//		SchwelleVorneBG    = 10; //pPar_->StartEndDetect.ThreshBackEnd;
		//		SchwelleHintenBG   = 10; //pPar_->StartEndDetect.ThreshBackStart;
		//	}
		//	else
		//	{
		//		SchwelleVorneRohr  = 10; //pPar_->StartEndDetect.ThreshTubeStart;
		//		SchwelleHintenRohr = 10; //pPar_->StartEndDetect.ThreshTubeEnd;
		//		SchwelleVorneBG    = 10; //pPar_->StartEndDetect.ThreshBackStart;
		//		SchwelleHintenBG   = 10; //pPar_->StartEndDetect.ThreshBackEnd;
		//	}
		//
		//	VRohr_ = VBG_ = HRohr_ = HBG_ = 0;
		//
		//	for (spalte = 0 ; spalte < ResX_ ; spalte += speederx)
		//	{
		//		ywert = (int) (Steigung_ * spalte + AchsAbschnitt_ + 0.5);
		//
		//		for (zeile = ywert - rangey ; zeile < ywert + rangey ; zeile += speedery)
		//		{
		//			gw = getPixel(spalte, zeile);
		//
		//			if ( gw != -1)
		//			{
		//				//Pixel liegt im Bild
		//				anzahl++;
		//				sum += gw;
		//
		//				if (gw >  curMax)              curMax = gw;
		//				if (gw <  curMin)              curMin = gw;
		//				if (gw >= SchwelleVorneRohr)   VRohr_++;
		//				if (gw >= SchwelleHintenRohr)  HRohr_++;
		//				if (gw <= SchwelleVorneBG)     VBG_++;
		//				if (gw <= SchwelleHintenBG)    HBG_++;
		//			}
		//		}  // for zeile
		//	} //for spalte
		//
		//	min_  = curMin;
		//	max_  = curMax;
		//	mean_ = anzahl ? sum / anzahl : 1;
		//}
		//
		//void DiagStripe::getValues()
		//{
		//}

		//////////////////////////////////////////////
		/*********************************************
		CLASS SBlockInfo
		*********************************************/
		//////////////////////////////////////////////

		//SBlockInfo::SBlockInfo()
		//{
		//	startX = startY = sizeX = sizeY = 0;
		//	mean = 0;
		//	min = max = 0;
		//}
		//
		//SBlockInfo::~SBlockInfo()
		//{
		//}
		//
		//
		//
		//
		//
		//SoutubeResult1::SoutubeResult1()
		//{
		//  reset();
		//}
		//
		//SoutubeResult2::SoutubeResult2()
		//{
		//  reset();
		//}
		//
		//void SoutubeResult1::reset()
		//{
		//  InspectionRange.start_=100;
		//  InspectionRange.end_=900;
		//  BildStatusOrg_ = 0;
		//  iLastStripeOnTube_=10;
		//  ResultValid_ = false;
		//}
		//
		//void SoutubeResult2::reset()
		//{
		//  iLastStripeOnTube_=10;
		//  ResultValid_ = false;
		//  notchSize_=0;
		//  edgeSkew_=0;
		//}
		//
		//void SoutubeResult2::copySR1(SoutubeResult1 SR1)
		//{
		//	BildStatus_ = SR1.BildStatus_;
		//	iLastStripeOnTube_ = SR1.iLastStripeOnTube_;
		//	InspectionRange = SR1.InspectionRange;
		//}
		//
		////////////////////////////////////////////////
		///*********************************************
		//		CLASS PointContainer
		//*********************************************/
		////////////////////////////////////////////////
		//
		//PointContainer::PointContainer()
		//{
		//	reset();
		//}
		//
		//void PointContainer::reset()
		//{
		//	points.clear();
		//}
		//
		//void PointContainer::addPoint(int x, int y)
		//{
		//	Point point;
		//	point.x = x;
		//	point.y = y;
		//	points.push_back(point);
		//}
		//
		//int PointContainer::getPointX(int nr)
		//{
		//	if (nr<0) return 0;
		//	if (nr>=(int)points.size()) return 0;
		//	return points[nr].x;
		//}
		//
		//int PointContainer::getPointY(int nr)
		//{
		//	if (nr<0) return 0;
		//	if (nr>=(int)points.size()) return 0;
		//	return points[nr].y;
		//}
		//
		//int PointContainer::size()
		//{
		//	return points.size();
		//}

		//////////////////////////////////////////////
		/*********************************************
		CLASS Line
		*********************************************/
		//////////////////////////////////////////////

		//Line::Line(double m, double b)
		//{
		//	m_ = m;
		//	b_ = b;
		//}
		//
		//void Line::draw(int xStart, int yStart, int xEnd, int yEnd, int color)
		//{
		//	UNUSED int y, startx=0, starty=0, endx=0, endy=0;
		//	for (int x=xStart; x<xEnd; x++)
		//	{
		//	  // Fktwert berechnen
		//	  y = (int) (m_ * x + b_ + 0.5);
		//	  if ( (y>=yStart) && (y<yEnd) )
		//	  {
		//	  	startx = x;
		//	  	starty = y;
		//	  	break;
		//	  }
		//	}
		//
		//	for (int x=xEnd; x>=xStart; x--)
		//	{
		//    y = (int) (m_ * x + b_ + 0.5);
		//	  if ( (y>=yStart) && (y<yEnd) )
		//	  {
		//	  	endx = x;
		//	  	endy = y;
		//	  	break;
		//	  }
		//	}
		//
		//	//Line(startx, starty, endx, endy, color);
		//}
		//
		//double Line::getY(double x)
		//{
		//	return (m_ * x + b_);
		//}


	} // namespace precitec
} // namespace filter
