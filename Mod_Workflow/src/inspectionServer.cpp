/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR
 *  @date			2009
 *  @brief			Signal inspection setup and inspection states	
 */

#include "workflow/inspectionServer.h"

#include "system/tools.h"
#include "Poco/Net/SocketAddress.h"

#include "common/systemConfiguration.h"

namespace precitec {
namespace workflow { 

InspectionServer::InspectionServer( SmStateContext p_pStateContext) :
		m_pStateContext( p_pStateContext )		
{
}


void InspectionServer::startAutomaticmode( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo)
{
	try
	{
        if (SystemConfiguration::instance().getBool("ContinuouslyModeActive", false))
        {
            // bit 31 is used for indicating, that no hardware-parameter-set is processed and no prepositioning of the axes is done
            m_pStateContext->setNoHWParaAxis((p_oProductType & 0x80000000) ? true : false);
            m_pStateContext->inspectManager().setNoHWParaAxis((p_oProductType & 0x80000000) ? true : false);
            p_oProductType &= ~0x80000000;
        }
        
        m_pStateContext->startAuto( p_oProductType, p_oProductNr, p_rExtendedProductInfo );

        
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch
} // startAutomaticmode



void InspectionServer::stopAutomaticmode()
{
	try {
		m_pStateContext->stopAuto();
        

	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch
} // stopAutomaticmode



void InspectionServer::start( int p_oSeamNumber )
{
	try {
		m_pStateContext->setSeam( p_oSeamNumber );
		m_pStateContext->beginInspect( p_oSeamNumber );
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch
} // start



void InspectionServer::end( int p_oSeam )
{
	try {
		m_pStateContext->endInspect( );
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch
} // end



void InspectionServer::info( int p_oSeamSeries )
{
	try {
		m_pStateContext->setSeamseries( p_oSeamSeries );
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch
} // info



void InspectionServer::linelaser ( bool onoff )
{
    if (onoff == false)
    {
        // in case of continuous mode, switch off line lasers when this function is called
        if (SystemConfiguration::instance().getBool("ContinuouslyModeActive", false))
        {
            m_pStateContext->enableLineLasers( false );
        }
    }
} // linelaser



void InspectionServer::startCalibration()
{
} // startCalibration



void InspectionServer::stopCalibration()
{
} // stopCalibration



void InspectionServer::seamPreStart( int p_oSeamNumber )
{
	try {
		m_pStateContext->setSeam( p_oSeamNumber );
		m_pStateContext->seamPreStart( p_oSeamNumber );
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch
} // seamPreStart


} // workflow
} // precitec
