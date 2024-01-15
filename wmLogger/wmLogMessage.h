#ifndef WMLOGMESSAGE_H_
#define WMLOGMESSAGE_H_

// clib includes
#include <string>

namespace precitec {
namespace utils {

/// Message type enum.<br/><i>Default: msgInfo</i>
enum tLogtype {
  _logNull=0, _logInfo = 1, _logWarning = 2, _logError = 4, _logFatal = 8, _logDebug = 16, _logInfoDebug = (1|16), _logNIO = 32, _logVerbose = 64, _logEverything = 255
};
/// Default log type for messages.
const tLogtype _defaultLogtype = _logInfo;

/// Default verbosity (9=stream all).
const unsigned int _defaultVerbosity = 9;

/**
 * \class wmLogMessage
 *
 * \brief Message class for the logger; includes message characteristics such as a timestamp, the sender ID etc.
 *
 * Given the fact that the implemented queue is intrusive, this class -- which serves as node class for the queue -- needs
 * a <i>next</i>-pointer indexing itself as a type.
 *
 * A message is composed of the messag itself and parameters/ characteristics defining the type of message, the sender etc.
 * Details about those are given at the descriptions of the respective variables and methods.
 *
 **/
class wmLogMessage
{
public:
  /// Create(Message, SenderID).<br /><b>Defaults: Logtype of message = info, verbosity = 9.</b>
  static wmLogMessage* create(std::string msg, std::string sid);

  /// Create(Message, SenderID, MessageType). <br /><b>Default: Verbosity = 9.</b>
  static wmLogMessage* create(std::string msg, std::string sid, tLogtype mt);

  /// Create(Message, SenderID, MessageType, Verbosity);
  static wmLogMessage* create(std::string msg, std::string sid, tLogtype mt, unsigned int vb);

  /// Create(Message, SenderID, Verbosity).<br /><b>Defaults: Logtype of message = info.</b>
  static wmLogMessage* create(std::string msg, std::string sid, unsigned int vb);

  /// Constructor, should not be called! Only here as wmLogMessage is intended to be used in as template in template containers.
  wmLogMessage()
  {
	m_Msg = std::string("");
	m_MessageType = precitec::utils::_defaultLogtype;
	m_SenderID = std::string("");
	m_Verbosity = precitec::utils::_defaultVerbosity;
	m_Timestamp = std::string("");
  }

  /// Destructor.
  ~wmLogMessage(){};

  /// Returns message type (info, warning, error, nio, ...).
  inline tLogtype msgType() const
  {
	return m_MessageType;
  }

  /// Returns debug level (should be in range [0, 9])
  inline unsigned int verbosity()
  {
	return m_Verbosity;
  }

  /// Sets timestamp
  inline void setTimestamp(std::string s)
  {
	m_Timestamp.assign(s);
  }

  /// Returns timestamp
  inline std::string timestamp()
  {
	return std::string(m_Timestamp);
  }

  /// Returns message
  inline std::string msg() const
  {
	return std::string(m_Msg);
  }

  /// Set sender ID
  inline void setSender(std::string s)
  {
	m_SenderID.assign(s);
  }

  /// Returns sender
  inline std::string sender()
  {
	return m_SenderID;
  }

  /// Public <i>next</i> pointer needed as the queue is intrusive.
  wmLogMessage* next;

protected:
  /// Protected constructor as we want object creation instead, so we can make sure objects do not get lost due to potentially local validity. Remember deletion!
  explicit wmLogMessage(std::string msg, std::string sid, tLogtype mt, unsigned int vb)
  {
	m_MessageType = mt;
	m_Verbosity = vb % 10;
	m_SenderID = sid;
	m_Msg = msg;
	m_Timestamp.assign("");
  }

  /// Message type: info, warning, error, ... check enum <b>tLogtype</b> for a complete list of possible, combinable message types.<br /><i>Default: msgInfo</i>
  tLogtype m_MessageType;

  /// Verbosity ([0, 9]==[log nothing, log all]).<br /><i>Default: 9 (log all)</i>).
  unsigned int m_Verbosity;

  /// ID of sender, user specific. For the <i>WeldMaster</i>-project we decided to use the QNX function call __PRETTY_FUNCTION__.
  std::string m_SenderID;

  /// Time stamp. Will be created at the time of the creation of the message in the its constructor.
  std::string m_Timestamp;

  /// The message itself.
  std::string m_Msg;

};	// class wmLogMessage

} // namespace utils
} // namespace precitec

#endif /* WMLOGMESSAGE_H_ */
