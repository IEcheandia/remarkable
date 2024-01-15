/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter extracs the width of the laserline, so seams can be found in thin line areas
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "lineWidth.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineWidth::m_oFilterName 		= std::string("LineWidth");
const std::string LineWidth::PIPENAME_OUT	= std::string("WidthArrayOut");

LineWidth::LineWidth() :
	TransformFilter( LineWidth::m_oFilterName, Poco::UUID{"E679509E-3138-4102-B5EE-199C6F5811FA"} ),
	m_pPipeInImageFrame( NULL ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeOutLineWidth( NULL ),
	m_oThreshold( 255 ),
	m_oSearchHeight( 20 ),
	m_oResolution(1),
	m_oSootThresh(40),
	m_oSootThresh2(60),
	m_oSootFac(40),
	m_oSootFac2(20),
	m_oLineWidthOut(1)
{
	m_pPipeOutLineWidth = new SynchronePipe< GeoVecDoublearray >( this, LineWidth::PIPENAME_OUT );

	// Set default values of the parameters of the filter
	parameters_.add("Mode",    Parameter::TYPE_int, m_oMode);
	parameters_.add("Threshold",    Parameter::TYPE_int, m_oThreshold);
	parameters_.add("SearchHeight", Parameter::TYPE_int, m_oSearchHeight);
	parameters_.add("SootThresh", Parameter::TYPE_int, m_oSootThresh);       // Dunkler Russ
	parameters_.add("SootThresh2", Parameter::TYPE_int, m_oSootThresh2);     // Leichter Russ
	parameters_.add("SootFac", Parameter::TYPE_int, m_oSootFac);             // Dunkler Russ
	parameters_.add("SootFac2", Parameter::TYPE_int, m_oSootFac2);           // Leichter Russ
	parameters_.add("Resolution", Parameter::TYPE_int, m_oResolution);

    setInPipeConnectors({{Poco::UUID("C15DEC9B-20F1-4A92-9630-51AAF2E45C2F"), m_pPipeInLaserLine, "LaserLineIn", 1, "LaserLine"},
    {Poco::UUID("99CE511E-A549-4B55-803B-E611C57C4D4E"), m_pPipeInImageFrame, "ImageFrameIn", 1, "ImageFrame"}});
    setOutPipeConnectors({{Poco::UUID("1D01CCB9-2B66-4F20-AE21-B7FC9830B49C"), m_pPipeOutLineWidth, PIPENAME_OUT, 0, ""}});
    setVariantID(Poco::UUID("41D502B1-0CDE-4573-823F-068DA70B225C"));
}

LineWidth::~LineWidth()
{
	delete m_pPipeOutLineWidth;
}

void LineWidth::setParameter()
{
	TransformFilter::setParameter();
	m_oMode    = parameters_.getParameter("Mode").convert<int>();
	m_oThreshold    = parameters_.getParameter("Threshold").convert<int>();
	m_oSearchHeight = parameters_.getParameter("SearchHeight").convert<int>();
	m_oSootThresh = parameters_.getParameter("SootThresh").convert<int>();
	m_oSootThresh2 = parameters_.getParameter("SootThresh2").convert<int>();
	m_oSootFac = parameters_.getParameter("SootFac").convert<int>();
	m_oSootFac2 = parameters_.getParameter("SootFac2").convert<int>();
	m_oResolution = parameters_.getParameter("Resolution").convert<int>();
}

bool LineWidth::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.type() == typeid(ImageFrame) )
		m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void LineWidth::paint()
{
	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineWidthOut) || m_oSpTrafo.isNull())
	{
		return;
	} // if

	int ydiff = m_overlayMax - m_overlayMin;
	if (ydiff <= 0) return;

    // Malbereich in y
	const int yo = 1;
	const int yu = m_overlayHeight - 2;

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto&	rGreyProfile	( m_oLineWidthOut.front().getData() );
    int yOld, yNew;

	for (unsigned int i = 0; i != rGreyProfile.size(); ++i)
	{
        if ((2 * m_overlayMax) < (m_overlayHeight -2))
        {
            yNew = (int) (0.5 + yu - 2 * rGreyProfile[i]);
        }
        else
        {
            // Scale the points for display
            yNew = (int) (0.5 + yu - (rGreyProfile[i] / m_overlayMax * (yu - yo)));
        }

        if (i == 0)
            yOld = yNew;

        if (yNew == yOld)
        {
            // Mark this single point
			rLayerContour.add<OverlayPoint>(rTrafo(Point(i, yNew)), Color::Orange());
        }
        else
        {
            // Draw a line from before point to actual
			rLayerContour.add<OverlayLine>(rTrafo(Point( i - 1, yOld)), rTrafo(Point(i, yNew)), Color::Orange());
            yOld = yNew;
        }
	}
}

void LineWidth::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	m_overlayMin = 1000000;
	m_overlayMax = -1000000;
    m_overlayHeight = 10;

	// Read out image frame from pipe
	const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
	m_oSpTrafo	= rFrameIn.context().trafo();
	// Extract actual image and size
	const BImage &rImageIn = rFrameIn.data();
	// Read-out laserline
	const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);
	m_oSpTrafo	= rLaserLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rLaserarray = rLaserLineIn.ref();
	// input validity check

	if ( inputIsInvalid(rLaserLineIn) )
	{
		GeoVecDoublearray oGeoProfile = GeoVecDoublearray( rFrameIn.context(), m_oLineWidthOut, rFrameIn.analysisResult(), interface::NotPresent );
		preSignalAction();
		m_pPipeOutLineWidth->signal( oGeoProfile );

		return; // RETURN
	}

	// Now do the actual image processing
	bool isGood = calcLineWidth( rImageIn, rLaserarray, m_oThreshold, m_oLineWidthOut, m_oSearchHeight);

	// Create a new byte array, and put the global context into the resulting profile
	const auto oAnalysisResult	= rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray(rFrameIn.context(), m_oLineWidthOut, oAnalysisResult, (isGood) ? interface::Perfect : interface::NotPresent );
	preSignalAction();
	m_pPipeOutLineWidth->signal( rGeoProfile );
} // proceedGroup

bool LineWidth::calcLineWidth( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, int p_oThreshold,
	geo2d::VecDoublearray &p_rProfileOut, int p_oSearchHeight)
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();   // Number of (laser) lines
	int imageHeight = p_rImageIn.height();
	m_overlayHeight = imageHeight;   // Height of the laser line ROI
	p_rProfileOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
    if ( oNbLines == 0 )
    {
        return false;
    }
    bool isGood = true;
    for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines

		// get the references to the stl vectors
		auto& rProfileOut_Data = p_rProfileOut[lineN].getData();
		auto& rProfileOut_Rank = p_rProfileOut[lineN].getRank();
		const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
		const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

		// if the size of the profile is not equal to the laser line size, resize the profile
		if ( rProfileOut_Data.size() != rLaserLineIn_Data.size() )
		{
			rProfileOut_Data.resize( rLaserLineIn_Data.size() );
			rProfileOut_Rank.resize( rLaserLineIn_Rank.size() );
		}
		std::copy( rLaserLineIn_Rank.begin(), rLaserLineIn_Rank.end(), rProfileOut_Rank.begin() );

		int oLineY;
		int oSum;

		int lastWidth = -1;
		if (m_oResolution < 1) m_oResolution = 1;

		double meanWidth = 0;
		int countValid = 0;
		unsigned int totalPoints = rLaserLineIn_Data.size();   // Width of the laser line ROI

		for (unsigned int x = 0; x<totalPoints; x++)
		{
			if ((x % m_oResolution) != 0) // Nicht jedes berechnen, Parameter Resolution steuert das
			{
				rProfileOut_Data[x] = lastWidth;
				continue;
			}

			oLineY = int( rLaserLineIn_Data[x] );
			int rank = rLaserLineIn_Rank[x];

			oSum = 0;

			if (rank <= 0)
			{
				rProfileOut_Data[x] = -1;
				m_overlayMin = 0;
				continue;
			}

			// Search range around the tracked laser line
			for (int y=oLineY-p_oSearchHeight/2; y<=oLineY+p_oSearchHeight/2; y++)
			{
				if (y < 0) continue;
				if (y >= imageHeight) continue;

				if (p_rImageIn[y][x] >= p_oThreshold) oSum++;
			}

			// Mittelwert drueber und drunter fuer Russdetektion berechnen

			int factor10 = 10;

			if ( (m_oSootThresh != 0) || (m_oSootThresh2 != 0) )
			{
				// Mittelwert oben berechnen
				int counterTop = 0;
				double sum = 0.0;
                int yUpper = oLineY - p_oSearchHeight * 0.5;

				for (int y = yUpper - 20; y <= yUpper; y += 2)
				{
					if (y < 0) continue;
					if (y >= imageHeight) continue;

					sum += p_rImageIn[y][x];
					counterTop++;
				}

				if (counterTop == 0)
				{
					sum = 255;
					counterTop = 1;
				}

				int meanTop = (int)(0.5 + (sum / counterTop));

				// Mittelwert unten berechnen
				sum = 0.0;
				int counterBottom = 0;
                int yLower = oLineY + p_oSearchHeight * 0.5;

				for (int y = yLower; y <= yLower + 20; y += 2)
				{
					if (y < 0) continue;
					if (y >= imageHeight) continue;

					sum += p_rImageIn[y][x];
					counterBottom++;
				}

				if (counterBottom == 0)
				{
					sum = 255;
					counterBottom = 1;
				}

				int meanBottom = (int)(0.5 + (sum / counterBottom));

				// 1 ist fuer richtig dunkeln Russ, 2 fuer leichten, etwas helleren
				if ((counterTop    > 7) && (meanTop <    m_oSootThresh2)) factor10 = m_oSootFac2; // __SOOTFACTOR2;
				if ((counterBottom > 7) && (meanBottom < m_oSootThresh2)) factor10 = m_oSootFac2; // __SOOTFACTOR2;
				if ((counterTop    > 7) && (meanTop <    m_oSootThresh))  factor10 = m_oSootFac;
				if ((counterBottom > 7) && (meanBottom < m_oSootThresh))  factor10 = m_oSootFac;
			}

			if ( (oSum<=2) && (factor10 > 10) ) oSum = 5; // Russ wurde gefunden, Linie ist aber komplett weg => hochsetzen
			oSum = (int)(0.5 + oSum * (factor10 * 0.1));

			rProfileOut_Data[x] = oSum;
			lastWidth = oSum;

			meanWidth += oSum;
			countValid++;

			if (oSum>m_overlayMax) m_overlayMax = oSum;
			if (oSum<m_overlayMin) m_overlayMin = oSum;
		}

		if (countValid == 0)
		{
			meanWidth = 0;
		}
		else
		{
			meanWidth = meanWidth / countValid;
		}

		for (unsigned int x = 0; x < totalPoints; x++)
		{
			if (rProfileOut_Data[x] == -1) rProfileOut_Data[x] = (int)meanWidth;

		}

		double ratio = (countValid * m_oResolution) / (double)totalPoints;

        isGood &= (ratio > 0.3);
	} // for
    return isGood;
} // extractLineProfile



} // namespace precitec
} // namespace filter
