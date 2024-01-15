/** \file wmMPSCQueue.h
 *
 *  \author Andreas Beschorner
 *
 *  \date Created on: Nov 25, 2010
 *
 * \brief Implementation of a (threadsafe) MultiProducer-SingleConsumer intrusive template FIFO-queue with low wait/lock times.
 *
 * <b style="color:#cc0000">Dependencies:</b> wmAtomics.h
 */

#ifndef WMMPSCQUEUE_H_
#define WMMPSCQUEUE_H_

#include <inttypes.h>				//< for standard width int types (also called strong types)
#include <unistd.h>			//< for usleep()
#include "../wmAtomics/wmAtomics.h"

#include <cstdlib>


namespace precitec {
namespace utils {

//const int c_CachelineSize = 128;		// in bytes

/**
 * \class wmMPSCQueue
 *
 * \brief Implementation of a (threadsafe) MultiProducer-SingleConsumer intrusive template FIFO-queue with low wait/lock times.
 *
 * T may be and type that has a public T *next member pointer
 *
 * This template class implements a nearly wait/lock free MultiProducer-SingleConsumer *MPSC* Queue.
 * The data handling is intrusive for performance reasons.
 * Head (first) and tail (last) pointers are kept in different cache lines to prevent false sharing.
 *
 * The implementation follows Dmitriy Vyukov's (DV) suggestion, see
 * <a href="https://groups.google.com/forum/?fromgroups#searchin/lock-free/mpsc/lock-free/Vd9xuHrLggE/B9-URa3B37MJ">Vyukov, D., lock free mpsc queue (google groups).</a><br/>
 * DV does the same as Herb Sutter in
 * <a href="http://www.drdobbs.com/high-performance-computing/211601363">Sutter, Herb: A Generalized Concurrent Queue</a><br/>
 * by inserting a dummy head element "stub".
 *
 * Note that DV uses the terms tail and head the other way round compared to general usage; I therefore changed names to match Sutter's:
 * head (DV) ==> last ; tail (DV) ==> first
 *
 * Finally, concerning the cache alignment, check out
 * <a href="http://www.drdobbs.com/go-parallel/article/showArticle.jhtml;jsessionid=J5EQ5THOKEDV3QE1GHPCKH4ATMY32JVN?articleID=217500206&pgno=4">DrDobbs, Cache alignment ref. 1</a>
 * or
 * <a href="http://www.drdobbs.com/223100705">DrDobbs, Cache alignment ref. 2</a>
 * or
 * <a href="http://www.codeproject.com/KB/threads/FalseSharing.aspx?msg=3692741#xx3692741xx">Code Project, Cache alignment ref. 3,</a><br/>
 * where the first reference makes extensive use of C++0x [[ ... ]] instructions not yet available in our compilers.
 * Thus I implemented the performance crucial cache alignment by the gnu-gcc attribute as given below.
 * We might also try using the cpuid-Information about the real size of the cache line of the system.
 *
 * Implementation offers non-waiting and spinlocked pop
 * \warning Do NOT mix spinlocked pop and non-waiting pop until you know exactly what you are doing....
 *
 * \attention <small style="color:#aa0022"><b>In a discussion with the author he informed me on 2011/01/18 that any
 * access to the next pointers (direct or indirect) requires loadAcquire or storeRelease. This has even more importance
 * than locking the atomics ops, as memory can be invalidated if not considering the necessary barriers. Hence
 * the rather big amount of loadAcquire and storeRelease memory accesses. Still, this is way faster then using
 * a full barrier/ mutex lock for the complete processes of pushing and/or popping elements.</b></small>
 *
 */
template<typename T>
class wmMPSCQueue {
public:
	/** \fn wmMPSCQueue()
	 * Standard constructor
	 */
	wmMPSCQueue()
	{
		m_stub.next = 0;                                                         // dummy 1st element in queue for easier empty queue handling
		m_last = &m_stub;
		m_first = &m_stub;
		m_consumerLock = 0;                                                      // not locked by default
		m_size = 0;
	}

	/** \fn ~wmMPSCQueue()
	 * Virtual destructor. Deletes remaining items, assuming they will no longer be needed.
	 */
	virtual ~wmMPSCQueue()
	{
	  // delete list assuming its elements will not be used any longer
	  __attribute__((unused)) T* tmpNode;

	  while ( (m_size > 0) && (m_first != NULL) ) {
	    tmpNode = popSpinlock();
	  }
	}

	/** \fn push(T* node)
	 * Inserts a new (pointer to a) node. node must be created before calling this function.<br/>
	 * push is consumer blocking during an extremly short period of time.<br/>
	 * <b>Threadsafe!</b>
	 */
	void push(T* node)
	{
		// IN: node (must be allocated beforehand!)

		//node->next = NULL;                                                        // this can also be done on node creation; in here for safety reasons!
		storeRelease(&node->next, (T*)NULL);
		/* Insert new node via atomic xchg (m_last <- node, prevm_last <- m_last) plus prevm_last->next <- node.
		 * Locking (other producers and also consumer) is done implicitly by atomic op atomicXchgPtr32,
		 * that is: as this is a concurrent container, atomicXchPtr32/64 synchronizes internally.
		 */
		T* prevm_last = (T*)atomicXchgPtr32((volatile void**)&m_last, node);        // locking done within atomic OP. Options: 32bit, 64bit.
		atomicInc32(&m_size);
		/* This is the (only) point, where the producer blocks the consumer, as would be in a mutex based
		 * implementation. So if the producer thread bails out right here (for instance timout and thereby
		 * being kicked by OS), we have the problem of being stuck on the consumer side. This, however,
		 * is a problem in general.
		 * AFAICT a lock for producer exclusivity is not necessary here, as the locking atomic Xchg command
		 * internally already sort of synchronizes things. In a private discussion, DV confirmed this.
		 */
	//	prevm_last->next = node;                                             // now we publish to consumer that the queue has changed
		storeRelease(&prevm_last->next, node);
	}

	/** \fn T* pop()
	 * Waitfree; pops the head node from the queue.<br/>
	 * <b>Depending on the task, this is not threadsafe!</b>
	 */
	T* pop()	      // pop without consumer lock when queue is busy
	{
	  /* remember this is a FIFO-queue, thus we pop from the head, not from the tail */
	  T *start = m_first;
	  T *second = loadAcquire(&(start->next));
	  if (start == &m_stub) {                                                // -> m_first pop action since queue was empty
			if (second == 0) {
				return 0;                                                          // -> list empty
			}
			m_first = loadAcquire(&second);                                      // 2nd element will become new dummy by this
			start = loadAcquire(&second);                                        // (1) skip m_stub pointer, set m_first/start to 1st (real) el. in queue
			second = loadAcquire(&(second->next));                               // set second to second (real) element in queue
			// &m_stub is now "forgotten" and will be re-inserted when queue is popped empty, see below
		}
		if (second != 0) {                                                     // -> "ordinary" pop action
			m_first = loadAcquire(&second);                                      // move m_first pointer to 2nd real el. in queue and thus "pop" 1st
			atomicDec32(&m_size);

			return start;                                                        // return popped (real) 1st queue element
		}
		if (start == m_last) {                                                 // queue start == queue end, i.o.w. just one el. in queue
			push (&m_stub);                                                      // as the m_stub pointer is "lost" in (1), we have to re-push it
			atomicDec32(&m_size);
			second = loadAcquire(&(start->next));
			// now second should point to &m_stub, m_first and start to the only el. in queue
			if (second != 0) {                                                   // methinks this 'if' is not necessary, as second = &m_stub now!
				m_first = loadAcquire(&second);
				atomicDec32(&m_size);
				return start;
			}
		}
		return 0;                                                               // list empty OR, if m_first != m_last, the queue is in WAIT state
	}

	/** \fn T* popSpinlock()
	 * Spinlocked; pops the head node from the queue.<br/>
	 * <b>Threadsafe!</b>
	 */
	T* popSpinlock()    // pop including consumer lock (spin lock wait) when queue is busy
	{
	  while (atomicXchgInt32(&m_consumerLock, 1))
	    usleep(10);
	  T* ret = pop();
	  /* The following line is implemented as a "memory release" lock, so that a potential concurrent pop
	   * does not produce a RACE-condition via its atmicXchg-operation
	   */
	  atomicXchgInt32(&m_consumerLock, 0);                                   // Locked TestAndSet = Store release
	  return ret;
	}

	/** \fn bool isEmpty()
	 * Returns true if queue is empty, false otherwise
	 */
	bool isEmpty()
	{
		//return ( (m_first == &m_stub) && (m_first->next == 0) );
		return (loadAcquireInt32(&m_size) == 0);
	}

	/** \fn int size()
	 * Returns size (number of elements) of queue.
	 */
	volInt32 size()
	{
		return (loadAcquireInt32(&m_size));
	}

private:
	T volatile *m_last;// __attribute__( (aligned(128)) );      // aligned volatile pointer, necessary for atomic operation
	T *m_first;// __attribute__( (aligned(128)) );              // aligned volatile pointer, necessary for atomic operation
	T m_stub;// __attribute__( (aligned(128)) );                // aligned volatile pointer, necessary for atomic operation
	volInt32 m_consumerLock;                                    // lock variable for popSpinlock action, consumer), volatile for store release
	volInt32 m_size;
};

} // namespace utils
} // namespace precitec


#endif /* WMMPSCQUEUE_H_ */
