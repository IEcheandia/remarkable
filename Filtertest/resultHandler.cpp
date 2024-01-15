/**
 *  @file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 *  @author		HS
 *  @date		2013
 *  @brief		The resultmanager is connected to the (invisible) out-pipes of the result filters.
 */

#include "resultHandler.h"
#include "event/results.h"
#include "module/moduleLogger.h"
#include "common/defines.h"
#include <fstream> 

using namespace fliplib;
namespace precitec {
	using namespace interface;
namespace filter {


ResultHandler::ResultHandler() :
	SinkFilter			        ( "Filtertest::resulthandler" ),
	m_WaitForResultsSema	    ( 0, 1e9 ),
    m_oResultCnt                ( 0 ),
    m_oNbResultsExpected        ( 0 )
{}


void ResultHandler::waitForResult(int p_oNbResultsExpected)
{
    m_oNbResultsExpected    =   p_oNbResultsExpected;

    if (m_oResultCnt >= m_oNbResultsExpected)
    {
        return;
    }

	m_WaitForResultsSema.wait();
}


std::size_t ResultHandler::getResultCnt() {
    return m_oResultCnt;
}



void ResultHandler::proceedGroup(const void* sender, PipeGroupEventArgs& e) 
{
    for (auto pInPipe : m_oInPipes)
    {
	    const auto& rResult    =   pInPipe->read(m_oCounter);

	    // Das hier konvertiert das Resultat in einen String. Im Augenblick habe ich dafuer keine Verwendung:
	    //std::string oString1 = rResult.getString();
	    //std::string oString2 = rResult.toString();

	    ResultType oType = rResult.resultType();

	    std::string oTypename = "Unknown";
	    if (oType == Convexity)  //  7
	    {
		    oTypename = "Convexity";
	    }
	    if (oType == Roundness)  //  8
	    {
		    oTypename = "Roundness";
	    }
	    if (oType == SideBurn)  //  11
	    {
		    oTypename = "Sideburn";
	    }
	    if (oType == CoordPositionX)   // 28
	    {
		    oTypename = "CoordPositionX";
	    }
	    if (oType == CoordPositionY)   // 29
	    {
		    oTypename = "CoordPositionY";
	    }
	    if (oType == CoordPositionZ)   // 30
	    {
		    oTypename = "CoordPositionZ";
	    }
	    if (oType == ScanTracker_Amplitude)   // 104
	    {
		    oTypename = "ScanTracker_Amplitude";
	    }
	    if (oType == ScanTracker_Offset)   // 105
	    {
		    oTypename = "ScanTracker_Offset";
	    }
	    if (oType == LaserControl_PowerOffset)   // 106
	    {
		    oTypename = "LaserControl_PowerOffset";
	    }
	    if (oType == Z_Collimator_Position)   // 107
	    {
		    oTypename = "Z_Collimator_Position";
	    }
        
        
        if (!m_oResultFolder.empty())
        {
            std::ofstream oStream(m_oResultFolder + "/"+  std::to_string(int(oType))+"result.csv",std::ios_base::app);
            
            if (m_oResultCnt == 0)
            {
                oStream << "imageNumber;position;resultType;resultSize;resultValue" << "\n";
            }
            
            std::vector<double> oVector = rResult.value();
            
            oStream << std::fixed << std::setprecision(4)
                << rResult.context().imageNumber() << ";"  << rResult.context().position() << ";"  << oType << ";" << oVector.size() << ";" ;
            for (int i = 0, end = std::min<int>(MAX_ELEMENTS_TO_EXPORT, oVector.size()); i < end; i++)
            {
                oStream << oVector[i] << ";";
            }
            oStream << "\n";
            oStream.close();
        }
        
    } // for



	//const auto oPos           =   rResult.context().position();
    //const auto oImgNbCurr     =   rResult.context().imageNumber();


    ++m_oResultCnt;

    if (m_oResultCnt >= m_oNbResultsExpected)
    {
	    m_WaitForResultsSema.set();
    }

    preSignalAction();    // unlocks proceed mutex
}

} // namespace filter
} // namespace precitec
