/**
 * 	@file       detectCalibrationLayers.cpp
 * 	@copyright  Precitec Vision GmbH & Co. KG
 * 	@author     Andreas Beschorner (BA)
 * 	@date       2013
 * 	@brief      Tries to detect two calibration workpiece main layers by simple summed pixel intensity per line decision
 */

#include "detectCalibrationLayers.h"

#include "image/image.h"
#include "overlay/overlayPrimitive.h"
#include "geo/array.h"
#include "module/moduleLogger.h"

#include <map>
#include <vector>

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

	const std::string DetectCalibrationLayers::m_oFilterName    = std::string("DetectCalibrationLayers");
	const std::string DetectCalibrationLayers::m_oPipeNameRois  = std::string("RoiLayers");

	const int DetectCalibrationLayers::m_oMinDist = 5; /// the value is chosen rather arbitrary, might become a parameter

	DetectCalibrationLayers::DetectCalibrationLayers() : TransformFilter( DetectCalibrationLayers::m_oFilterName, Poco::UUID{"9636638E-C08F-41F6-897F-E92030E90A93"} ),
		m_pPipeInImageFrame( nullptr ), m_oPipeOutRois( this, DetectCalibrationLayers::m_oPipeNameRois ),
		m_oExtent(20), m_oExtentFactor(0.20), m_oParameterThreshold(30), m_oPaint(false), m_oWidth(0)
	{
		parameters_.add( "Extent", fliplib::Parameter::TYPE_int, m_oExtent);
		parameters_.add( "Threshold", fliplib::Parameter::TYPE_int, m_oParameterThreshold);

        m_oActualThreshold = m_oParameterThreshold;
		resetMaximae();

        setInPipeConnectors({{Poco::UUID("58408F95-DCD0-42A5-93CB-10FA75B61DEE"), m_pPipeInImageFrame, "Image", 0, ""}});
        setOutPipeConnectors({{Poco::UUID("D5845CA8-B692-4810-8667-241706D999E6"), &m_oPipeOutRois, "RoiLayers", 0, ""}});
        setVariantID(Poco::UUID{"83096CD1-1685-42FE-9E59-3223AEE9B5FC"});
	}

	DetectCalibrationLayers::~DetectCalibrationLayers()
	{
	}

	void DetectCalibrationLayers::resetMaximae()
	{
		m_oMaximae.oIdx[0] = -1; m_oMaximae.oValue[0] = -1;
		m_oMaximae.oIdx[1] = -1; m_oMaximae.oValue[1] = -1;
		m_oRoiYTop[0] = -1; m_oRoiYBottom[0] = -1;
		m_oRoiYTop[1] = -1; m_oRoiYBottom[1] = -1;
	}

	void DetectCalibrationLayers::setParameter()
	{
		TransformFilter::setParameter();
		m_oExtentFactor = parameters_.getParameter("Extent").convert<int>()/100.0;
		m_oParameterThreshold = parameters_.getParameter("Threshold").convert<int>();
		m_oActualThreshold = m_oParameterThreshold;
	}

	bool DetectCalibrationLayers::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
	{
		m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame >* >(&p_rPipe);
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	}

	void DetectCalibrationLayers::paint()
	{
		if (m_oVerbosity < eLow || !m_oPaint )
		{
			return;
		}

		const Trafo		&rTrafo		( *m_oSpTrafo );
		OverlayCanvas	&rCanvas	( canvas<OverlayCanvas>(m_oCounter) );
		OverlayLayer	&rLayer		( rCanvas.getLayerLine() ); // layer 2: paint over roi

		rLayer.add( new OverlayRectangle( rTrafo(Rect(1, m_oRoiYTop[0], m_oWidth-1, m_oRoiYBottom[0]-m_oRoiYTop[0])), Color::Red() ) );
		rLayer.add( new OverlayRectangle( rTrafo(Rect(1, m_oRoiYTop[1], m_oWidth-1, m_oRoiYBottom[1]-m_oRoiYTop[1])), Color::Green() ) );
	}


	// --------------------------------------------------------------------------------------------------------

	unsigned int DetectCalibrationLayers::guessThreshold(std::map<int, int> &p_rBins)
	{
		std::map<int, int>::reverse_iterator oIt = p_rBins.rbegin();
		std::map<int, int>::reverse_iterator oFinal = p_rBins.rbegin();

		while ( (oIt != p_rBins.rend()) && (oIt->first >= (0.8*m_oExtentFactor*oFinal->first) ) )
		{
			++oIt;
		}
		if (oIt != p_rBins.rend())
		{
			return oIt->first;
		}
		else
		{
			return 0;
		}
	}

	bool DetectCalibrationLayers::validMaximae()
	{
		if ( (m_oMaximae.oIdx[1] < 0) || (m_oMaximae.oIdx[0] < 0) )
		{
			return false;
		}
		if (std::abs(m_oMaximae.oIdx[0] - m_oMaximae.oIdx[1]) < m_oMinDist)
		{
			return false;
		}
		return true;
	}

	inline void DetectCalibrationLayers::insert(std::vector<int> &p_rVecMaximae, const int p_oIdxMaximae,
		const std::vector<int> &p_rIntensities, const int p_oIndex)
	{
		while (p_oIdxMaximae >= (int)p_rVecMaximae.size())
		{
			p_rVecMaximae.push_back(p_oIndex);
		}
		if (p_rIntensities[p_oIndex] > p_rIntensities[p_rVecMaximae[p_oIdxMaximae]])
		{
			p_rVecMaximae[p_oIdxMaximae] = p_oIndex;
		}
	}

	void DetectCalibrationLayers::getHighestPeaks(tMaximae &p_rMaximae, const std::vector<int> &p_rIntensities, const std::vector<int> &p_rVecMaximae)
	{
		for (unsigned int i=0; i < p_rVecMaximae.size(); ++i)
		{
			int oIdx = p_rVecMaximae[i];
			if (p_rIntensities[oIdx] <= m_oMaximae.oValue[1])
			{
				continue;
			}
			if (p_rIntensities[oIdx] <= m_oMaximae.oValue[0])
			{
				m_oMaximae.oIdx[1] = oIdx; m_oMaximae.oValue[1] = p_rIntensities[oIdx];
			}
			else
			{
				m_oMaximae.oIdx[1] = m_oMaximae.oIdx[0]; m_oMaximae.oValue[1] = m_oMaximae.oValue[0];
				m_oMaximae.oIdx[0] = oIdx; m_oMaximae.oValue[0] = p_rIntensities[oIdx];
			}
		}
	}

	bool DetectCalibrationLayers::findLayerCenters(std::vector<int> &p_rIntensities, const ImageFrame &p_rFrame)
	{
		auto oData = p_rFrame.data();
		int oHeight = oData.height(); m_oWidth = oData.width();

		p_rIntensities.assign(oHeight, 0);
		std::map<int, int> oBins;
		int oNormalizedIntensity = 0; const double oNormalizer = 1.0;// p_rFrame.data().width();

		const unsigned int oBackgroundThreshold = 60;

		// First step: Compute pooled intensities
		p_rIntensities[0] = 0;
		for (int oY = 1; oY < oHeight; ++oY)
		{
			unsigned int oSum = 0; //sum of bright pixel intensities along the row
			unsigned int oTCnt = 0;
			for (int oX = 0; oX < m_oWidth; ++oX)
			{
				if ( oData[oY][oX] > oBackgroundThreshold ) //if pixelvalue bigger than background threshold
				{
					oSum += oData[oY][oX];
					++oTCnt;
				}
			} //now I've scanned all the row, i compute the mean (if it's not just noise)
			int oBinIdx = 0;
			if (oTCnt > 10) // we need at least 10 pixel in a row, otherwise it is just noise
			{
				p_rIntensities[oY] = oSum / oTCnt;
				oBinIdx = oSum / oTCnt;
			}
			else
			{
				p_rIntensities[oY] = 0;
			}
			if ( oBins.find( oBinIdx ) != oBins.end() ) //at the same time build an histogram of the row mean "bright" intensities
			{
				oBins[oBinIdx]++;
			}
			else
			{
				oBins[oBinIdx] = 1;
			}
		}


        auto firstQuartileBrightLines = [&oBins, &oHeight]()
            {
                int backgroundLines = oBins[0];
                int foregroundLines = (oHeight-backgroundLines);
                int counterLimitAtFirstQuartile = foregroundLines*0.75; // counting from end

                int counter = 0;
                auto oIt = oBins.rbegin();
                for ( ; oIt != oBins.rend() && counter < counterLimitAtFirstQuartile; oIt++)
                {
                    counter += oIt->second;
                }
                return oIt->first;
            }();

		m_oActualThreshold = m_oParameterThreshold;
		if (m_oActualThreshold == 0)
		{
			m_oActualThreshold = (int)(guessThreshold(oBins) * oNormalizer);
			if (m_oActualThreshold == 0)
			{
				//guess Threshold was not successful. I try another time with the 3rd quartile of the intensities
				int counter = 0;
				auto oIt = oBins.rbegin();
				for ( ; oIt != oBins.rend() && counter < oHeight/4  ; oIt++)
				{
					counter += oIt->second;
				}
				m_oActualThreshold = oIt->first;

				wmLog( eInfo, "Threshold parameter suggestion (Detect calibration layers - method 2 ): %d\n", m_oActualThreshold);
			}
			else
			{
				wmLogTr(eInfo, "QnxMsg.Calib.AutoThOK", "Threshold parameter suggestion (Detect calibration layers): %d\n", m_oActualThreshold);
			}
		}
		if (m_oActualThreshold == -1)
        {
            m_oActualThreshold = firstQuartileBrightLines;
            wmLog( eInfo, "Threshold parameter suggestion (Detect calibration layers - method 3 ): %d\n", m_oActualThreshold);
        }

		wmLog( eDebug, "Image Rows intensities: from %d to %d (1st quartile bright lines %d), Threshold: %d\n", oBins.begin()->first, oBins.rbegin()->first, firstQuartileBrightLines, m_oActualThreshold );

		// 2nd step: average over 4 pixel to eliminate potential insignificant threshold gaps. No margin treatment here, kept very simple
		/*
		for (int oY = 1; oY < oHeight - 5; ++oY)
		{
			oSum = (p_rIntensities[oY] + p_rIntensities[oY+1] + p_rIntensities[oY+2] + p_rIntensities[oY+3]) >> 2;
			p_rIntensities[oY] = oSum;
		}
		*/
		resetMaximae();

		std::vector<int> oMaximae;

		// 3rd step: Seek two well separated maximae and determine ROIs
		bool oAreaOfInterest = false; // Areas of interest are regions where the normalized intensities are above the threshold
		int oPrevArea = -1; int oPos = 0;

		for (int oY=1; oY < oHeight; ++oY)
		{
			oNormalizedIntensity = (int)(p_rIntensities[oY] * oNormalizer);
			oAreaOfInterest = (oNormalizedIntensity > m_oActualThreshold);
			if (oAreaOfInterest)
			{
				if ( (oPrevArea <= 0) || (oPrevArea == m_oMinDist) )
				{
					oPrevArea = m_oMinDist;
					insert(oMaximae, oPos, p_rIntensities, oY);
				}
				else
				{
					--oPrevArea;
				}
			}
			else
			{
				if (oPrevArea > 0)
				{
					--oPrevArea;
					if (oPrevArea == 0)
					{
						++oPos;
					}
				}
			}
		}

        {
            std::ostringstream oMsg;
            oMsg << "Found " << oMaximae.size() << " max values ";
            for (auto & idx : oMaximae)
            {
                oMsg << "y= " << idx;
                if (idx < (int) p_rIntensities.size())
                {
                    oMsg << "(" << p_rIntensities[idx] << ")";
                }
            }
            oMsg << "\n";
            wmLog(eDebug, oMsg.str());
        }

		if (oMaximae.size() > 1)
		{
			getHighestPeaks( m_oMaximae, p_rIntensities, oMaximae);
			return validMaximae();
		}
		else
		{
			return false;
		}
	}

	bool DetectCalibrationLayers::determineExtent(const std::vector<int> &p_rIntensities, const int p_oIdxMax)
	{
		unsigned int oMinAdjacentExtension = 6; // evtl. in Parameter umwandeln!!!

		if (( p_oIdxMax < 0) || (p_oIdxMax > 1))
		{
			return false;
		}

		unsigned int oGoove = oMinAdjacentExtension;
		int oCnt = 0;

		while ( (oGoove >= 2) && (oCnt < 2) ) // brute force search over number of adjacent lines
		{
			oGoove = oMinAdjacentExtension; bool oTestOK = false; oCnt = 0;
			m_oRoiYTop[p_oIdxMax] = m_oRoiYBottom[p_oIdxMax] = m_oMaximae.oIdx[p_oIdxMax];

			while ( (m_oRoiYTop[p_oIdxMax] > 1) && (oGoove > 0) )
			{
				oTestOK = ( p_rIntensities[m_oRoiYTop[p_oIdxMax]] >= (m_oExtentFactor * m_oMaximae.oValue[p_oIdxMax]) );
				if (  (oGoove > 0) || oTestOK )
				{
					if (!oTestOK)
					{
						--oGoove;
					}
					else
					{
						oGoove = oMinAdjacentExtension;
					}
				}
				--m_oRoiYTop[p_oIdxMax];
			}
			oCnt += (oGoove == 0);
			//	++m_oRoiYTop[m_oIdxMax];

			// Rather arbitrarily chosen, we need at least oGoove adjacent lines with avg. intensity above or equal to the threshold m_oExtentFacitor*m_oMaximae...
			oGoove = oMinAdjacentExtension; oTestOK = false;
			while ( (m_oRoiYBottom[p_oIdxMax] < (int)p_rIntensities.size()-2) && (oGoove > 0) )
			{
				oTestOK = ( p_rIntensities[m_oRoiYBottom[p_oIdxMax]] >= (m_oExtentFactor * m_oMaximae.oValue[p_oIdxMax]) );
				if (  (oGoove > 0) || oTestOK )
				{
					++m_oRoiYBottom[p_oIdxMax];
					if (!oTestOK)
					{
						--oGoove;
					}
					else
					{
						oGoove = oMinAdjacentExtension;
					}
				}
			}
			oCnt += (oGoove == 0);

			--oMinAdjacentExtension;
			//--m_oRoiYBottom[m_oIdxMax];
		}

		return ( (oCnt == 2) && (m_oRoiYBottom[p_oIdxMax] - m_oRoiYTop[p_oIdxMax]) > (m_oMinDist >> 2) );
	}

	bool DetectCalibrationLayers::determineROIs(const std::vector<int> &p_rIntensities)
	{
		// very simple, unrolled implementation: Given the "Extent" parameter, seek valid region above and below both center positions
		return ( determineExtent(p_rIntensities, 0) && determineExtent(p_rIntensities, 1) );
	}

	void DetectCalibrationLayers::signalSend(const ImageContext &p_rImgContext, const int p_oIO)
	{
		if (p_oIO == 1)
		{
			preSignalAction();
			m_oPipeOutRois.signal( GeoVecDoublearray(p_rImgContext, m_oRois, AnalysisOK, 1.0) );
		}
		else
		{
			preSignalAction();
			m_oPipeOutRois.signal( GeoVecDoublearray(p_rImgContext, m_oRois, interface::AnalysisErrBadCalibration, eRankMin) );
		}
	}

	void DetectCalibrationLayers::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
	{
		m_oPaint = false; // needs to be at the very top to make sure paint will not be calles when errors occur!

		poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

		const ImageFrame &rFrame (m_pPipeInImageFrame->read(m_oCounter));
		const ImageContext &rContext(rFrame.context());

		if ( !rFrame.data().isValid() )
		{
			wmLog(eWarning, "Detect Calibration Layers: empty input image\n");
			signalSend(rContext, 0);
			return;
		}

		std::vector<int> oIntensities;
		m_oRois.resize(1);

		m_oRois[0].getData().assign(4, 0.0);
		m_oRois[0].getRank().assign(4, eRankMin);
		m_oSpTrafo = rFrame.context().trafo();

		bool foundLayerCenters = findLayerCenters(oIntensities, rFrame);
		if ( !foundLayerCenters )
		{
			wmLog(eWarning, "Detect Calibration Layers: could not find layer centers\n");
			signalSend(rContext, 0);
			return;
		}

		bool foundROIs = determineROIs(oIntensities);
		if ( !foundROIs )
		{
			wmLog(eWarning, "Detect Calibration Layers: could not determine ROI\n");
			signalSend(rContext, 0);
			return;
		}

		int oIdx=0;
		if (m_oRoiYBottom[0] > m_oRoiYTop[1])
		{
			oIdx = 1;
		}
		m_oPaint = true;
		m_oRois[0].getData()[0] = 1.0*m_oRoiYTop[oIdx]; m_oRois[0].getRank()[0] = eRankMax; // top layer (lower y pixel value)
		m_oRois[0].getData()[1] = 1.0*m_oRoiYBottom[oIdx]; m_oRois[0].getRank()[1] = eRankMax;
		m_oRois[0].getData()[2] = 1.0*m_oRoiYTop[1-oIdx]; m_oRois[0].getRank()[2] = eRankMax; // bottom layer (higher y pixel value)
		m_oRois[0].getData()[3] = 1.0*m_oRoiYBottom[1-oIdx]; m_oRois[0].getRank()[3] = eRankMax;
		signalSend(rContext, 1);
	}


}
}
