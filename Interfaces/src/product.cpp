/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, AB, HS
 * 	@date		2010
 * 	@brief		Product class.
 */

#include <algorithm> // for toupper

#include "common/product.h"
#include "module/moduleLogger.h"

using namespace Poco;
using Poco::UUID;
namespace precitec
{
	using namespace interface;
namespace interface
{

bool operator<(const Product& p_rFirst, const Product& p_rSecond) {
	return p_rFirst.productID() < p_rSecond.productID();
} // operator<


} // namespaces
}

