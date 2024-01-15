/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter extracs the brightness of the laserline, so seams can be found in darker areas
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "lineBrightness.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineBrightness::m_oFilterName 		= std::string("LineBrightness");
const std::string LineBrightness::PIPENAME_OUT1	= std::string("BrightnessArrayOut");
const std::string LineBrightness::PIPENAME_OUT2 = std::string("MeanBrightnessOut");

LineBrightness::LineBrightness() :
	TransformFilter( LineBrightness::m_oFilterName, Poco::UUID{"E9C0CF37-2711-489A-9FC7-5278CFC4614C"} ),
	m_pPipeInImageFrame( NULL ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeOutLineBrightness( NULL ),
	m_pPipeOutLineMeanBrightness( NULL ),
	m_oSearchHeight(20),
	m_oLineBrightnessOut( 1 ),
	m_oLineMeanBrightnessOut( 0 )
{
	m_pPipeOutLineBrightness = new SynchronePipe< GeoVecDoublearray >( this, LineBrightness::PIPENAME_OUT1 );
	m_pPipeOutLineMeanBrightness = new SynchronePipe< GeoDoublearray >(this, LineBrightness::PIPENAME_OUT2);

	// Set default values of the parameters of the filter
	parameters_.add("SearchHeight", Parameter::TYPE_int, m_oSearchHeight);

    setInPipeConnectors({{Poco::UUID("9984FF4E-0989-4A5B-A681-5B7668B0688E"), m_pPipeInLaserLine, "LaserLineIn", 1, "LaserLine"},
    {Poco::UUID("09AB5276-AD52-4D76-9AF6-08A669EBCCC3"), m_pPipeInImageFrame, "ImageFrameIn", 1, "ImageFrame"}});
    setOutPipeConnectors({{Poco::UUID("56AD24D6-D991-46C0-99C4-1DCAA87CDEF8"), m_pPipeOutLineBrightness, PIPENAME_OUT1, 0, ""},
    {Poco::UUID("CF205874-6DE5-407D-8CFB-B9B7CE8C6BC4"), m_pPipeOutLineMeanBrightness, PIPENAME_OUT2, 0, ""}});
    setVariantID(Poco::UUID("025BBC78-9942-4646-B253-1E0DA3BA60A8"));
} // LineProfile

LineBrightness::~LineBrightness()
{
	delete m_pPipeOutLineBrightness;
	delete m_pPipeOutLineMeanBrightness;
} // ~LineProfile

void LineBrightness::setParameter()
{
	TransformFilter::setParameter();
	m_oSearchHeight = parameters_.getParameter("SearchHeight").convert<int>();

} // setParameter

bool LineBrightness::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.type() == typeid(ImageFrame) )
		m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe

void LineBrightness::paint()
{
	// bool isInvalid = inputIsInvalid(m_oLineBrightnessOut);

	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineBrightnessOut) || m_oSpTrafo.isNull())
	{
		return;
	} // if

	int ydiff = m_overlayMax - m_overlayMin;
	if (ydiff < 0) return;

	const int yo = 1; // Malbereich in y
	const int yu = 100;

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto&	rGreyProfile	( m_oLineBrightnessOut.front().getData() );

    {
        int yOld = 0, yNew = 0;

        for (unsigned int i = 0; i != rGreyProfile.size(); ++i)
        {
            if ((2 * m_overlayMax) < (yu - yo - 4))
            {
                yNew = (int) (0.5 + yu - 2 * rGreyProfile[i]);
            }
            else
            {
                // Scale the points for display
                yNew = (int) (0.5 + yu - (rGreyProfile[i] / m_overlayMax * (yu - yo -4)));
            }

            if (i == 0)
            {
                yOld = yNew;
            }

            if (yNew == yOld)
            {
                // Mark this single point
                rLayerContour.add<OverlayPoint>(rTrafo(Point(i, yNew)), Color::Orange());
            }
            else
            {
                // Draw a line from before point to actual
                rLayerContour.add<OverlayLine>(rTrafo(Point(i - 1, yOld)), rTrafo(Point(i, yNew)), Color::Orange());
                yOld = yNew;
            }
        } // for
    }
} // paint



void LineBrightness::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	m_overlayMin = 1000000;
	m_overlayMax = -1000000;

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
		const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray( rFrameIn.context(), m_oLineBrightnessOut, rFrameIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rGeoMean = GeoDoublearray(rFrameIn.context(), m_oLineMeanBrightnessOut, rFrameIn.analysisResult(), interface::NotPresent);
		preSignalAction();
		m_pPipeOutLineBrightness->signal( rGeoProfile );
		m_pPipeOutLineMeanBrightness->signal(rGeoMean);

		return; // RETURN
	}

	// Now do the actual image processing
	calcLineBrightness(rImageIn, rLaserarray, m_oLineBrightnessOut, m_oSearchHeight, m_overlayMin, m_overlayMax, m_oLineMeanBrightnessOut);

	// Create a new byte array, and put the global context into the resulting profile
	const auto oAnalysisResult	= rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray(rFrameIn.context(), m_oLineBrightnessOut, oAnalysisResult, rLaserLineIn.rank() );
	const GeoDoublearray &rGeoMean = GeoDoublearray(rFrameIn.context(), m_oLineMeanBrightnessOut, rFrameIn.analysisResult(), rLaserLineIn.rank());
	preSignalAction();
	m_pPipeOutLineBrightness->signal( rGeoProfile );
	m_pPipeOutLineMeanBrightness->signal(rGeoMean);

} // proceedGroup

void LineBrightness::calcLineBrightness( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn,
	geo2d::VecDoublearray &p_rProfileOut, int p_oSearchHeight, int &overlayMin, int &overlayMax, geo2d::Doublearray &p_rLineMeanOut)
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();
	p_rProfileOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	geo2d::Doublearray oLineMean;

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

		//int oStartY = p_oProfileHeight + p_oLineHeight;
		int oLineY;
		int oSum;
		int count;
		int oSumMean = 0;
		int countMean = 0;
		//int * SumArray = new int[rLaserLineIn_Data.size()];

		for (unsigned int x=0; x<rLaserLineIn_Data.size(); x++)
		{
			oLineY = int( rLaserLineIn_Data[x] );
			oSum = 0; count=0;

			if (rLaserLineIn_Rank[x] == 0)
			{
				rProfileOut_Data[x] = 0;
				continue;
			}

			for (int y=oLineY-p_oSearchHeight/2; y<=oLineY+p_oSearchHeight/2; y++)
			{
				if (y<0) continue;
				if (y>=p_rImageIn.height()) continue;

				oSum += p_rImageIn[y][x];
				count++;
			} // for y

			if (count > 0)
            {
                int val = oSum / count;
                rProfileOut_Data[x] = val;

                if (val>overlayMax) overlayMax = val;
                if (val<overlayMin) overlayMin = val;

                // Mean Brightness of laser line
                oSumMean += val;
                countMean++;
            }
		} // for x

		// Mean Brightness of Laser line
		int valMean = 0;
        if (countMean > 0)
        {
            valMean = oSumMean / countMean;
        }

		oLineMean.getData().push_back(valMean);
		if (valMean > 0) {
			oLineMean.getRank().push_back(255);
		}
		else {
			oLineMean.getRank().push_back(0);
		}

	} // for

	p_rLineMeanOut = oLineMean;
} // extractLineProfile



} // namespace precitec
} // namespace filter
