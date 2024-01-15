/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2015
* 	@brief 		This filter looks for the grey values on the tracked laser line. A gap is detected if several values are lower than a given threshold.
*/

#include <algorithm>
// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "lineTrackingQuality.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
using namespace interface;
using namespace image;
using namespace geo2d;
namespace filter {

const std::string LineTrackingQuality::m_oFilterName = std::string("LineTrackingQuality");
const std::string LineTrackingQuality::PIPENAME_GAPWIDTH_OUT = std::string("GapWidthOut");
const std::string LineTrackingQuality::PIPENAME_GAPDETECTED_OUT = std::string("GapDetectedOut");

LineTrackingQuality::LineTrackingQuality() :
	TransformFilter(LineTrackingQuality::m_oFilterName, Poco::UUID{"bf060d27-b711-4a70-b591-91dee2a80448"}),
	m_pPipeInImageFrame(NULL),
	m_pPipeInLaserLine(NULL),
	m_pPipeOutGapWidth(NULL),
	m_pPipeOutGapDetected(NULL),
	m_oMode(0),
	m_oThreshold(20),
	m_oSearchHeight(0),
	m_oMaxGapWidth(10)
{
	m_pPipeOutGapWidth = new SynchronePipe< GeoDoublearray >(this, LineTrackingQuality::PIPENAME_GAPWIDTH_OUT);
	m_pPipeOutGapDetected = new SynchronePipe< GeoDoublearray >(this, LineTrackingQuality::PIPENAME_GAPDETECTED_OUT);

	// Set default values of the parameters of the filter
	parameters_.add("Mode", Parameter::TYPE_int, m_oMode);
	parameters_.add("Threshold", Parameter::TYPE_int, m_oThreshold);
	parameters_.add("SearchHeight", Parameter::TYPE_int, m_oSearchHeight);
	parameters_.add("MaxGapWidth", Parameter::TYPE_int, m_oMaxGapWidth);

    setInPipeConnectors({{Poco::UUID("34d4e3f0-d812-476d-b8d7-f0b3a6768cb4"), m_pPipeInLaserLine, "LaserLineIn", 1, "LaserLine"},
    {Poco::UUID("19e387e9-2f48-4e1e-b95e-c3f175cc7bc9"), m_pPipeInImageFrame, "ImageFrameIn", 1, "ImageFrame"}});
    setOutPipeConnectors({{Poco::UUID("05d53f70-96fd-44d6-8e33-cc90029fd923"), m_pPipeOutGapWidth, PIPENAME_GAPWIDTH_OUT, 0, ""},
    {Poco::UUID("9fa1c5b4-955e-4f7e-b272-328707209cda"), m_pPipeOutGapDetected, PIPENAME_GAPDETECTED_OUT, 0, ""}});
    setVariantID(Poco::UUID("cd2dbc0d-e232-4c13-9698-24fbc3efdf2a"));
}

LineTrackingQuality::~LineTrackingQuality()
{
	delete m_pPipeOutGapWidth;
	delete m_pPipeOutGapDetected;
}

void LineTrackingQuality::setParameter()
{
	TransformFilter::setParameter();
	m_oMode = parameters_.getParameter("Mode").convert<int>();
	m_oThreshold = parameters_.getParameter("Threshold").convert<int>();
	m_oSearchHeight = parameters_.getParameter("SearchHeight").convert<int>();
	m_oMaxGapWidth = parameters_.getParameter("MaxGapWidth").convert<int>();
} // setParameter

bool LineTrackingQuality::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.type() == typeid(GeoVecDoublearray))
		m_pPipeInLaserLine = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if (p_rPipe.type() == typeid(ImageFrame))
		m_pPipeInImageFrame = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe

void LineTrackingQuality::paint()
{
	// bool isInvalid = inputIsInvalid(m_oLineBrightnessOut);

	if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
	{
		return;
	} // if

	const Trafo		&rTrafo(*m_oSpTrafo);
	OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer	&rLayerContour(rCanvas.getLayerContour());


	if (m_oOverlayGapY * m_oOverlayGapStart * m_oOverlayGapEnd != 0)
	{
		rLayerContour.add(new OverlayRectangle(rTrafo(Rect(m_oOverlayGapStart, m_oOverlayGapY - 10, m_oOverlayGapEnd - m_oOverlayGapStart +1, 20)), Color::Red()));
		rLayerContour.add(new OverlayLine(rTrafo(geo2d::Point(m_oOverlayGapStart, m_oOverlayGapY - 25)), rTrafo(geo2d::Point(m_oOverlayGapStart, m_oOverlayGapY + 25)), Color::Red()));
		rLayerContour.add(new OverlayLine(rTrafo(geo2d::Point(m_oOverlayGapEnd, m_oOverlayGapY - 25)), rTrafo(geo2d::Point(m_oOverlayGapEnd, m_oOverlayGapY + 25)), Color::Red()));
	}
} // paint

void LineTrackingQuality::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	m_overlayMin = 1000000;
	m_overlayMax = -1000000;

	geo2d::Doublearray oGapWidthOut;
	geo2d::Doublearray oGapDetectedOut;

	// Read out image frame from pipe
	const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
	m_oSpTrafo = rFrameIn.context().trafo();
	// Extract actual image and size
	const BImage &rImageIn = rFrameIn.data();
	// Read-out laserline
	const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);
	m_oSpTrafo = rLaserLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rLaserarray = rLaserLineIn.ref();
	// input validity check

	if (inputIsInvalid(rLaserLineIn))
	{
		oGapWidthOut.getData().push_back(0);
		oGapWidthOut.getRank().push_back(0);
		oGapDetectedOut.getData().push_back(0);
		oGapDetectedOut.getRank().push_back(0);

		const GeoDoublearray &rGeoWidth = GeoDoublearray(rFrameIn.context(), oGapWidthOut, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rGeoGapDetected = GeoDoublearray(rFrameIn.context(), oGapDetectedOut, rFrameIn.analysisResult(), interface::NotPresent);
		preSignalAction();

		m_pPipeOutGapWidth->signal(rGeoWidth);
		m_pPipeOutGapDetected->signal(rGeoGapDetected);

		return; // RETURN
	}

	// Now do the actual image processing
	calcGapWidth(rImageIn, rLaserarray, oGapWidthOut, oGapDetectedOut);

	// Create a new byte array, and put the global context into the resulting profile
	const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoDoublearray &rGeoWidth = GeoDoublearray(rFrameIn.context(), oGapWidthOut, oAnalysisResult, 1.0);
	const GeoDoublearray &rGeoGapDetected = GeoDoublearray(rFrameIn.context(), oGapDetectedOut, oAnalysisResult, 1.0);
	preSignalAction();
	m_pPipeOutGapWidth->signal(rGeoWidth);
	m_pPipeOutGapDetected->signal(rGeoGapDetected);
} // proceedGroup



int LineTrackingQuality::calcGapWidth(const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn,
	geo2d::Doublearray & widthOut, geo2d::Doublearray & gapDetected)
{
	const unsigned int	oNbLines = p_rLaserLineIn.size();

	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines

		// get the references to the stl vectors

		const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
		const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

		int roiHeight = p_rImageIn.height();

		int oLineY;

		bool onGap = false;

		int longestGapLength = 0;
		int currentGapLength = 0;

		int longestGapStart = 0;
		int longestGapEnd = 0;
		int currentGapStart = 0;
		int currentGapEnd = 0;

		int currentGapStartY = 0;
		int currentGapEndY = 0;

		double currentGapMeanY = 0;
		double longestGapMeanY = 0;
		int countCurrentValidY = 0;
		int countLongestValidY = 0;

		int xStart = 0;
		int xEnd = 0;

		//start and end of laserline are not categorical at the beginning and end of the roi
		const int roiSize = static_cast<int>(rLaserLineIn_Data.size());
		for (int x = 0; x < roiSize; ++x)
		{
			if (rLaserLineIn_Rank[x] > 0)
			{
				xStart = x;
				x = roiSize; //interrupt
			}
		}
		for (int x = roiSize-1; x > 0; --x)
		{
			if (rLaserLineIn_Rank[x] > 0)
			{
				xEnd = x;
				x = 0;
			}
		}


		//Laserlinienunterbrechungen weden als gap betrachtet
		if (m_oMode >0 )
		{
			//check if the laserline has interruptions no tracking data (rank = 0)
			bool gapStart = false;
			for (int x = xStart; x < xEnd-1; x++)
			{
				if ((rLaserLineIn_Rank[x] <= 0) && (gapStart == false))
				{
					currentGapStart = x;
					currentGapStartY = int(rLaserLineIn_Data[x - 1]);
					gapStart = true;
				}
				if (gapStart && (rLaserLineIn_Rank[x] >0))
				{
					currentGapEnd = x;
					currentGapEndY = int(rLaserLineIn_Data[x]);
					gapStart = false;
					currentGapLength = currentGapEnd - currentGapStart;
					if (currentGapLength > longestGapLength)
					{
						longestGapLength = currentGapLength;
						longestGapStart = currentGapStart;
						longestGapEnd = currentGapEnd;
						m_oOverlayGapY = (int)(0.5 * (currentGapStartY + currentGapEndY));
						m_oOverlayGapStart = longestGapStart;
						m_oOverlayGapEnd = longestGapEnd;



						if (m_oMode == 2)
						{
							//check mean greyValue in the gap
							int meanGreyVal = 0;
							int ctr = 0;
							if (currentGapStartY == currentGapEndY)
							{
								int j = currentGapStartY;
								for (int i = currentGapStart; i < currentGapEnd; ++i)
								{
									meanGreyVal += static_cast<int>(p_rImageIn[j][i]);
									ctr++;
								}
							}
							else
							{
								int dYStart = currentGapStartY;
								int dYEnd   = currentGapEndY;
								if (currentGapStartY>currentGapEndY)
								{
									dYStart = currentGapEndY;
									dYEnd = currentGapStartY;
								}
								for (int i = currentGapStart; i < currentGapEnd; ++i)
								{

									for (int j = dYStart; j < dYEnd; ++j)
									{
										meanGreyVal += static_cast<int>(p_rImageIn[j][i]);
										ctr++;
									}
								}
							}
							if (ctr>0)
							{
								meanGreyVal = meanGreyVal / ctr;
							}
							else
							{
								meanGreyVal = m_oThreshold;//no error
							}

							int errorVal = 0;
							if ((meanGreyVal<m_oThreshold) && (longestGapLength>m_oMaxGapWidth))
							{
								errorVal = 1;
							}
							widthOut.getData().push_back(longestGapLength);
							widthOut.getRank().push_back(255);
							gapDetected.getData().push_back(errorVal);
							gapDetected.getRank().push_back(255);

						}
						else
						{
						    widthOut.getData().push_back(longestGapLength);
						    widthOut.getRank().push_back(255);
						    gapDetected.getData().push_back((longestGapLength > m_oMaxGapWidth) ? 1 : 0);
						    gapDetected.getRank().push_back(255);
					    }

					}

					onGap = true;
				}
			}

			if (onGap)
			{

				return(0);
			}


		}//if mode == 1
		else
		{
			for (int x = xStart; x < xEnd; x++)
			{
				if (rLaserLineIn_Rank[x] <= 0)
				{
					longestGapLength = 0;
					longestGapStart = 0;
					longestGapEnd = 0;
					m_oOverlayGapY = 0;
					m_oOverlayGapStart = 0;
					m_oOverlayGapEnd = 0;
					widthOut.getData().push_back(0);
					widthOut.getRank().push_back(255);
					gapDetected.getData().push_back(0);
					gapDetected.getRank().push_back(255);
					return 0;
				}
			}
		}



		for (int x = xStart; x<xEnd; x++)
		{
			if (rLaserLineIn_Rank[x])
				oLineY = int(rLaserLineIn_Data[x]);
			else
				oLineY = 0;

			bool isTooDark;
			unsigned char greyVal;

			if (rLaserLineIn_Rank[x]>0)
			{
				greyVal = p_rImageIn[oLineY][x];
				isTooDark = greyVal < m_oThreshold;
			}
			else
			{
				isTooDark = true;
			}

			if ( isTooDark && (m_oSearchHeight > 0) )
			{
				int yIndex = 0;
				for (int y = oLineY + 1; y <= oLineY + m_oSearchHeight; y++)
				{
					yIndex = std::min(roiHeight-1, y);
					greyVal = p_rImageIn[yIndex][x];
					if (greyVal >= m_oThreshold)
					{
						isTooDark = false;
					}
				}

				for (int y = oLineY - 1; y >= oLineY - m_oSearchHeight; y--)
				{
					yIndex = std::max(1, y);
					greyVal = p_rImageIn[yIndex][x];
					if (greyVal >= m_oThreshold)
					{
						isTooDark = false;
					}
				}
			}

			if (isTooDark) // zu dunkle Stelle gefunden
			{
				// fuer Overlay y sichern
				if (rLaserLineIn_Rank[x] > 0)
				{
					currentGapMeanY += oLineY;
					countCurrentValidY++;
				}

				if (onGap)
				{

				}
				else
				{ // neuen Gap-Start gefunden
					currentGapStart = x;
					onGap = true;
				}
			}
			else // Stelle nicht zu dunkel
			{
				if (onGap)
				{ // aktueller Gap beendet
					currentGapEnd = x - 1;
					currentGapLength = currentGapEnd - currentGapStart + 1;
					// letzten Gap mit groesstem vergleichen
					if (currentGapLength > longestGapLength)
					{
						longestGapLength = currentGapLength;
						longestGapStart = currentGapStart;
						longestGapEnd = currentGapEnd;
						longestGapMeanY = currentGapMeanY;
						countLongestValidY = countCurrentValidY;
					}
					else // Gap war uninteressant, gab bereits groessere, nix machen
					{

					}
					onGap = false;
					currentGapMeanY = 0;
					countCurrentValidY = 0;
				}
				else
				{ // Fall Punkt hell genug und kein aktueller Gap => alles gut, nix zu tun

				}

			}


		}

		// Linie von links nach rechts ist durch => noch schauen, ob wir zuletzt auf Gap waren
		if (onGap)
		{
			currentGapEnd = rLaserLineIn_Data.size() - 1;
			currentGapLength = currentGapEnd - currentGapStart + 1;
			// letzten Gap mit groesstem vergleichen
			if (currentGapLength > longestGapLength)
			{
				longestGapLength = currentGapLength;
				longestGapStart = currentGapStart;
				longestGapEnd = currentGapEnd;
				longestGapMeanY = currentGapMeanY;
				countLongestValidY = countCurrentValidY;
			}
		}

		m_oOverlayGapY = 0;
		if (countLongestValidY != 0)
			m_oOverlayGapY = (int)(0.5 + longestGapMeanY / countLongestValidY);
		m_oOverlayGapStart = longestGapStart;
		m_oOverlayGapEnd = longestGapEnd;

		widthOut.getData().push_back(longestGapLength);
		widthOut.getRank().push_back(255);
		gapDetected.getData().push_back( (longestGapLength > m_oMaxGapWidth) ? 1 : 0);
		gapDetected.getRank().push_back(255);

	} // for
	return 0;
}


} // namespace precitec
} // namespace filter
