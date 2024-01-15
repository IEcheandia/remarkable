#ifndef CONFIG_H_
#define CONFIG_H_

#include <map>		// std::map
#include "Poco/TypeList.h"
#include "Poco/DynamicAny.h"
#include "system/types.h"		// PvString
namespace precitec
{
namespace interface
{
	using Poco::DynamicAny;
	typedef std::map<PvString, DynamicAny> 	ParameterList;
	typedef std::pair<PvString, DynamicAny> 	ParameterEntry;
		
} // namespace interface
} // namespace precitec

	
#endif /*CONFIG_H_*/
