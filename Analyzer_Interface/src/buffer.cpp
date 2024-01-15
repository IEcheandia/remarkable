/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		A global buffer class - used by the filter graphs through the BufferRecorder and BufferPlayer filters to store and retrieve data.
 */

#include <filter/buffer.h>
#include <module/moduleLogger.h>

namespace precitec {
namespace filter {

    
template<typename T>
TBuffer<T>::TBuffer( )
{
} // CTor


template<typename T>
TBuffer<T>::~TBuffer( )
{
} // DTor


template<typename T>
bool TBuffer<T>::exists( unsigned int p_oSlot, unsigned int p_oSeamSeries, unsigned int p_oSeam )
{
	try
	{
		std::shared_ptr< T > pSp = m_oArrays.at( std::make_tuple( p_oSlot, p_oSeamSeries, p_oSeam ) );

	} catch( ... )
	{
		return false;
	}
	return true;

} // exists


template<>
void TBuffer<interface::GeoDoublearray>::init( unsigned int p_oSlot, unsigned int p_oSeamSeries, unsigned int p_oSeam, unsigned int p_oSize )
{
	wmLog( eDebug, "Buffer::init - (%d,%d,%d,%d)\n", p_oSlot, p_oSeamSeries, p_oSeam, p_oSize );

	// now create the array and store it
	auto pSp = std::make_shared< interface::GeoDoublearray >();
	// buffer_key_t
	m_oArrays[ std::make_tuple( p_oSlot, p_oSeamSeries, p_oSeam ) ] = pSp;
	// resize the array
	m_oArrays[ std::make_tuple( p_oSlot, p_oSeamSeries, p_oSeam ) ]->ref().assign( p_oSize );

} // init


template<>
void TBuffer<interface::GeoVecAnnotatedDPointarray>::init( unsigned int p_oSlot, unsigned int p_oSeamSeries, unsigned int p_oSeam, unsigned int p_oSize )
{
	wmLog( eDebug, "Buffer::init - (%d,%d,%d,%d)\n", p_oSlot, p_oSeamSeries, p_oSeam, p_oSize );

	// now create the array and store it
	auto pSp = std::make_shared<interface::GeoVecAnnotatedDPointarray >();
	// buffer_key_t
	m_oArrays[ std::make_tuple( p_oSlot, p_oSeamSeries, p_oSeam ) ] = pSp;
	// resize the array
    m_oArrays[ std::make_tuple( p_oSlot, p_oSeamSeries, p_oSeam ) ]->ref().assign( p_oSize, geo2d::AnnotatedDPointarray{} );

} // init

template<typename T>
std::shared_ptr< T > TBuffer<T>::get( unsigned int p_oSlot, unsigned int p_oSeamSeries, unsigned int p_oSeam )
{
	std::shared_ptr< T > pBuffer;

	try
	{
		pBuffer = m_oArrays.at( std::make_tuple( p_oSlot, p_oSeamSeries, p_oSeam ) );

	} catch( ... )
	{
		wmLog( eDebug, "Buffer::get() - Cannot find buffer for (%d, %d, %d)\n", p_oSlot, p_oSeamSeries, p_oSeam );
		wmLog( eDebug, "Buffer::get() - Will create new buffer ... \n");

		init( p_oSlot, p_oSeamSeries, p_oSeam );
		pBuffer = m_oArrays[ std::make_tuple( p_oSlot, p_oSeamSeries, p_oSeam ) ];

	}

	return pBuffer;

} // get


} // namespace filter
} // namespace precitec
