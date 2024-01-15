/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), Andreas Beschorner (AB)
 * 	@date		2013
 * 	@brief		NIO Handler similar to the analyzer nioHandler.cpp, but without sending results to windows and some other changes.
 */

// project includes
#include <calibration/calibrationResultHandler.h>
#include <event/results.proxy.h>
#include <event/results.interface.h>
#include <module/moduleLogger.h>
// clib includes
#include <cstdio>

using namespace fliplib;

namespace precitec {
	using namespace interface;
	using namespace system;
namespace calibration {


CalibrationResultHandler::CalibrationResultHandler() : SinkFilter("precitec::analyzer::CalibrationResultHandler")
{
} // CTor



CalibrationResultHandler::~CalibrationResultHandler()
{
	clear();

} // DTor



void CalibrationResultHandler::clear()
{
	for (unsigned int i=0; i < m_oResults.size(); ++i)
	{
		delete m_oResults[i];
	}
	m_oResults.clear();

} // clear



void CalibrationResultHandler::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{

    for (auto pInPipe : m_oInPipes)
    {
#ifndef NDEBUG
    	wmLog( eDebug, "CalibrationResultHandler::proceed[%i]\n", m_oCounter);
#endif

		interface::ResultArgs *oRes = new interface::ResultArgs(pInPipe->read(m_oCounter));
		m_oResults.push_back( oRes );
    }

	preSignalAction();
} // proceed



std::vector< interface::ResultArgs* > CalibrationResultHandler::getResults()
{
#ifndef NDEBUG
	wmLog( eDebug, "CalibrationResultHandler::getResult()\n");
#endif
	return m_oResults;

} // get results


} // namespace calibration
} // namespace precitec
