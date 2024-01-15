/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		Martin Graesslin (GM)
 * 	@date		2017
 * 	@brief		Helper for setting thread nice
 */
#include "videoRecorder/threadHelper.h"

#ifdef __linux__
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <linux/types.h>
#endif


namespace precitec {
namespace vdr {

#ifdef __linux__
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
#endif

int setThreadNice(int32_t nice)
{
#ifdef __linux__
	struct sched_attr attr;
	attr.size = sizeof(attr);
	attr.sched_policy = SCHED_OTHER;
	attr.sched_flags = 0;
	attr.sched_nice = nice;
	attr.sched_priority = 0;
	attr.sched_runtime = 0;
	attr.sched_deadline = 0;
	attr.sched_period = 0;
	
	return syscall(SYS_sched_setattr, 0, &attr, (unsigned int)0);
#else
	return -1;
#endif
}

}
}
