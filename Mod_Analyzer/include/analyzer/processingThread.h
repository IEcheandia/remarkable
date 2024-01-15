/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		GM
 * 	@date		2017
 * 	@brief		Worker Thread for processing one run in the inspection graph
 */

#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Thread.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace precitec {
namespace analyzer {

/**
 * This class provides a long running thread which can take over specific
 * work tasks to perform the processing of an Image/Sample.
 *
 * It uses one thread which is started when constructing an instance of
 * ProcessingThread and gets terminated on deconstruction of the ProcessingThread.
 *
 * One can pass a work tass to the thread through scheduleWork. The thread used
 * by this class is either processing the work or is blocked on a wait condition.
 * It gets woken up whenever new work is available through scheduleWork.
 **/
class ProcessingThread
{
public:
	ProcessingThread();
	virtual ~ProcessingThread();

	/**
	 * Schedules the @p work to be processed next by the worker thread.
	 * Blocks until the current work got processed, but for maximum @p waitFor microseconds.
	 *
	 * The ProcessingThread takes ownership over the passed in @p work.
     *
     * @returns @c true if the work got scheduled, @c false on timeout.
	 **/
	bool scheduleWork(std::unique_ptr<Poco::Runnable> work, const std::chrono::microseconds &waitFor);

	/**
	 * Waits till one work finished
	 **/
	void join();
	
	void setCPUAffinity(size_t cpu);

    void setWorkDoneCallback(std::function<void()> &&callback)
    {
        m_callback = std::move(callback);
    }

    void startThread();

    void setNice(int nice)
    {
        m_nice = nice;
        requestChangeScheduler();
    }
    void setBatch(bool batch)
    {
        m_batch = batch;
        requestChangeScheduler();
    }

    void setRtPriority(bool enable, int priority = 0)
    {
        m_priority = priority;
        m_rtPrio = enable;
        requestChangeScheduler();
    }

    void setName(const std::string &name)
    {
        m_threadName = name;
    }

private:
	/**
	 * Method run in the worker thread through a RunnableAdapter.
	 * This method processes all work.
	 **/
	void run();

    void changeScheduler();
    void requestChangeScheduler();

	std::mutex m_mutex;
	Poco::Thread m_thread;
	Poco::RunnableAdapter<ProcessingThread> m_runnable;
	std::condition_variable m_condition;
	std::unique_ptr<Poco::Runnable> m_work;
	bool m_finishing;
    bool m_working = false;
    std::function<void()> m_callback;
    int m_nice = 0;
    int m_priority = 0;
    bool m_batch = false;
    bool m_rtPrio = false;
    bool m_needsChangeScheduler = false;
    std::string m_threadName;
};

}
}

#endif
