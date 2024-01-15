/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Processes commands. A fifo queue manages commands. One worker thread executes the commands.
 */

// local includes
#include "videoRecorder/commandProcessor.h"
//#include "videoRecorder/fileCommand.h" // for debugging of commands only
#include "videoRecorder/literal.h"
#include "videoRecorder/threadHelper.h"
//#include "system/timer.h" // for debugging
// stl includes
#include <sstream>
#undef min
#undef max
#include <limits>
#include <sys/prctl.h>

namespace precitec {
	using namespace Poco;
namespace vdr {


class WriteImageCmd;


CommandProcessor::CommandProcessor(int p_oOsPriority, std::size_t p_oMaxQueueSize) :
		m_oOsPriority				( p_oOsPriority ),
		m_oMaxQueueSize				( p_oMaxQueueSize ),
		m_oWorkerSema				( 0, std::numeric_limits<int>::max() ),
		m_oRunWork					( *this, &CommandProcessor::work ), // Thread adapter that executes the work method
		m_oWorker					( g_oCommandProcessorThread ),
		m_oWorking					( false ),
		m_oShutdown					( false )
{
	startWorkerThread();
} // CommandProcessor



bool CommandProcessor::pushBack(upBaseCommand_t&& p_rFileCommand, EnqueueMode mode) {
	ScopedWriteRWLock	oWriteLock(m_oJobQueueMutex);
//#ifndef NDEBUG
//			if (dynamic_cast<DeleteRecursivelyCmd*>(p_rFileCommand.get()) != nullptr) {
//				wmLog(eDebug, "IN: DeleteRecursivelyCmd\n");
//			}
//			else if (dynamic_cast<WriteConfCmd*>(p_rFileCommand.get()) != nullptr) {
//				wmLog(eDebug, "IN: WriteConfCmd\n");
//			}
//			else if (dynamic_cast<CreateDirsCmd*>(p_rFileCommand.get()) != nullptr) {
//				wmLog(eDebug, "IN: CreateDirsCmd\n");
//			}	// Queue is full. Return insertion error.
//			else if (dynamic_cast<RenameCmd*>(p_rFileCommand.get()) != nullptr) {
//				wmLog(eDebug, "IN: RenameCmd\n");
//			}
//			else if (dynamic_cast<WriteImageCmd*>(p_rFileCommand.get()) != nullptr) {
//				wmLog(eDebug, "IN: WriteImageCmd\n");
//			}
//			else if (dynamic_cast<ProcessRecordCmd*>(p_rFileCommand.get()) != nullptr) {
//				wmLog(eDebug, "IN: ProcessRecordCmd\n");
//			}
//			else {
//				wmLog(eDebug, "IN:Unknown Command\n");
//			}
//#endif // #ifndef NDEBUG

	const auto oJobQueueSize	= m_oJobQueue.size();
	const bool oQueueIsFull		= oJobQueueSize >= m_oMaxQueueSize;
	if (oQueueIsFull == true && mode == EnqueueMode::FailOnFull) {	//maximum queue size reached
		wmLog(eDebug, "Command discarded. Maximum queue size reached (%u).\n", m_oMaxQueueSize);
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, "Maximum queue size reached.");
		return false;
	} // if

	m_oJobQueue.push(std::move(p_rFileCommand)); 	// enqueue job
	m_oWorkerSema.set(); // signal new data. This must happen after queue insertion. Else the consumer thread will shutdown immediatly.

	if (oJobQueueSize != 0)
	{
		if ((oJobQueueSize % 20 == 0) && oJobQueueSize < 100)
		{
			wmLog(eDebug, "High job queue size (%u).\n", oJobQueueSize);
		}
		else if (oJobQueueSize % 100 == 0)
		{
			wmLog(eDebug, "WARNING: Very high job queue size (%u).\n", oJobQueueSize);
		}
	}

	return true;
} // pushBack



bool CommandProcessor::isBusy() const {
	ScopedReadRWLock	oReadLock(m_oJobQueueMutex);
	return (m_oJobQueue.empty() == false) || (m_oWorking == true);
} // isBusy



void CommandProcessor::uninitialize() {

	m_oShutdown = true; // Set shutdown state. Image writer thread will shutdown as soon as image queues is empty.
	m_oWorkerSema.set(); // post sema. Image writer thread will shutdown as soon as image queues is empty.

	if (m_oWorker.isRunning() == true) {
		wmLog(eDebug, "%s: Shutdown as soon as image/command queue is empty...\n", __FUNCTION__);
		const bool oJoinedWorker	( m_oWorker.tryJoin(g_oJoinCmdProcTimespan) ); // wait for thread to terminate
		if (oJoinedWorker == false) {
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << "\t: Failed to join thread '" << m_oWorker.name() << "'.\n";
			wmLog(eWarning, oMsg.str());
		} // if
	} // if
} // uninitialize



void CommandProcessor::startWorkerThread() {
	if (m_oWorker.isRunning() == false) {
		m_oWorker.start		(m_oRunWork);	// start thread
		m_oWorker.setOSPriority(m_oOsPriority);
		std::ostringstream oMsg;
//		oMsg << __FUNCTION__ << ": Thread " << m_oWorker.name() <<
//			" started at QNX priority " << m_oWorker.getOSPriority() << ".\n";
//		wmLog(eDebug, oMsg.str());
	}
	else {
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ": Thread " << m_oWorker.name() <<
			" already running at QNX priority " << m_oWorker.getOSPriority() << ".\n";
		wmLog(eDebug, oMsg.str());
	} // else
} // startWorkerThread



void CommandProcessor::work() {
    prctl(PR_SET_NAME, "CommandProcessor");
	setThreadNice(1);
	
	try {
		bool	oIsJobQueueEmpty	(true);
		std::ostringstream oMsg;
		//std::ostringstream oTime;
		while(true) {
			m_oWorkerSema.wait(); // wait for insertion signaled. Or extra signal in shutdown case.

			{
				ScopedReadRWLock	oReadLock(m_oJobQueueMutex);
				oIsJobQueueEmpty	= m_oJobQueue.empty(); // get current queue state
			}
			if (m_oShutdown == true && oIsJobQueueEmpty == true) {
				break; // shutdown
			}
			else {
				m_oWorking = true;
			} // else

			upBaseCommand_t			oCommand;
			{
				ScopedWriteRWLock	oWriteLock(m_oJobQueueMutex); // prevents push on write queue before pop of current
				oCommand	= std::move( m_oJobQueue.front() );
				m_oJobQueue.pop();
				//wmLog(eDebug, "m_oJobQueue.size() %i\n", m_oJobQueue.size()); // debug
			}

// debug - log which command is processed

//#ifndef NDEBUG
//			if (dynamic_cast<DeleteRecursivelyCmd*>(oCommand.get()) != nullptr) {
//				wmLog(eDebug, "OUT: DeleteRecursivelyCmd\n");
//			}
//			else if (dynamic_cast<WriteConfCmd*>(oCommand.get()) != nullptr) {
//				wmLog(eDebug, "OUT: WriteConfCmd\n");
//			}
//			else if (dynamic_cast<CreateDirsCmd*>(oCommand.get()) != nullptr) {
//				wmLog(eDebug, "OUT: CreateDirsCmd\n");
//			}	// Queue is full. Return insertion error.
//			else if (dynamic_cast<RenameCmd*>(oCommand.get()) != nullptr) {
//				wmLog(eDebug, "OUT: RenameCmd\n");
//			}
//			else if (dynamic_cast<WriteImageCmd*>(oCommand.get()) != nullptr) {
//				wmLog(eDebug, "OUT: WriteImageCmd\n");
//			}
//			else if (dynamic_cast<ProcessRecordCmd*>(oCommand.get()) != nullptr) {
//				wmLog(eDebug, "OUT: ProcessRecordCmd\n");
//			}
//			else {
//				wmLog(eDebug, "OUT:Unknown Command\n");
//			}
//#endif // #ifndef NDEBUG

			try {
				//const system::ScopedTimer		oTimer(__FUNCTION__, oTime); // debug info
				oCommand->execute();
			} // try
			catch(const Exception &p_rException) {
				std::ostringstream oMsg;
				oMsg  << p_rException.what() << " - " << p_rException.message() << "\n";
				wmLog(eDebug, oMsg.str()); // file command log provides a localized warning
			} // catch
			m_oWorking = false;

			//wmLog(eDebug, "%s\n", oTime.str().c_str()); // debug
			//oTime.str(""); // debug
		} // while
		oMsg.str("");
		oMsg << "-- Leaving thread TID " << m_oWorker.currentTid() << " '" << m_oWorker.name() << "'. --\n";
		wmLog(eDebug, oMsg.str());
	} // try
	catch(const Exception &p_rException) {
		std::ostringstream oMsg;
		oMsg  << __FUNCTION__ << " " << p_rException.what() << " - " << p_rException.message() << "\n";
		wmLog(eError, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcException", "An error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, p_rException.message().c_str());
	} // catch
	catch(const std::exception &p_rException) {
		std::ostringstream oMsg;
		oMsg  << __FUNCTION__ << " " << p_rException.what() << "\n";
		wmLog(eError, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcException", "An error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
	} // catch
	catch(...) {
		std::ostringstream oMsg;
		oMsg  << __FUNCTION__ << " Unknown exception encountered.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcException", "An error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
	} // catch
} // work


} // namespace vdr
} // namespace precitec
