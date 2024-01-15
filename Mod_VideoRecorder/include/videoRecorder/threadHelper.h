/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		Martin Graesslin (GM)
 * 	@date		2017
 * 	@brief		Helper for setting thread nice
 */
#ifndef threadHelper_h
#define threadHelper_h

#include <stdint.h>

namespace precitec {
namespace vdr {

/**
 * Sets the niceness of the current thread to @p nice.
 * Only implemented on Linux using the sched_setattr syscall.
 * 
 * @returns 0 on success, -1 on failure
 **/
int setThreadNice(int32_t nice);

}
}

#endif
