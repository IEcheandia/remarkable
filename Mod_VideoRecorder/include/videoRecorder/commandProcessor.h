/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Processes commands. A fifo queue manages commands. One worker thread executes the commands.
 */

#ifndef COMMANDPROCESSSOR_H_20121030_INCLUDE
#define COMMANDPROCESSSOR_H_20121030_INCLUDE


// local includes
#include "videoRecorder/baseCommand.h"
#include "module/moduleLogger.h"
// stl includes
#include <queue>
#include <memory>
#include <atomic>
// Poco includes
#include "Poco/Thread.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/RWLock.h"
#include "Poco/Semaphore.h"


namespace precitec {
namespace vdr {


/**
 * @brief	Manages and executes image sequence writing.
 */
class CommandProcessor {
public:
	/**
	 * @brief CTor.
	 * @param	p_oOsPriority				OS priorty that determines the priority of the worker thread.
	 * @param	p_oMaxQueueSize				Maximal size of command queue.
	 */
	CommandProcessor(int p_oOsPriority, std::size_t p_oMaxQueueSize);

    enum class EnqueueMode {
        FailOnFull,
        Force
    };
	/**
	 * @brief 								Insert command in job queue. If the queue is full, it skips the command and emmits a warning.
	 * @param	p_oFileCommand				Concrete command that defines an operation on a file. && forces move of unique pointer content.
	 * @return	void
	 */
	bool pushBack(upBaseCommand_t&& p_rFileCommand, EnqueueMode mode = EnqueueMode::FailOnFull);

	/**
	 * @brief	Returns wether the command qeue is not empty OR if a command is being processed.
	 * @return	bool						If the command qeue is not empty OR if a command is being processed.
	 */
	bool isBusy() const;

	/**
	 * @brief	Joins running thread, cleanup.
	 * @return void
	 */
	void uninitialize();



private:

	/**
	 * @brief 			Starts the thread that executes the commands.
	 * @return	void
	 */
	void startWorkerThread();

	/**
	 * @brief 			Executes file commands in a separate low-priority thread.
	 * @return	void
	 */
	void work();


	typedef Poco::RunnableAdapter<CommandProcessor>						CmdProcessor_Adapter_t;

	const int							m_oOsPriority;				///< OS priorty that determines the priority of the worker thread.
	const std::size_t 					m_oMaxQueueSize;			///< Maximal size of command queue.

	mutable Poco::RWLock 				m_oJobQueueMutex;			///< Mutex to protect the job queue.
	mutable Poco::Semaphore				m_oWorkerSema;				///< semaphore to signal image

	CmdProcessor_Adapter_t				m_oRunWork;					///< Thread adapter that executes the work method
	Poco::Thread						m_oWorker;					///< Thread that handles image writing from fifo at lower priority
	typedef std::queue<upBaseCommand_t>	commandQueue_t;
	commandQueue_t 						m_oJobQueue;				///< Internal work queue.
	std::atomic<bool>					m_oWorking;					///< if a command is being executed.
	std::atomic<bool>					m_oShutdown;				///< if shutdown state
}; // Writer



} // namespace vdr
} // namespace precitec

#endif // COMMANDPROCESSSOR_H_20121030_INCLUDE
