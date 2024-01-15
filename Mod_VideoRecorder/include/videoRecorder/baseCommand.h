/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		File commands which can be executed by a command processor.
 */

#ifndef BASECOMMAND_H_20121109_INCLUDE
#define BASECOMMAND_H_20121109_INCLUDE

// stl includes
#include <memory>


namespace precitec {
namespace vdr {


class BaseCommand;
typedef std::unique_ptr<BaseCommand> upBaseCommand_t;

/**
 * @brief	Base command.
 */
class BaseCommand {
public:
	virtual void execute() = 0;
	virtual ~BaseCommand() {}

	BaseCommand() 								= default;
	BaseCommand(const BaseCommand&) 			= delete;
	BaseCommand& operator=(const BaseCommand&) 	= delete;
};


} // namespace vdr
} // namespace precitec

#endif // BASECOMMAND_H_20121109_INCLUDE
