
/**
 *	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2010-2011
 *  @brief			Managed data container for 1d data.
 */


#ifndef ARRAY_H
#define ARRAY_H


#include <vector>					      ///< vector
#include <utility>					      ///< fill
#include <cassert>					      ///< assert
#include <tuple>					      ///< tuple
#include <sstream>					      ///< ostringstream

#include "system/types.h"			      ///< byte type

#include "filter/parameterEnums.h"	      ///< enum ValueRankType
#include "geo/point.h"				      ///< Point
#include "geo/blob.h"				      ///< Blob
#include "geo/seamfinding.h"		      ///< SeamFinding
#include "geo/poorPenetrationCandidate.h" ///< PoorPenetration
#include "geo/houghPPCandidate.h"         ///< Hough PoorPenetration
#include "geo/startEndInfo.h"             ///< StartEndInfo
#include "geo/surfaceInfo.h"              ///< SurfaceInfo
#include "geo/lineModel.h"

namespace precitec
{
namespace geo2d
{

/**
 * @brief	Managed data container for 1d data.
 * @details	The data array is hold as a std::vector<T>.
 *			The corresponding rank array is hold as a std::vector<int>.
 *			Synchronisation of array sizes is up to user. Thus, constant acces time can be maintained.
 *			Compiler-defined copy-ctor, assignment op and dtor working properly and intended to be used.
 *			Used as Laserline class with type int. Used in seam search as seam contour container. Used in positionLowPass as point container.
 */
template <typename T>
class TArray
{
public:
	/// construction, destruction

	/** 
	 *	@brief	std constructor. allocates a vector and its rank of given length, zero initialisation by default, bad rank initialisation
	 *	@param	p_oSize			Array length for both, data and its rank. Optional.
	 *	@param	p_oVal			Value to be assigned.
	 *	@param	p_oRank			Rank to be assigned.
	 *	@return	-
	 *	@sa -
	*/
	explicit TArray(std::size_t p_oSize = 0, T p_oVal = T(), int p_oRank = filter::eRankMin) :
		m_oData( p_oSize, p_oVal ),
		m_oRank( p_oSize, p_oRank ) {
	}



	/** 
	 *	@brief	const ref accessor via getter
	 *	@return	const std::vector<T> &	Reference to data vector.		
	 *	@sa std::vector
	*/
	const std::vector<T> &getData () const {
		// check consistency
		assert(m_oRank.size() == m_oData.size() && "GETDATA() Inconsistency between rank and data. Size must be equal.");
		return m_oData;
	}



	/** 
	 *	@brief	ref accessor via getter
	 *	@return	const std::vector<T> &	Reference to data vector.		
	 *	@sa std::vector
	*/
	std::vector<T> &getData () { return m_oData; }



	/** 
	 *	@brief	const ref accessor via getter
	 *	@return	const std::vector<T> &	Reference to rank vector.		
	 *	@sa std::vector
	*/
	const std::vector<int> &getRank () const {
		// check consistency
		assert(m_oRank.size() == m_oData.size() && "GETRANK() Inconsistency between rank and data. Size must be equal.");
		return m_oRank;
	}



	/** 
	 *	@brief	ref accessor via getter
	 *	@return	const std::vector<T> &	Reference to rank vector.		
	 *	@sa std::vector
	*/
	std::vector<int> &getRank () { return m_oRank; }



	/** 
	 *	@brief	Get number of elements.
	 *	@return	Number of data alements stored. This number is always always equal to the size of the rank vector.
	 *	@sa 
	*/
	std::size_t size() const {
		assert(m_oRank.size() == m_oData.size() && "SIZE() Inconsistency between rank and data. Size must be equal.");
		return m_oData.size(); 
	}



	/** 
	 *	@brief	reinitialize data and rank vector with zeros / default constructor and minimum rank. Size remains equal, no reallocation.
	 *	@param	void
	 *	@return	void			
	 *	@sa TArray
	*/
	void reinitialize(const T &rValue = T()) {
		std::fill(m_oData.begin(), m_oData.end(), rValue);
		std::fill(m_oRank.begin(), m_oRank.end(), filter::eRankMin);
	}



	/** 
	 *	@brief	assign data and rank vector at a certain size
	 *	@param	p_oSize			Array length for both, data and its rank.
	 *	@param	p_oVal			Value to be assigned.
	 *	@param	p_oRank			Rank to be assigned.
	 *	@return	void			
	 *	@sa TArray
	*/
	void assign( std::size_t p_oSize = 0u, T p_oVal = T(), int p_oRank = filter::eRankMin ) {
		m_oData.assign( p_oSize, p_oVal );
		m_oRank.assign( p_oSize, p_oRank );
	}

	/** 
	 *	@brief	resize data and rank vector (only new elements will be set to 0)
	 *	@param	p_oSize			Array length for both, data and its rank.
	 *	@return	void			
	 *	@sa TArray
	*/
	void resize( std::size_t p_oSize) {
		m_oData.resize( p_oSize);
		m_oRank.resize( p_oSize);
	}



	/** 
	 *	@brief	swap. exeption save.
	 *	@param	p_rToSwap		Other array.
	 *	@return	void			
	 *	@sa std::vector::swap
	*/
	void swap(TArray<T> &p_rToSwap) {
		m_oData.swap(p_rToSwap.m_oData);
		m_oRank.swap(p_rToSwap.m_oRank);
	}



	/** 
	 *	@brief	syntactic sugar. Const overload.
	 *	@param	p_oIndex		Element index.
	 *	@return	std::tuple		Pair that contains data and rank for the given index.	
	 *	@sa std::tuple<T, int>
	*/
	std::tuple<const T&, const int&> operator[](std::size_t p_oIndex) const {
		return std::tie(m_oData[p_oIndex], m_oRank[p_oIndex]);
	}



	/** 
	 *	@brief	syntactic sugar. Non-const overload.
	 *	@param	p_oIndex				Element index.
	 *	@return	std::tuple<T&, int&>	Pair that contains references on data and rank for the given index.
	 *	@sa std::tuple
	*/
	std::tuple<T&, int&> operator[](std::size_t p_oIndex) {
		return std::tie(m_oData[p_oIndex], m_oRank[p_oIndex]); // make a tuple of references
	}

    /**
     *	@brief	syntactic sugar for push_back operation
     *	@param	data: value for the data array
     * @param rank: value for the rank array
    */
    void push_back(const T& data, int rank)
    {
        m_oData.push_back(data);
        m_oRank.push_back(rank);
    }

	void reserve(std::size_t n)
    {
        m_oData.reserve(n);
        m_oRank.reserve(n);
    }

    void clear()
    {
        m_oData.clear();
        m_oRank.clear();
    }

    bool empty() const
    {
        return size() == 0;
    }

private:
	/// data array
	std::vector<T>		m_oData;
	/// rank array corresponding to data. The rank may lie between [0, 255]. 0: worst reliability, 255: best reliability.
	/// The length must be equal to the length of the data vector.
	std::vector<int>	m_oRank;

};



/// definitions

typedef TArray< byte >					   Bytearray;
typedef TArray< int >					   Intarray;
typedef TArray< double >				   Doublearray;
typedef TArray< Point >					   Pointarray;
typedef TArray< DPoint >				   DPointarray;
typedef TArray< Blob >					   Blobarray;
typedef TArray< SeamFinding >			   SeamFindingarray;
typedef TArray< PoorPenetrationCandidate > PoorPenetrationCandidatearray;
typedef TArray< HoughPPCandidate >         HoughPPCandidatearray;
typedef TArray< StartEndInfo >             StartEndInfoarray;
typedef TArray< SurfaceInfo >              SurfaceInfoarray;
typedef TArray< LineModel >                LineModelarray;


typedef std::vector< TArray< double > > VecDoublearray;
typedef std::vector< TArray< int > > 	VecIntarray;
typedef std::vector< TArray< byte > > 	VecBytearray;



} // namespace geo2d
} // namespace precitec


#endif // ARRAY_H
