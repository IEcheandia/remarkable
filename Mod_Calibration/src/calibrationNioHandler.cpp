/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2013
 * 	@brief		NIO handler, derived from calibrationResultHandler. Sets an internal flag if a result is received and therefore the graph has reported an NIO.
 */

// project includes
#include <calibration/calibrationNioHandler.h>
#include <module/moduleLogger.h>

namespace precitec {
namespace calibration {


CalibrationNioHandler::CalibrationNioHandler() : CalibrationResultHandler(), m_bNIO(false)
{
} // CTor



CalibrationNioHandler::~CalibrationNioHandler()
{
	clear();

} // DTor



void CalibrationNioHandler::clear()
{
	CalibrationResultHandler::clear();
	m_bNIO = false;

} // clear



bool CalibrationNioHandler::isNIO()
{
	return m_bNIO;

} // isNIO



/*virtual*/ void CalibrationNioHandler::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE )
{
	wmLog( eDebug, "CalibrationNioHandler::proceed\n");

	m_bNIO = true;

	CalibrationResultHandler::proceed( p_pSender, p_rE );

} // proceed


} // namespace calibration
} // namespace precitec

