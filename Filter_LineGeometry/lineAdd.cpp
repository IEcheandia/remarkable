/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter extracs the width of the laserline, so seams can be found in thin lien areas
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "lineAdd.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineAdd::m_oFilterName 		= std::string("LineAdd");
const std::string LineAdd::PIPENAME_OUT	= std::string("AddArray");


LineAdd::LineAdd() :
	TransformFilter( LineAdd::m_oFilterName, Poco::UUID{"71B8C861-A66D-471E-B8FB-1B7986BA11D1"} ),
	m_pPipeInLaserLine1( NULL ),
	m_pPipeInLaserLine2( NULL ),
	m_pPipeOutLine( NULL ),
	m_oFac1( 1 ),
	m_oFac2( 1 ),
	m_oLineOut( 1 )
{
	m_pPipeOutLine = new SynchronePipe< GeoVecDoublearray >( this, LineAdd::PIPENAME_OUT );

	// Set default values of the parameters of the filter
	parameters_.add("Factor1",	Parameter::TYPE_int, m_oFac1);
	parameters_.add("Factor2",	Parameter::TYPE_int, m_oFac2);

    setInPipeConnectors({{Poco::UUID("2D25F161-1B94-4F78-A75C-BA7A72051DCB"), m_pPipeInLaserLine1, "Line1In", 1, "Line1"},
    {Poco::UUID("4E276E89-A728-4AED-9554-FEFC28FE04D8"), m_pPipeInLaserLine2, "Line2In", 1, "Line2"}});
    setOutPipeConnectors({{Poco::UUID("0CF57FB9-BE9C-4AC5-A0CE-148C8FEE7845"), m_pPipeOutLine, PIPENAME_OUT, 0, ""}});
    setVariantID(Poco::UUID("61743000-DF8D-45C5-A4F5-2C893BDF5AB0"));
}

LineAdd::~LineAdd()
{
	delete m_pPipeOutLine;
} // ~LineProfile

void LineAdd::setParameter()
{
	TransformFilter::setParameter();
	m_oFac1	= parameters_.getParameter("Factor1").convert<int>();
	m_oFac2	= parameters_.getParameter("Factor2").convert<int>();

} // setParameter

bool LineAdd::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "Line1" )
		m_pPipeInLaserLine1  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "Line2" )
		m_pPipeInLaserLine2  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void LineAdd::paint()
{
	bool isInvalid = inputIsInvalid(m_oLineOut);
	if(m_oVerbosity < eLow || isInvalid || m_oSpTrafo.isNull())
	{
		return;
	} // if

	int ydiff = m_overlayMax - m_overlayMin;
	if (ydiff <= 0) return;

	const int yo = 20; // Malbereich in y
	const int yu = 100;

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto&	rProfile	( m_oLineOut.front().getData() );
	for (unsigned int i = 0; i != rProfile.size(); ++i)
	{
		int yval = (int)(yu - ((rProfile[i] - m_overlayMin) / (ydiff)) * (yu-yo)+0.5);
		rLayerContour.add<OverlayPoint>(rTrafo(Point(i, yval )), Color::Yellow());
	} // for

} // paint


void LineAdd::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine1 != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInLaserLine2 != nullptr); // to be asserted by graph editor
	//poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	m_overlayMin = 1000000;
	m_overlayMax = -1000000;

	// Read out image frame from pipe
	//const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
	//m_oSpTrafo	= rFrameIn.context().trafo();
	// Extract actual image and size
	//const BImage &rImageIn = rFrameIn.data();
	// Read-out laserline
	const GeoVecDoublearray& rLaserLineIn1 = m_pPipeInLaserLine1->read(m_oCounter);
	const GeoVecDoublearray& rLaserLineIn2 = m_pPipeInLaserLine2->read(m_oCounter);
	m_oSpTrafo	= rLaserLineIn1.context().trafo();
	// And extract byte-array
	const VecDoublearray& rLaserarray1 = rLaserLineIn1.ref();
	const VecDoublearray& rLaserarray2 = rLaserLineIn2.ref();
	// input validity check

	if ( inputIsInvalid(rLaserLineIn1) )
	{
		const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray( rLaserLineIn1.context(), m_oLineOut, rLaserLineIn1.analysisResult(), interface::NotPresent );
		preSignalAction();
		m_pPipeOutLine->signal( rGeoProfile );

		return; // RETURN
	}

	if ( inputIsInvalid(rLaserLineIn2) )
	{
		const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray( rLaserLineIn2.context(), m_oLineOut, rLaserLineIn2.analysisResult(), interface::NotPresent );
		preSignalAction();
		m_pPipeOutLine->signal( rGeoProfile );

		return; // RETURN
	}

	// Now do the actual image processing
	calcLineAdd( rLaserarray1, rLaserarray2, m_oFac1, m_oFac2, m_oLineOut);

	// Create a new byte array, and put the global context into the resulting profile
	//const auto oAnalysisResult	= rLaserLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLaserLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray(rLaserLineIn1.context(), m_oLineOut, rLaserLineIn1.analysisResult(), filter::eRankMax );
	preSignalAction();
	m_pPipeOutLine->signal( rGeoProfile );

} // proceedGroup

void LineAdd::calcLineAdd(const geo2d::VecDoublearray &p_rLaserLine1In, const geo2d::VecDoublearray &p_rLaserLine2In, int p_oFac1, int p_oFac2,
		geo2d::VecDoublearray &p_rSumOut)
{
	const unsigned int	oNbLines1	= p_rLaserLine1In.size();
	const unsigned int	oNbLines2	= p_rLaserLine2In.size();
	const unsigned int	oNbLines = (oNbLines1 < oNbLines2) ? oNbLines1 : oNbLines2;
	p_rSumOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines

		// get the references to the stl vectors
		auto& rProfileOut_Data = p_rSumOut[lineN].getData();
		auto& rProfileOut_Rank = p_rSumOut[lineN].getRank();
		const auto& rLaserLine1In_Data = p_rLaserLine1In[lineN].getData();
		const auto& rLaserLine1In_Rank = p_rLaserLine1In[lineN].getRank();
		const auto& rLaserLine2In_Data = p_rLaserLine2In[lineN].getData();
		const auto& rLaserLine2In_Rank = p_rLaserLine2In[lineN].getRank();

		// if the size of the profile is not equal to the laser line size, resize the profile
		unsigned int size1 = rLaserLine1In_Data.size();
		unsigned int size2 = rLaserLine2In_Data.size();
		unsigned int size = (size1 < size2) ? size1 : size2;
		if (rProfileOut_Data.size() != size)
		{
			rProfileOut_Data.resize(size);
			rProfileOut_Rank.resize(size);
		}

		if (size1 < size2)
		{
			std::copy(rLaserLine1In_Rank.begin(), rLaserLine1In_Rank.end(), rProfileOut_Rank.begin());
		}
		else
		{
			std::copy(rLaserLine2In_Rank.begin(), rLaserLine2In_Rank.end(), rProfileOut_Rank.begin());
		}

		//int oStartY = p_oProfileHeight + p_oLineHeight;
		int oLineVal;

		//int * SumArray = new int[rLaserLineIn_Data.size()];

		for (unsigned int x = 0; x < size; x++)
		{
			double val = p_oFac1 * rLaserLine1In_Data[x] + p_oFac2 * rLaserLine2In_Data[x];
			val = (p_oFac1 + p_oFac2 == 0) ? val : val / (p_oFac1 + p_oFac2);
			oLineVal = (int)(0.5 + val);

			if ((rLaserLine1In_Rank[x] == 0) || (rLaserLine2In_Rank[x] == 0))
			{
				rProfileOut_Data[x] = 0;
				continue;
			}

			rProfileOut_Data[x] = oLineVal;

			if (oLineVal > m_overlayMax) m_overlayMax = oLineVal;
			if (oLineVal < m_overlayMin) m_overlayMin = oLineVal;


		} // for
	}
} // extractLineProfile



} // namespace precitec
} // namespace filter
