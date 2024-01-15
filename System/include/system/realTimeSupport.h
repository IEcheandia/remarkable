#pragma once

#include <cstdint>
#include <pthread.h>

namespace precitec
{
namespace system
{

enum class Priority : std::uint32_t
{
    None = 0,
    GraphProcessing = 1,
    Results = 5,
    Sensors = 10,
    InspectionControl = 20,
    EtherCATDependencies = 38,
    FieldBus = 50,
    FieldBusCyclicTask = 56,
    MaxPriority = FieldBusCyclicTask
};

/**
 * Sets the rlimit for the calling process to the maximum priority
 **/
void raiseRtPrioLimit();

/**
 * Makes the current thread realtime with SCHED_FIFO and SCHED_RESET_ON_FORK
 **/
void makeThreadRealTime(Priority priority);

/**
 * Sets the pthread_attr_t to ensure that the thread already starts in realtime mode
 **/
void makeThreadRealTime(Priority priority, pthread_attr_t* pthreadAttr);

/**
 * Overloaded for convenience.
 **/
void makeThreadRealTime(uint32_t priority);

}
}
