/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter deletes part of the laser line.
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "improveLine.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string ImproveLine::m_oFilterName 		 = std::string("ImproveLine");
//const std::string ImproveLine::PIPENAME_IMAGE_OUT	 = std::string("ImageFrameOut");
const std::string ImproveLine::PIPENAME_LASERLINE_OUT = std::string("LaserLineOut");
//const std::string ImproveLine::PIPENAME_DOUBLE1_OUT   = std::string("Double1Out");
//const std::string ImproveLine::PIPENAME_DOUBLE2_OUT	 = std::string("Double2Out");
//const std::string ImproveLine::PIPENAME_DOUBLE3_OUT   = std::string("Double3Out");

ImproveLine::ImproveLine() :
	TransformFilter( ImproveLine::m_oFilterName, Poco::UUID{"C5EC90BD-BD69-486B-8018-18CC3740DD61"} ),
	m_pPipeInImageFrame( NULL ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeInDouble1( NULL ),
	m_pPipeInDouble2( NULL ),
	m_pPipeInDouble3( NULL ),

	m_oMeanRange( 0 ),	m_oMinBright( 0 )
	//m_oParameter2( 0 ),
	//m_oParameter3( 0 ),
	//m_oParameter4( 0 ),
	//m_oParameter5( 0 ),
	//m_oParameter6( 0 ),
	//m_oParameter7( 0 )
{
	//m_pPipeOutImageFrame = new SynchronePipe< interface::ImageFrame > ( this, ImproveLine::PIPENAME_IMAGE_OUT );
	m_pPipeOutLaserLine = new SynchronePipe< interface::GeoVecDoublearray > ( this, ImproveLine::PIPENAME_LASERLINE_OUT );
	//m_pPipeOutDouble1 = new SynchronePipe< interface::GeoDoublearray > ( this, ImproveLine::PIPENAME_DOUBLE1_OUT );
	//m_pPipeOutDouble2 = new SynchronePipe< interface::GeoDoublearray > ( this, ImproveLine::PIPENAME_DOUBLE2_OUT );
	//m_pPipeOutDouble3 = new SynchronePipe< interface::GeoDoublearray > ( this, ImproveLine::PIPENAME_DOUBLE3_OUT );

	// Set default values of the parameters of the filter
	parameters_.add("MeanRange", Parameter::TYPE_int, m_oMeanRange);
	parameters_.add("MinBright", Parameter::TYPE_int, m_oMinBright);
	//parameters_.add("Parameter2", Parameter::TYPE_int, m_oParameter2);
	//parameters_.add("Parameter3", Parameter::TYPE_int, m_oParameter3);
	//parameters_.add("Parameter4", Parameter::TYPE_int, m_oParameter4);
	//parameters_.add("Parameter5", Parameter::TYPE_int, m_oParameter5);
	//parameters_.add("Parameter6", Parameter::TYPE_int, m_oParameter6);
	//parameters_.add("Parameter7", Parameter::TYPE_int, m_oParameter7);

    setInPipeConnectors({{Poco::UUID("75268020-D434-4946-B552-AF1BF5777560"), m_pPipeInImageFrame, "ImageFrameIn", 1, "ImageFrameIn"},
    {Poco::UUID("406568D4-F9B6-4938-8DC6-E848B85AD5DD"), m_pPipeInLaserLine, "LaserLineIn", 1, "LaserLineIn"}});
    setOutPipeConnectors({{Poco::UUID("7161CF6E-0342-4913-820B-E293646AA8DA"), m_pPipeOutLaserLine, PIPENAME_LASERLINE_OUT, 0, ""}});
    setVariantID(Poco::UUID("37C0C395-64CC-4769-B4EB-251D944B4E68"));
}

ImproveLine::~ImproveLine()
{
	//delete m_pPipeOutImageFrame;
	delete m_pPipeOutLaserLine;
	//delete m_pPipeOutDouble1;
	//delete m_pPipeOutDouble2;
	//delete m_pPipeOutDouble3;
}

void ImproveLine::setParameter()
{
	TransformFilter::setParameter();
	m_oMeanRange = parameters_.getParameter("MeanRange").convert<int>();
	m_oMinBright = parameters_.getParameter("MinBright").convert<int>();
	//m_oParameter2 = parameters_.getParameter("Parameter2").convert<int>();
	//m_oParameter3 = parameters_.getParameter("Parameter3").convert<int>();
	//m_oParameter4 = parameters_.getParameter("Parameter4").convert<int>();
	//m_oParameter5 = parameters_.getParameter("Parameter5").convert<int>();
	//m_oParameter6 = parameters_.getParameter("Parameter6").convert<int>();
	//m_oParameter7 = parameters_.getParameter("Parameter7").convert<int>();

} // setParameter

bool ImproveLine::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(ImageFrame) )
		m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	//if ( p_rPipe.tag() == "Double1" )
	//	m_pPipeInDouble1  = dynamic_cast< SynchronePipe < GeoDoublearray > * >(&p_rPipe);
	//if ( p_rPipe.tag() == "Double2" )
	//	m_pPipeInDouble2 = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	//if ( p_rPipe.tag() == "Double3" )
	//	m_pPipeInDouble3 = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe

void ImproveLine::paint()
{
	if(m_oVerbosity < eLow  || m_oSpTrafo.isNull())
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

		for (int i=0; i<(int)_drawPoints.size(); i++)
		{
			rLayerContour.add(new OverlayPoint(rTrafo(Point(_drawPoints.at(i)._x, _drawPoints.at(i)._y)), Color::Red()));
		}

		// Example => to delete
		//rLayerContour.add(new OverlayLine(rTrafo(Point(10, 10)), rTrafo(Point(20, 20)), Color::Blue()));
		//rLayerContour.add(new OverlayCross(rTrafo(Point(15, 15)), Color::Red()));

	}
	catch(...)
	{
		return;
	}
} // paint


void ImproveLine::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	//geo2d::Doublearray oOutImageFrame;
	geo2d::Doublearray oOutLaserLine;
	//geo2d::Doublearray oOutDouble1;
	//geo2d::Doublearray oOutDouble2;
	//geo2d::Doublearray oOutDouble3;

	_drawPoints.clear();

	// Read out image frame from pipe
	const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);

	try
	{
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
			//oOutDouble1.getData().push_back(0);
			//oOutDouble1.getRank().push_back(0);
			//oOutDouble2.getData().push_back(0);
			//oOutDouble2.getRank().push_back(0);
			//oOutDouble3.getData().push_back(0);
			//oOutDouble3.getRank().push_back(0);

			const GeoVecDoublearray &rGeoLaserLine = GeoVecDoublearray( rFrameIn.context(), m_oLaserLineOut, rFrameIn.analysisResult(), interface::NotPresent );
			//const GeoDoublearray &rDouble1 = GeoDoublearray(rFrameIn.context(), oOutDouble1, rLaserLineIn.analysisResult(), interface::NotPresent);
			//const GeoDoublearray &rDouble2 = GeoDoublearray(rFrameIn.context(), oOutDouble2, rLaserLineIn.analysisResult(), interface::NotPresent);
			//const GeoDoublearray &rDouble3 = GeoDoublearray(rFrameIn.context(), oOutDouble3, rLaserLineIn.analysisResult(), interface::NotPresent);

			preSignalAction();

			//m_pPipeOutImageFrame->signal(rFrameIn);
			m_pPipeOutLaserLine->signal(rGeoLaserLine);
			//m_pPipeOutDouble1->signal(rDouble1);
			//m_pPipeOutDouble2->signal(rDouble2);
			//m_pPipeOutDouble3->signal(rDouble3);

			return; // RETURN
		}

		// Here: Do some calculation
		// Fill up oOutDoubleX and LaserLineOut
		m_hasPainting = true;

		const unsigned int	oNbLines	= rLaserarray.size();
		geo2d::VecDoublearray rLineOut;
		rLineOut.resize(oNbLines);

		for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
		{ // loop over N lines

			std::vector<double, std::allocator<double>> & rLineOut_Data = rLineOut[lineN].getData();
			std::vector<int, std::allocator<int>> & rLineOut_Rank = rLineOut[lineN].getRank();

			const std::vector<double, std::allocator<double>> & rLaserLineIn_Data = rLaserarray[lineN].getData();
			const std::vector<int, std::allocator<int>> & rLaserLineIn_Rank = rLaserarray[lineN].getRank();

			// if the size of the line is not equal to the laser line size, resize the profile
			if (rLineOut_Data.size() != rLaserLineIn_Data.size())
			{
				rLineOut_Data.resize(rLaserLineIn_Data.size());
				rLineOut_Rank.resize(rLaserLineIn_Rank.size());
			}
			std::copy(rLaserLineIn_Rank.begin(), rLaserLineIn_Rank.end(), rLineOut_Rank.begin());


			if (m_oMode < 100)
			{
				snipLine(rImageIn, rLaserLineIn_Data, rLaserLineIn_Rank, rLineOut_Data, rLineOut_Rank);
			}
			else
			{
				smoothLine(m_oMode, rImageIn, rLaserLineIn_Data, rLaserLineIn_Rank, rLineOut_Data, rLineOut_Rank);
			}

		}

		////////

		//const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type

		const GeoVecDoublearray &rGeoLaserLine = GeoVecDoublearray( rFrameIn.context(), rLineOut, rFrameIn.analysisResult(), filter::eRankMax );
		//const GeoDoublearray &rDouble1 = GeoDoublearray(rFrameIn.context(), oOutDouble1, rLaserLineIn.analysisResult(), filter::eRankMax);
		//const GeoDoublearray &rDouble2 = GeoDoublearray(rFrameIn.context(), oOutDouble2, rLaserLineIn.analysisResult(), filter::eRankMax);
		//const GeoDoublearray &rDouble3 = GeoDoublearray(rFrameIn.context(), oOutDouble3, rLaserLineIn.analysisResult(), filter::eRankMax);

		preSignalAction();

		//m_pPipeOutImageFrame->signal(rFrameIn);
		m_pPipeOutLaserLine->signal(rGeoLaserLine);
		//m_pPipeOutDouble1->signal(rDouble1);
		//m_pPipeOutDouble2->signal(rDouble2);
		//m_pPipeOutDouble3->signal(rDouble3);

	}
	catch (...)
	{
		//oOutDouble1.getData().push_back(0);
		//oOutDouble1.getRank().push_back(0);
		//oOutDouble2.getData().push_back(0);
		//oOutDouble2.getRank().push_back(0);
		//oOutDouble3.getData().push_back(0);
		//oOutDouble3.getRank().push_back(0);

		const GeoVecDoublearray &rGeoLaserLine = GeoVecDoublearray( rFrameIn.context(), m_oLaserLineOut, rFrameIn.analysisResult(), interface::NotPresent );
		//const GeoDoublearray &rDouble1 = GeoDoublearray(rFrameIn.context(), oOutDouble1, rFrameIn.analysisResult(), interface::NotPresent);
		//const GeoDoublearray &rDouble2 = GeoDoublearray(rFrameIn.context(), oOutDouble2, rFrameIn.analysisResult(), interface::NotPresent);
		//const GeoDoublearray &rDouble3 = GeoDoublearray(rFrameIn.context(), oOutDouble3, rFrameIn.analysisResult(), interface::NotPresent);

		preSignalAction();

		//m_pPipeOutImageFrame->signal(rFrameIn);
		m_pPipeOutLaserLine->signal(rGeoLaserLine);
		//m_pPipeOutDouble1->signal(rDouble1);
		//m_pPipeOutDouble2->signal(rDouble2);
		//m_pPipeOutDouble3->signal(rDouble3);

		return;
	}
} // proceedGroup



void ImproveLine::snipLine(const precitec::image::BImage & rImageIn, const std::vector<double, std::allocator<double>> & rLaserLineIn_Data, const std::vector<int, std::allocator<int>> & rLaserLineIn_Rank,
				std::vector<double, std::allocator<double>> & rLineOut_Data, std::vector<int, std::allocator<int>> & rLineOut_Rank)
{
	double sum = 0; //, sumbad = 0;
	int counter = 0;
	int size = rLaserLineIn_Data.size();

	for (int x = 0; x < size; x++)
	{
		int rank = rLaserLineIn_Rank[x];
		if (rank <= 0) continue;

		int yVal = int(rLaserLineIn_Data[x]);

		sum += rImageIn[yVal][x];
		counter++;
	}

	double mean = (counter>=0) ? sum / counter : 0;

	int snipFront = 0, snipBack = 0;
	int noFailCounter = 0;

	for (int x = 0; x < size; x++)
	{
		snipFront = x;

		int rank = rLaserLineIn_Rank[x];
		if (rank <= 0)
		{
			noFailCounter = 0;
			continue;
		}

		//int yVal = int(rLaserLineIn_Data[x]);
		int meanGreyVal = (int)(0.5+calcMean(rImageIn, rLaserLineIn_Data, rLaserLineIn_Rank, x, m_oMeanRange));

		if (meanGreyVal <= 0)
		{
			noFailCounter = 0;
			continue;
		}

		if (meanGreyVal < m_oMinBright*mean/100.0)
		{
			noFailCounter = 0;
			continue;
		}

		noFailCounter++;

		if (noFailCounter>=3)
		{
			break;
		}
	}

	noFailCounter = 0;

	for (unsigned int x = size-1; x >= 0; x--)
	{
		snipBack = x;

		int rank = rLaserLineIn_Rank[x];
		if (rank <= 0)
		{
			noFailCounter = 0;
			continue;
		}

		//int yVal = int(rLaserLineIn_Data[x]);
		int meanGreyVal = (int)(0.5+calcMean(rImageIn, rLaserLineIn_Data, rLaserLineIn_Rank, x, m_oMeanRange));

		if (meanGreyVal <= 0)
		{
			noFailCounter = 0;
			continue;
		}

		if (meanGreyVal < m_oMinBright*mean/100.0)
		{
			noFailCounter = 0;
			continue;
		}

		noFailCounter++;

		if (noFailCounter>=3)
		{
			break;
		}
	}

	for (int x = 0; x < size; x++)
	{
		if (x<snipFront)
		{
			rLineOut_Data[x]=-1;
			rLineOut_Rank[x]=0;
			continue;
		}

		if (x>snipBack)
		{
			rLineOut_Data[x]=-1;
			rLineOut_Rank[x]=0;
			continue;
		}

		rLineOut_Data[x]=rLaserLineIn_Data[x];
		rLineOut_Rank[x]=rLaserLineIn_Rank[x];

		int rank = rLaserLineIn_Rank[x];
		if (rank <= 0)
		{
			continue;
		}

		int yVal = int(rLaserLineIn_Data[x]);
		_drawPoints.push_back(DrawPoint(x, yVal));
	}

	m_hasPainting = true;
}

void ImproveLine::smoothLine(int mode, const precitec::image::BImage & rImageIn, const std::vector<double, std::allocator<double>> & rLaserLineIn_Data, const std::vector<int, std::allocator<int>> & rLaserLineIn_Rank,
				std::vector<double, std::allocator<double>> & rLineOut_Data, std::vector<int, std::allocator<int>> & rLineOut_Rank)
{
	//double sum = 0, sumbad = 0;
	//int counter = 0;
	int size = rLaserLineIn_Data.size();

	for (int x = 0; x < size; x++)
	{
		rLineOut_Data[x]=0;
		rLineOut_Rank[x]=0;

		int rank = rLaserLineIn_Rank[x];
		if (rank <= 0) continue;

		StatisticCalculator statCalc;
		for (int y = x - m_oMeanRange; y <= x + m_oMeanRange; y++)
		{
			if (y < 0) continue;
			if (y >= size) continue;

			int rank = rLaserLineIn_Rank[y];
			if (rank <= 0) continue;

			statCalc.addValue(rLaserLineIn_Data[y]);
		}

		double val = (mode==100) ? statCalc.getMean() : statCalc.getMedian();
		rLineOut_Data[x]=(int)(val+0.5);

		rLineOut_Rank[x]=rLaserLineIn_Rank[x];

		int yVal = int(rLineOut_Data[x]);
		_drawPoints.push_back(DrawPoint(x, yVal));
	}

	m_hasPainting = true;
}




double ImproveLine::calcMean(const image::BImage &p_rImageIn, const std::vector<double, std::allocator<double>> data, const std::vector<int, std::allocator<int>> rank, int index, int range)
{
	int size_data = data.size();
	int size_rank = rank.size();
	int size = (size_data<size_rank) ?  size_data : size_rank;

	if (index<0) return -1;
	if (index>=size) return -1;

	if (rank[index]<=0) return -1;
	if (data[index]<=0) return -1;

	if (index-range<0) return -1;
	if (index+range>=size) return -1;

	double sum = 0;
	int counter = 0;

	for (int i=-range; i<=range; i++)
	{
		int xVal = index + i;

		if (rank[xVal]<=0) continue;
		if (data[xVal]<=0) continue;

		int yVal = (int)data[xVal];

		sum += p_rImageIn[yVal][xVal];
		counter++;
	}

	if (counter<=0) return -1;

	return sum / counter;
}

DrawPoint::DrawPoint()
{
	_x = 0;
	_y = 0;
}

DrawPoint::DrawPoint(int x, int y)
{
	_x = x;
	_y = y;
}


StatisticCalculator::StatisticCalculator()
{
	reset();
}

void StatisticCalculator::reset()
{
	_data.clear();
}

void StatisticCalculator::addValue(double value)
{
	_data.push_back(value);
}

double StatisticCalculator::getMedian()
{
	int size = _data.size();
	if (size<=0) return 0;
	if (size==1) return _data[0];

	sortIt();
	if (size%2==0) // Anzahl gerade
	{
		return (_data[size/2] + _data[size/2 -1]) / 2.0;
	}
	else // Anzahl ungerade
	{
		return _data[size/2];
	}
}

double StatisticCalculator::getMean()
{
	int size = _data.size();
	if (size<=0) return 0;
	if (size==1) return _data[0];

	double sum = 0;
	for (int i=0; i<size; i++) sum += _data[i];
	return sum / size;
}

void StatisticCalculator::sortIt()
{ // Bubble Sort
	int size = _data.size();
	if (size <= 1) return;
	for (int i=0; i<size-1; i++)
	{
		for (int j=0; j<size-i-1; j++)
		{
			if (_data[j]>_data[j+1]) exchange(_data[j], _data[j+1]);
		}
	}
}

void StatisticCalculator::exchange(double & d1, double & d2)
{
	double d = d1;
	d1 = d2;
	d2 = d;
}



} // namespace precitec
} // namespace filter
