/**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Simple display filter for laser-line objects.
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "overlay/layerType.h"
#include <common/geoContext.h>
#include <filter/parameterEnums.h>		///< rank
#include <filter/algoArray.h>			///< algorithmic interface for class TArray
#include "system/typeTraits.h"
#include "module/moduleLogger.h"
// local includes
#include "lineWrite.h"
#include <fstream>
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec
{
using namespace interface;
using namespace image;
using namespace geo2d;
namespace filter
{

const std::string LineWrite::m_oFilterName 		= std::string("LineWrite");
const std::string LineWrite::PIPENAME_OUT1	= std::string("Line");


LineWrite::LineWrite() :
	TransformFilter( LineWrite::m_oFilterName, Poco::UUID{"190769c1-5032-4b23-a891-317e540a6bf8"} ),
	m_pPipeLineIn(NULL),
	m_pPipeLineOut(NULL),
	m_oStart(-1),
	m_oEnd(-1),
	m_curNumber(0),
	m_oOutputFolder("."),
	m_oOutputFilename("")
{
	m_pPipeLineOut = new SynchronePipe< GeoVecDoublearray >( this, LineWrite::PIPENAME_OUT1 );

	// Set default values of the parameters of the filter
	parameters_.add("Start", Parameter::TYPE_int, m_oStart);
	parameters_.add("End", Parameter::TYPE_int, m_oEnd);
	parameters_.add("OutputFolder", fliplib::Parameter::TYPE_string, m_oOutputFolder);
	parameters_.add("OutputFilename", fliplib::Parameter::TYPE_string, m_oOutputFilename);

    setInPipeConnectors({{Poco::UUID("6c2a6800-a0fb-4d03-bcb4-70e0f9f3fdbf"), m_pPipeLineIn, "Line", 0, ""}});
    setVariantID(Poco::UUID("515a6df7-1aa2-4fc8-87de-cd2c1c3fe9b0"));
} // LineWrite



LineWrite::~LineWrite()
{
	delete m_pPipeLineOut;

} // ~LineWrite



void LineWrite::setParameter()
{
	TransformFilter::setParameter();
	m_oStart = parameters_.getParameter("Start").convert<int>();
	m_oEnd = parameters_.getParameter("End").convert<int>();
	m_oOutputFolder = parameters_.getParameter("OutputFolder").convert<std::string>();
	m_oOutputFilename = parameters_.getParameter("OutputFilename").convert<std::string>();

	// sanity checks
	if (m_oEnd > 0 && m_oStart > m_oEnd)
		std::swap( m_oStart, m_oEnd );

} // setParameter



void LineWrite::write()
{

	//check output line max size
	size_t maxSize = 0;
	for (auto & line : m_oLineOut)
	{
		if (line.getData().size() > maxSize)
		{
			maxSize = line.getData().size();
		}
	}

	// Is there actually some data?
	if (maxSize == 0)
	{
		return;
	}

	// create outputfolder and filename
	Poco::File	oDestDir(m_oOutputFolder);
	if (!oDestDir.exists())
	{
		oDestDir.createDirectories();
	} // if
	std::ostringstream oFilename, oSliceNum;
	oSliceNum << std::setfill('0') << std::setw(3) << m_curNumber;
	oFilename << m_oOutputFolder << "/" << m_oOutputFilename << oSliceNum.str() << ".csv";
	std::ofstream resultFile;
	resultFile.open(oFilename.str().c_str());

	//write the header
	resultFile << "index,";
	for (unsigned int i = 0; i < m_oLineOut.size(); i++)
	{
		resultFile << "line" << i << ", rank " << i << ",";
	}
	resultFile << std::endl;

	// write the signal (2 columns for each line : y,rank)
	for (size_t i = 0; i < maxSize; i++)
	{
		resultFile << std::setfill('0') << std::setw(3) << i << ",";
		for (auto & line : m_oLineOut)
		{
			// Get output line and rank
			auto &rLineOut_Data = line.getData();
			auto &rLineOut_Rank = line.getRank();

			if (i < rLineOut_Data.size())
			{
				resultFile << rLineOut_Data[i] <<","<< rLineOut_Rank[i] << ",";
			}
			else
			{
				resultFile << " , ,";
			}
		}
		resultFile << std::endl; //end row
	}
	resultFile.close();

} // write



bool LineWrite::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineWrite::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor

	// Do we have an in-pipe?
	if (m_pPipeLineIn == NULL)
    {
        preSignalAction();
		return;
    }

	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	interface::SmpTrafo	oSpTrafo = rLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();
	m_curNumber = rLineIn.context().imageNumber();

	// some more sanity checks before we extract the data
	int oStart = m_oStart;
	int oEnd   = m_oEnd;

	if (rArrayIn.empty() || rArrayIn.front()/*draw first only*/.getData().empty())
	{
		wmLog(eDebug, "Input data not ok\n");
        preSignalAction();
		return;
	}

	// let's check what we should display
	if (m_oStart < 0)
		oStart = 0;
	if (m_oEnd < 0 || m_oEnd > (int)(rArrayIn.front()/*draw first only*/.getData().size()))
		oEnd = rArrayIn.front()/*draw first only*/.getData().size();


	// Extract the line
	//LineExtract::extract( rArrayIn, oStart, oEnd, m_oLineOut );

	const unsigned int	oNbLines = rArrayIn.size();
	m_oLineOut.resize( oNbLines ); // if the size of the output signal is not equal to the input line size, resize
	for ( unsigned int lineN = 0; lineN < oNbLines; ++lineN )
	{ // loop over N lines
		// get the direct references to the stl vectors
		auto& rLineOut_Data = m_oLineOut[lineN].getData();
		auto& rLineOut_Rank = m_oLineOut[lineN].getRank();
		const auto& rLineIn_Data = rArrayIn[lineN].getData();
		const auto& rLineIn_Rank = rArrayIn[lineN].getRank();

		// sanity check
		if ( oStart > (int(rLineIn_Data.size()) - 1) )
		{
			//input line is too short, output should be empty
			rLineOut_Data.clear();
			rLineOut_Rank.clear();
			continue;
		}

		// if the size of the output signal is not equal to the input line size, resize
		if ( rLineOut_Data.size() != (unsigned int) (oEnd - oStart) )
		{
			rLineOut_Data.resize( oEnd - oStart );
			rLineOut_Rank.resize( oEnd - oStart );
		}

		// copy
		copy( rLineIn_Data.begin() + oStart, rLineIn_Data.begin() + oEnd, rLineOut_Data.begin() );
		copy( rLineIn_Rank.begin() + oStart, rLineIn_Rank.begin() + oEnd, rLineOut_Rank.begin() );
	} // for

	write();

	// now adjust the context to reflect the new offset
	geo2d::Point oOffset( oStart, 0 );

	// Trafo ...
	LinearTrafo oSubTrafo(oOffset);
	// ... and generate a new context with the new trafo and the old meta information
	SmpImageContext pNewContext = new ImageContext(rLineIn.context(), oSubTrafo(oSpTrafo));
	// Now create a new byte array, put the global context into the resulting profile and copy the rank over
	const GeoVecDoublearray& rGeoProfile = GeoVecDoublearray(*pNewContext, m_oLineOut, AnalysisOK, rLineIn.rank());
	preSignalAction();  m_pPipeLineOut->signal( rGeoProfile );

} // proceed

} // namespace precitec
} // namespace filter
