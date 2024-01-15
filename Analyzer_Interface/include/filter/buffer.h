/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		A global buffer class - used by the filter graphs through the BufferRecorder and BufferPlayer filters to store and retrieve data.
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include "Analyzer_Interface.h"
#include "fliplib/Fliplib.h"
#include <geo/geo.h>
#include <geo/array.h>

class ContourBufferTest;
class BufferTest;

namespace precitec {
namespace filter {


/**
 * @brief A data buffer class - used by the filter graphs through the BufferRecorder and BufferPlayer filters to store and retrieve data.
 */
template<typename T >
class ANALYZER_INTERFACE_API TBuffer
{
	typedef std::tuple<unsigned int, unsigned int, unsigned int> buffer_key_t;	///< used by the array map for the keys

public:

	/**
	 * @brief CTor.
	 */
	TBuffer( );
	/**
	 * @brief DTor.
	 */
	~TBuffer( );

	/**
	 * @brief Does a specific buffer exist?
	 */
	bool exists( unsigned int p_oSlot, unsigned int p_oSeamSeries, unsigned int p_oSeam );

	/**
	 * @brief Initialize a buffer
	 */
	void init( unsigned int p_oSlot, unsigned int p_oSeamSeries, unsigned int p_oSeam, unsigned int p_oSize = 1000 );

	/**
	 * @brief Get an individual buffer.
	 */
	std::shared_ptr< T > get( unsigned int p_oSlot, unsigned int p_oSeamSeries, unsigned int p_oSeam );

protected:

	std::map< buffer_key_t, std::shared_ptr< T > > m_oArrays;
    friend ContourBufferTest;
    friend BufferTest;

};


//explicit instantation 
template class TBuffer<interface::GeoDoublearray>;
template class TBuffer<interface::GeoVecAnnotatedDPointarray>; 


typedef TBuffer<interface::GeoDoublearray> Buffer;
typedef TBuffer<interface::GeoVecAnnotatedDPointarray> ContourBuffer;

/**
 * @brief The buffer is a singleton and is only created on demand using this class.
 */
template<typename T >
class ANALYZER_INTERFACE_API TBufferSingleton
{
public:

    static TBuffer<T>& getInstanceData()
    {
        static TBuffer<T> g_oInstanceData;
        return g_oInstanceData;
    }
    static Buffer& getInstancePos()
    {
        static Buffer g_oInstancePos;
        return g_oInstancePos;
    }

private:

	TBufferSingleton() {};
	TBufferSingleton(TBufferSingleton const&);
	void operator=(TBufferSingleton const&);

};

//explicit instantation 
template class TBufferSingleton<interface::GeoDoublearray>;
template class TBufferSingleton<interface::GeoVecAnnotatedDPointarray>; 


typedef TBufferSingleton<interface::GeoDoublearray> BufferSingleton;
typedef TBufferSingleton<interface::GeoVecAnnotatedDPointarray> ContourBufferSingleton;



} // namespace filter
} // namespace precitec

#endif /* BUFFER_H_ */
