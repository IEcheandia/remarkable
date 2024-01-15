/** \file wmAtomics.h
 *     \author Andreas Beschorner
 *
 *     \date   Created on: Nov 25, 2010
 *
 *	\brief This header implements important atomic ops.
 *
 *    This header implements a subset of the atomic ops in gcc-C++-assembler. Due to Logger specs we do NOT need
 *    flexibility w.r.t to processors and OSses; as of 2010/11 the implementation thus focus on the 32bit and 64bit linux/QNX
 *    implementations using Intel64 (NOT IA64!) architecture (which the Core i7 CPU belongs to).
 *    Furthermore, no 8bit or 16bit operations are implemented.
 *
 *    Constant width integers are used to avoid confusions and incompatibilities concerning 32bit <-> 64bit and potentially undefined calling behaviour.
 *
 *    As a reference for gcc-assembly, check out <a href="http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html">gcc assembly</a>
 *
 */

// !!! This will NOT compile within Visual Studio without AT&T assembler switch during compilation !!!

#ifndef WMATOMIC_H_
#define WMATOMIC_H_

//#include <cstdint>									// for constant width integers
#include <inttypes.h>

namespace precitec {
namespace utils {

/*
 * Even though macros would prevent some code repetition here (32/64 bits), I refrain from using them.
 *
 * Named constraints do not allow identical constraint variables within both in- and outlist,
 * see respective methods. So instead we have to clobber where necessary (e.g. memory in cmpxchg)
 * and make use the + operand.
 *
 * The command implemented is always documented in Intel-, NOT in AT&T notation in contrast
 * to the implementation itself (thanks to gcc...).
 *
 * Implementations for xchg and cmpxchg encompass both variants, integers and pointers.
 * Add, sub, dec and inc are just implemented for integers. Xadd is not yet implemented due to the fact that
 * our (relatively old) gcc version does not support it correctly.
 *
 * References:
 * 		1) http://faydoc.tripod.com/cpu/add.htm (and other commands)
 * 		2) Intel Software Developer's Manual, Vol. 2A and 2B (Instruction Set References), 2010/09
 */

/// volatile int32_t type for atomic 32bit operations
typedef volatile int32_t volInt32;
/// volatile int64_t type for atomic 64bit operations
typedef volatile int64_t volInt64;
/// volatile void* pointer type for atomic pointer operations
typedef volatile void* volPtr;

// ----------------------------------------------------------------------------------

// LoadAcquire and StoreRelease, memory-barrier based threadsafe load/ store of pointers
/// LoadAcquire, load with memory barrier.
template <typename T>
T* loadAcquire(T** a)
{
	T* v = *(T* volatile*)a;
	__asm __volatile ("" ::: "memory");
	return v;
}

/// StoreRelease, store with memory barrier.
template <typename T>
void storeRelease(T** a, T* v)
{
	__asm __volatile ("" ::: "memory");
	*(T* volatile*)a = v;
}

/// LoadAcquire for volatile 32 bit integers (parameter).
static inline volInt32 loadAcquireInt32(volInt32 *a)
{
	volInt32 v = (*a);
	__asm __volatile ("" ::: "memory");
	return v;
}

/// LoadAcquire for volatile 32 bit integers (value).
static inline volInt32 loadAcquireInt32(volInt32 a)
{
	volInt32 v = *(&a);
	__asm __volatile ("" ::: "memory");
	return v;
}

/// StoreRelease for volatile 32 bit integers.
static inline void storeReleaseInt32(volInt32* a, volInt32 v)
{
	__asm __volatile ("" ::: "memory");
	*a = (volInt32)v;
}

// ---------------------------------------------------------------------------------
// BB is 32 or 64 (number of bits)

/* inc mBB
 *
 * OUT:			dest++
 *
 * Complexity:	3 clock cycles (memory inc)
 * Pipes:		Pairable in both U and V pipe
 *
 */

/// Locked atomic inc m32.
static inline void atomicInc32(volInt32 *dest) {
    __atomic_fetch_add(dest, 1, __ATOMIC_SEQ_CST);
}


/* dec mBB
 *
 * OUT:			dest--
 *
 * Complexity:	3 clock cycles (memory dec)
 * Pipes:		Pairable in both U and V pipe
 *
 */

/// Locked atomic dec m32.
static inline void atomicDec32(volInt32 *dest) {
    __atomic_fetch_sub(dest, 1, __ATOMIC_SEQ_CST);
}

/** xchg mBB, rBB
 *
 * OUT:			val1 <-> val2
 *
 * Complexity:	3 clock cycles
 * Pipes:		Not pairable
 *
 * NOTE:	As of 2011/01, if a memory operand is referenced, xchg is automatically a locked operation,
 * 			whether the LOCK prefix is used or not. However, as we never know how things might change,
 * 			the prefix is included in this implementation.
 *
 *	We read and write from and to both parameters, which is reflected in the in- and outlist.
 *  Memory will be changed, as one of the references is a memory operand. Thus, we clobber it.
 *
 */

/// Locked atomic xchg m32, r32 for pointers/ references.
static inline void* atomicXchgPtr32(volPtr *valMem, void *valReg) {
	return (void*)__atomic_exchange_n(valMem, valReg, __ATOMIC_SEQ_CST);
}

/// Locked atomic xchg m32, r32 for 32bit integers.
static inline int32_t atomicXchgInt32(volInt32 *valMem, int32_t valReg)
{
	return __atomic_exchange_n(valMem, valReg, __ATOMIC_SEQ_CST);
}

/*cmpxchg mBB, rBB -- also known as (CAS) --
 *
 * Params:		mem, reg; eax (implicitly)
 * OUT:			mem==eax: {mem <- reg, set ZF} else {eax <- mem, clear ZF}.
 *
 * Complexity:	6 clock cycles
 * Pipes:		Not pairable
 *
 *	We read and write from and to both parameters, which is reflected in the in- and outlist.
 *  Memory will be changed, as one of the references is a memory operand. Thus, we clobber it.
 * 
 * Returns the value before the exchange.
 *
 */

/// Locked atomic cmpxchg m32, r32 for 32bit integers.
static inline int32_t atomicCASInt32(volInt32 *ptr, int32_t vOld, int32_t vNew)
{
	__atomic_compare_exchange(ptr, &vOld, &vNew, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return vOld;
}

// ----------
/* -- xadd --
 *
 * OUT:			dest <- source+dest
 *
 * Complexity:	4 clock cycles
 * Pipes:		Not pairable
 *
 *	dest 	is modified by the xadd command, so we have to clobber the memory. Furthermore,
 *		 	we change dest in the function, so both dest and source need to be in both in-
 *		 	and outlist, which again is not possible with named constraints...
 *	source	is the register variable.
 *
 *	Returns: addend valReg, so the old value can still be computed via valMem->val-valReg
 */
/* not yet included due to some strange gcc operand problems with the old gcc version... */
//
//static inline int32_t atomicXaddInt32(volInt32 *valMem, int32_t valReg) {
	// IMPLEMENTS:	xadd r32, m32
	//__asm__ __volatile__(
    //		"lock; xaddl %[dest], %[source]"
    //		: [source] "=r"(valReg)//, [dest] "+m"(*valMem)								/* out */
    //		: "[source]""0"(valReg), [dest] "m"(*valMem)//, 	/* in */
    //		: "memory"										/* clobber */
    //		);
    //return valReg;
//}


//static inline int64_t atomicXaddInt64(volInt64 *valMem, int64_t valReg) {
	// IMPLEMENTS:	xadd r64, m64
  //  __asm__ __volatile__(
    //		"lock; xadd %[source], %[dest]"
    	//	: [source] "=r"(valReg)								/* out */
    	//	: "[source]" "0"(valReg), [dest] "m"(*valMem)	/* in */
    	//	: "memory", "cc"											/* clobber */
    	//	);
    //return valReg;
//}

} // namespace utils
} // namespace precitec

#endif /* WMATOMIC_H_ */
