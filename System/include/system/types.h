/**
 *  @defgroup	System System
 *
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		wor, hs
 * 	@date		2010
 * 	@brief		Typedefs, typetraits
 */

#ifndef TYPES_H_
#define TYPES_H_

/// @cond HIDDEN_SYMBOLS
#include <string>
#include <vector>
#include <map>
/// @endcond

#include "SystemManifest.h"

typedef std::string 	stdString; ///< obsolet wg Windows Inkompatibilitaet -> PvString
typedef std::string 	PvString;
typedef unsigned char 	byte;
typedef unsigned int	uInt;
typedef unsigned short	uShort;
typedef unsigned long 	uLong;
typedef double  		real;
typedef long long		xLong;

typedef byte 			*PByte;
typedef void 			*PVoid;
typedef char			*PChar;
typedef const char		*PCChar;


namespace precitec {


enum Types { TChar, TByte, TInt, TUInt, TBool, TFloat, TDouble, TString, TNumTypes, TOpMode,   /*TInt32,*/ TUnknown};

// Sucht aus einem Standardtypen die entsprechende Nummer
// Example: Types t = FindTType<int>::TType:
template <class T>	struct SYSTEM_API FindTType		{ };
template <>	struct SYSTEM_API FindTType<char>			{ const static Types TType = TChar; 	};
template <>	struct SYSTEM_API FindTType<byte>			{ const static Types TType = TByte; 	};
template <>	struct SYSTEM_API FindTType<int>			{ const static Types TType = TInt; 		};
template <>	struct SYSTEM_API FindTType<uInt>			{ const static Types TType = TUInt; 	};
template <>	struct SYSTEM_API FindTType<bool>			{ const static Types TType = TBool; 	};
template <>	struct SYSTEM_API FindTType<float>			{ const static Types TType = TFloat; 	};
template <>	struct SYSTEM_API FindTType<double>			{ const static Types TType = TDouble;	};
template <>	struct SYSTEM_API FindTType<PvString>		{ const static Types TType = TString;	};
//template <> struct SYSTEM_API FindTType<std::int32_t> { const static Types TType = TInt32; };

/// Umkehrfunktion von FindTType
// Example: FindStdTType<TChar>::Type t;
template <Types T> 	struct SYSTEM_API FindStdTType 		{ };
template <>	struct SYSTEM_API FindStdTType<TChar> 		{ typedef char 			Type; static const PvString ToStr; };
template <>	struct SYSTEM_API FindStdTType<TByte> 		{ typedef byte 			Type; static const PvString ToStr; };
template <>	struct SYSTEM_API FindStdTType<TInt> 		{ typedef int 			Type; static const PvString ToStr; };
template <>	struct SYSTEM_API FindStdTType<TUInt> 		{ typedef uInt 			Type; static const PvString ToStr; };
template <>	struct SYSTEM_API FindStdTType<TBool> 		{ typedef bool 			Type; static const PvString ToStr; };
template <>	struct SYSTEM_API FindStdTType<TFloat> 		{ typedef float 		Type; static const PvString ToStr; };
template <>	struct SYSTEM_API FindStdTType<TDouble> 	{ typedef double		Type; static const PvString ToStr; };
template <>	struct SYSTEM_API FindStdTType<TString> 	{ typedef PvString 		Type; static const PvString ToStr; };
//template <> struct SYSTEM_API FindStdTType<TInt32> { typedef std::int32_t Type; static const PvString ToStr; };


namespace system {


/// fixed string representation of types

const /*static*/ std::string	CHAR	=	"char";
const /*static*/ std::string	BYTE	=	"byte";
const /*static*/ std::string	INT		=	"int";
const /*static*/ std::string	UINT	=	"uint";
const /*static*/ std::string	BOOL	=	"bool";
const /*static*/ std::string	FLOAT	=	"float";
const /*static*/ std::string	DOUBLE	=	"double";
const /*static*/ std::string	STRING	=	"string";
//const /*static*/ std::string INT32 = "std::int32_t";


/// type string to type enum relation. Usage: Types oType = StringToType::get("int");. Currently not used.

class SYSTEM_API StringToType {
public:
	static Types get(const std::string &p_rTypeName) {
		return m_oTypesMap.at(p_rTypeName); // access specified element with bounds checking 
	}
private:
	StringToType() {}; // forbid construction
	static std::map<std::string, Types> createMap(); // map factory
	const static std::map<std::string, Types> m_oTypesMap;
}; // StringToType


} // system
} // precitec


#endif /* TYPES_H_*/
