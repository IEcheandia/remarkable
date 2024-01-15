/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		hs
 * 	@date		2011
 * 	@brief		Implementation of std implementations that are not provided, e.g. template specialisations for custom types.
 */

#ifndef STDIMPLEMENTATION_H_20111214_INCLUDED
#define STDIMPLEMENTATION_H_20111214_INCLUDED


#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>
#include <functional>				///< hash 

#include "Poco/UUID.h"				


namespace std {
/**
 *	@brief		Inserts array data into output stream.
 *	@param		p_rOStream	Out stream to be modified.
 *	@param		p_rVecIn	Input data
 *	@return		ostream		Modified stream.
*/
template <typename T>
inline ostream& operator<<( ostream& p_rOStream, const vector<T> &p_rVecIn ) {
	p_rOStream << "<Vector= ";
	ostream_iterator<T> out_it (p_rOStream, " "); // cast necessary for numeric char data
	copy(p_rVecIn.begin(), p_rVecIn.end(), out_it);
	p_rOStream << "Vector>";
	return p_rOStream;
}

/**
  *	@brief	defines std::hash<tuple<string, string>> as specialization of std::hash<T> 
  *	@details	types that are to serve as key for a map or a set need to have a hash function defined as sort criterium
  *	@sa		http://marknelson.us/2011/09/03/hash-functions-for-c-unordered-containers/
*/
	typedef tuple<string, string> str_pair_t;
	template <>
	class hash<str_pair_t> {
	public:
		size_t operator()(const str_pair_t &str_pair) const {
			return hash<string>()(get<0>(str_pair)) ^ hash<string>()(get<1>(str_pair)); // XOR recommended to compose hashes
		} // operator()
	}; // hash<str_pair_t>


/**
  *	@brief	defines std::hash<Poco::UUID> as specialization of std::hash<T> 
  *	@details	types that are to serve as key for a map or a set need to have a hash function defined as sort criterium
  *	@sa		http://marknelson.us/2011/09/03/hash-functions-for-c-unordered-containers/
*/
	template <>
	class hash<Poco::UUID> {
	public:
		size_t operator()(const Poco::UUID &p_rUuid) const {
			return hash<string>()(p_rUuid.toString()) ;
		} // operator()
	}; // hash<Poco::UUID>
} // namespace std

#endif // STDIMPLEMENTATION_H_20111214_INCLUDED
