/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		GM
 * 	@date		2017
 * 	@brief		Worker Thread for processing one run in the inspection graph
 */
#include "analyzer/processingThread.h"

#include <Poco/RunnableAdapter.h>
#include <Poco/ScopedLock.h>

#include <iostream>

#include <module/moduleLogger.h>

#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/types.h>

struct sched_attr {
	__u32 size;              /* Size of this structure */
	__u32 sched_policy;      /* Policy (SCHED_*) */
	__u64 sched_flags;       /* Flags */
	__s32 sched_nice;        /* Nice value (SCHED_OTHER, SCHED_BATCH) */
	__u32 sched_priority;    /* Static priority (SCHED_FIFO, SCHED_RR) */
	/**
	 * What follows are the deadline values, which are not of interest to us
	 **/
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
};

namespace precitec {
namespace analyzer {

ProcessingThread::ProcessingThread()
	: m_runnable(*this, &ProcessingThread::run)
	, m_work()
	, m_finishing(false)
{
}

ProcessingThread::~ProcessingThread()
{
	{
		// let the thread terminate by setting the finished flag and waking it up.
		std::unique_lock<std::mutex> lock{m_mutex};
		m_finishing = true;
		lock.unlock();
		m_condition.notify_all();
	}
	m_thread.join();
}

void ProcessingThread::startThread()
{
    m_thread.start(m_runnable);
}

bool ProcessingThread::scheduleWork(std::unique_ptr<Poco::Runnable> work, const std::chrono::microseconds &waitFor)
{
    const auto startTime{std::chrono::steady_clock::now()};
    std::unique_lock<std::mutex> lock{m_mutex};
    while (true)
    {
        if (!m_work)
        {
            m_work.swap(work);
            break;
        }
        const auto waited = std::chrono::steady_clock::now() - startTime;
        const auto remainingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(waitFor) - waited;
        if (remainingTime.count() < 0)
        {
            return false;
        }
        if (m_condition.wait_for(lock, remainingTime) == std::cv_status::timeout)
        {
            return false;
        }
    }
    lock.unlock();
    m_condition.notify_all();
    return true;
}

void ProcessingThread::join()
{
    std::unique_lock<std::mutex> lock{m_mutex};
    while (true)
    {
        // work == null && working -> currently working
        // work != null && !working -> work is scheduled
        // work != null && working -> work is scheduled and is working
        // work == null && !working -> neither work is scheduled nor working
        if (!m_work && !m_working)
        {
            break;
        }
        m_condition.wait(lock);
    }
}

void ProcessingThread::run()
{
    if (!m_threadName.empty())
    {
        prctl(PR_SET_NAME, m_threadName.c_str());
    }
    changeScheduler();
    std::unique_lock<std::mutex> lock{m_mutex};
    while (!m_finishing)
    {
        if (!m_work)
        {
            m_condition.wait(lock);
        }
        if (m_finishing)
        {
            break;
        }
        if (m_needsChangeScheduler)
        {
            changeScheduler();
        }
        if (!m_work)
        {
            continue;
        }
        m_working = true;
        auto work = std::move(m_work);
        lock.unlock();
        m_condition.notify_all();
        try {
            work->run();
        } catch(...)
        {
        }
        lock.lock();
        m_working = false;
        if (m_callback)
        {
            m_callback();
        }
        m_condition.notify_all();
    }
}

void ProcessingThread::changeScheduler()
{
    struct sched_attr attr;
    attr.size = sizeof(attr);
    attr.sched_policy = m_batch ? SCHED_BATCH : (m_rtPrio ? SCHED_RR : SCHED_OTHER);
    attr.sched_flags = 0;
    attr.sched_nice = (!m_batch && !m_rtPrio) ? m_nice : 0;
    attr.sched_priority = m_rtPrio ? m_priority : 0;
    attr.sched_runtime = 0;
    attr.sched_deadline = 0;
    attr.sched_period = 0;

    if (syscall(SYS_sched_setattr, 0, &attr, (unsigned int)0) != 0)
    {
        wmLog(eError, "Failed to adjust nice %s\n", strerror(errno));
    }
    m_needsChangeScheduler = false;
}

void ProcessingThread::requestChangeScheduler()
{
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_needsChangeScheduler = true;
    }
    m_condition.notify_all();
}


void ProcessingThread::setCPUAffinity(size_t cpu)
{
	// TODO: replace by Poco functionality once it's available
	// Poco develop branch has support since 158aaab1803d84f90b82bdf2488240be60b1e14d
	// but it is not yet in any release (at time of writing 1.8 is the most recent poco release)
#ifdef __linux__
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	if (pthread_setaffinity_np(m_thread.tid(), sizeof(cpuset), &cpuset) != 0)
	{
		std::cout << "Setting cpu affinity for Processing Thread failed" << std::endl;
	}
#endif
}

}
}
