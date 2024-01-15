/*
 * wmLoffer.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: abeschorner
 */

// WM includes
#include "wmLogger.h"
// clib includes
#include <cstdarg>

namespace precitec {
namespace utils {

	/// Create(Message, SenderID).<br /><b>Defaults: Logtype of message = info, verbosity = 9.</b>
  wmLogMessage* wmLogMessage::create(std::string msg, std::string sid)
	{
		return new wmLogMessage(msg, sid, precitec::utils::_defaultLogtype, precitec::utils::_defaultVerbosity);
	}

	/// Create(Message, SenderID, MessageType). <br /><b>Default: Verbosity = 9.</b>
	wmLogMessage* wmLogMessage::create(std::string msg, std::string sid, tLogtype mt)
	{
		return new wmLogMessage(msg, sid, mt, precitec::utils::_defaultVerbosity);
	}

	/// Create(Message, SenderID, MessageType, Verbosity);
	wmLogMessage* wmLogMessage::create(std::string msg, std::string sid, tLogtype mt, unsigned int vb)
	{
		return new wmLogMessage(msg, sid, mt, vb%10);
	}

  /// Create(Message, SenderID, Verbosity).<br /><b>Defaults: Logtype of message = info.</b>
	wmLogMessage* wmLogMessage::create(std::string msg, std::string sid, unsigned int vb)
	{
		return new wmLogMessage(msg, sid, precitec::utils::_defaultLogtype, precitec::utils::_defaultVerbosity);
	}

	// -------------------------------------------------------------------

	bool wmLoggerBase::generateTimestampOnCreation = true;

}
}
