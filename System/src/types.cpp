/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		wor, hs
 * 	@date		2010
 * 	@brief		Typedefs, typetraits
 */

#include <string>
#include "../include/system/types.h"


namespace precitec {
namespace system {

	/// type string to type enum relation

	/*static*/ std::map<std::string, Types> StringToType::createMap() {
		std::map<std::string, Types> oMap;
		oMap[CHAR]		= TChar;
		oMap[BYTE]		= TByte;
		oMap[INT]		= TInt;
		oMap[UINT]		= TUInt;
		oMap[BOOL]		= TBool;
		oMap[FLOAT]		= TFloat;
		oMap[DOUBLE]	= TDouble;
		oMap[STRING]	= TString;
		//oMAP[INT32] = TInt32;
		return oMap;
	} 

	const /*static*/ std::map<std::string, Types> StringToType::m_oTypesMap = createMap();






} // system


using namespace precitec::system;

const std::string FindStdTType<TChar>::ToStr		= CHAR;
const std::string FindStdTType<TByte>::ToStr		= BYTE;
const std::string FindStdTType<TInt>::ToStr		= INT;
const std::string FindStdTType<TUInt>::ToStr		= UINT;
const std::string FindStdTType<TBool>::ToStr		= BOOL;
const std::string FindStdTType<TFloat>::ToStr	= FLOAT;
const std::string FindStdTType<TDouble>::ToStr	= DOUBLE;
const std::string FindStdTType<TString>::ToStr	= STRING;
//const std::string FindStdTType<TInt32>::ToStr = INT32;


} // precitec
