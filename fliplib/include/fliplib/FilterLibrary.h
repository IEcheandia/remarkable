/*
 * FilterLibrary.h
 *
 *  Created on: 07.12.2010
 *      Author: Administrator
 */

#ifndef FILTERLIBRARY_H_
#define FILTERLIBRARY_H_

#include <string>
#include "Poco/Path.h"

#include "fliplib/Fliplib.h"

namespace fliplib
{

	class FLIPLIB_API FilterLibrary
	{
	public:

		std::string  collect (Poco::Path const& path);
	};
}

#endif /* FILTERLIBRARY_H_ */
