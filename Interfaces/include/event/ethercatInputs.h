/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       19.11.2016
 *  @brief      Part of the EthercatInputs Interface
 *  @details
 */

#ifndef ETHERCATINPUTS_H_
#define ETHERCATINPUTS_H_

#include <stdint.h>
#include <vector>

#include "event/ethercatDefines.h"
#include "event/viService.h"

namespace precitec
{
		 	
namespace interface
{

	typedef std::vector<uint8_t> stdVecUINT8;
	typedef std::vector<int16_t> stdVecINT16;
	typedef std::vector<uint16_t> stdVecUINT16;

} // namespace interface
} // namespace precitec

#endif /* ETHERCATINPUTS_H_ */

