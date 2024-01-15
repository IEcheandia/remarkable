/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Wolfgang Reichl (WoR), SB, HS
 * 	@date		2016
 * 	@brief		Shared-memory buffer for event interfaces.
 */

// stdlib includes
//#include <thread>	// not yet supported by gcc 4.6
// clib
#include <cstdint>
#include <cassert>

// Poco includes
#include "Poco/Thread.h"

// WM includes
#include "system/sharedMem.h"
#include "module/moduleLogger.h"
#include "system/exception.h"
#include "system/shMemRingAllocator.h"

#include <thread>


namespace precitec {
namespace system {
namespace message {

struct ShMemRingAllocator::Header
{
	std::int32_t	m_oLockCount;	///< 0 = frei, 1..n n-Fach gelockt
};



ShMemRingAllocator::ShMemRingAllocator( std::string p_oName, SharedMem const &p_rShMem, int p_oPayloadSize)
	:
	m_pShMemBegin		( reinterpret_cast<char*>(p_rShMem.begin()) ),
	m_pShMemEnd			( m_pShMemBegin + p_rShMem.size() ),
	m_oShMemSize		( p_rShMem.size() ),
	m_oPayloadSize		( p_oPayloadSize ),
	m_oBlockSize		( headerSize() + m_oPayloadSize ),
	m_oName				( p_oName )
{
	assert(m_oPayloadSize > 0);
	init();
}



/*static*/ int ShMemRingAllocator::headerSize() { return sizeof(Header); }



char* ShMemRingAllocator::allocBlock(int numReaders)
{
	assert(numReaders > 0);

	auto		oBlockedCnt			=	0;
	auto 		oFreeElementFound 	=	false;
	auto 		pCurrHeader			=	(Header*)nullptr;

	// wir suchen nach einem freien Block (erstes Element sollte eigentlich idr direkt frei sein)
	//
	for (auto pCurrHeaderRaw = m_pShMemBegin; pCurrHeaderRaw < m_pShMemEnd; pCurrHeaderRaw += m_oBlockSize)
	{
		// no free element found beyond half of buffer?
		//
		if (pCurrHeaderRaw >= m_pShMemBegin + int( 0.5 * m_oShMemSize ) )
		{
			++oBlockedCnt;

            std::this_thread::yield();
		} // if

		// ist das Element frei?
		pCurrHeader = memPosToHeader(pCurrHeaderRaw);

#if defined __QNX__ || defined __linux__
		const auto oCasOk	=	__sync_bool_compare_and_swap (&pCurrHeader->m_oLockCount, 0/*free*/, numReaders);
#else
        const auto oCasOk	= false;
        throw std::runtime_error{ std::string{ __FUNCTION__ } + "NOT IMPLEMENTED" };
#endif // #ifdef __QNX__||__linux__

		if (oCasOk == true)
		{
			oFreeElementFound	=	true;
			break;
		}
	} // for

	if ( oBlockedCnt > 0 && oFreeElementFound == true )
	{
		auto oNumBlk = (int)((m_oShMemSize / m_oBlockSize) * 0.5);
		wmLog( eError, "interface %s must wait (fill level: %i / %i).\n", m_oName.c_str(), oBlockedCnt + oNumBlk, oNumBlk * 2 );
	}

	// if no free element was found, log and throw or return nullptr
	//
	if ( oFreeElementFound == false )
	{
        if (!m_allocFailed)
        {
            wmFatal( eInternalError, "QnxMsg.Fatal.ShMemRingAlloc", "allocBlock() no free element for interface %s.\n", m_oName.c_str() );
            m_allocFailed = true;
        }

		throw AllocFailedException{m_oName};

		/*return nullptr;*/
	} // if

	const auto pPayloadMemPos	=	headerToMemPos(pCurrHeader) + headerSize();
    m_allocFailed = false;

//	if (m_oName == "Sensor")
//		wmLog(eWarning, "DEV MSG - ALLOC(%i) in %s #%i\n", numReaders, m_oName.c_str(), memPosToShMenOffset(pPayloadMemPos - headerSize()) / m_oBlockSize);

	return pPayloadMemPos;
} // allocBlock



void ShMemRingAllocator::freeBlock(char* p_pPayloadMemPos)
{
	auto 		pHeader 	= 	memPosToHeader(p_pPayloadMemPos - headerSize());
#if defined __QNX__ || defined __linux__
	const auto	oRemaining	=	__sync_sub_and_fetch (&pHeader->m_oLockCount, 1);
#else
        const auto oRemaining	= -1;
        throw std::runtime_error{ std::string{ __FUNCTION__ } + "NOT IMPLEMENTED" };
#endif // #ifdef __QNX__||__linux__

//	if (m_oName == "Sensor")
//		wmLog(eWarning, "DEV MSG - FREE(%i) in %s #%i\n", oRemaining, m_oName.c_str(), memPosToShMenOffset(p_pPayloadMemPos - headerSize()) / m_oBlockSize);

	assert(oRemaining >= 0);
	if (oRemaining < 0)
	{
		wmLog( eError, "LOCK COUNT CORRUPTION for interface  %s.\n", m_oName.c_str() );
		throw std::runtime_error{ std::string{ "LOCK COUNT CORRUPTION for interface  " } + m_oName + ".\n" };
	}
} // freeBlock



void ShMemRingAllocator::init()
{
	for (auto pCurrHeaderRaw = m_pShMemBegin; pCurrHeaderRaw < m_pShMemEnd; pCurrHeaderRaw += m_oBlockSize)
	{
		memPosToHeader(pCurrHeaderRaw)->m_oLockCount	= 0;
	}
} // init



inline char* ShMemRingAllocator::headerToMemPos(Header* p_pHeader) const
{	return reinterpret_cast<char*>(p_pHeader);
}



inline ShMemRingAllocator::Header* ShMemRingAllocator::memPosToHeader(char* p_pMemPos) const
{	return  reinterpret_cast<Header*>(p_pMemPos);
}

} // namespace message
} // namespace system
} // namespace precitec

