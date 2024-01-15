/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB), changes: Daniel Feist (Ft)
 *  @date       2012
 *  @brief      Here the log-messages are defined. The log messages are stored in a shared-memory region and collected by the logger server.
 */
#ifndef LOGMESSAGE_H_
#define LOGMESSAGE_H_

// Poco includes
#include <Poco/Timestamp.h>
// clib includes
#include <sstream>
#include <cstdint>
#include <cstdarg>

#include <type_traits>

namespace precitec {

namespace typetraits {

template <typename... Ts>
struct all_double_or_string;

template <typename T, typename... Ts>
struct all_double_or_string<T, Ts...>
{
    static const bool value = (std::is_convertible<T, double>::value || std::is_convertible<T, std::string>::value) && 
                               all_double_or_string<Ts...>::value;
};

template <typename T>
struct all_double_or_string<T>
{
    static const bool value = std::is_convertible<T, double>::value || std::is_convertible<T, std::string>::value;
};

template<>
struct all_double_or_string<>
{
    static const bool value = true;
};

} // namespace typetraits

const unsigned int LogMessageLength = 256;      ///< Max length of a single message-string - the rest is cut off.
const unsigned int LogMessageCapacity = 2048;   ///< How many log messages can be stored in the shared memory.
const unsigned int LogModuleNameLength = 40;    ///< Max length of the module name in the log message.
const unsigned int LogMessageParams = 5;        ///< Number of parameters in a SharedMemory message.
const unsigned int LogIntKeyLength = 30;        ///< Length of internationalization key.
const unsigned int LogParamStringLength = 40;   ///< Length of parameter string.


/**
 * @brief Super simple logger parameter class (TM).
 */
class LogParam
{

public:

    /**
     * @brief CTor - constructs an object that is not a valid parameter yet - please use the setString or setValue functions after constructing the object.
     */
    LogParam() :
        m_oValid( false )
    {}
    /**
     * @brief CTor - constructs a number parameter.
     */
    LogParam( double p_oValue ) :
        m_oValue( p_oValue ),
        m_oStringType( false ),
        m_oValid( true )
    {}

    /**
     * @brief CTor - constructs a string parameter.
     */
    LogParam( std::string p_oString )
    {
        setString( p_oString );
    }

    /// Retrieve the (double) value from the param object.
    double value()                              { return m_oValue; }
    /// Set the value of the param object - this object is now a valid number parameter.
    void setValue( double p_oValue )            { m_oValue = p_oValue; m_oStringType = false; m_oValid = true; }
    /// Get the string stored in this param object.
    std::string string()                        { return m_oString; }
    /**
     * @brief Set the string stored in this object - this object is now a valid string parameter.
     * @param p_oString std::string object, but be aware that only the first 40 characters are stored.
     */
    void setString( std::string p_oString )
    {
        p_oString.copy( m_oString, LogParamStringLength );
        if (p_oString.length() < LogParamStringLength)
            m_oString[p_oString.length()] = '\0';
        else
            m_oString[ LogParamStringLength-1 ] = '\0';
        m_oStringType = true; m_oValid = true;
    }
    /// Is this object a string container or a number container?
    bool isString()                             { return m_oStringType; }
    /// Is this object valid.
    bool isValid()                              { return m_oValid; }

private:

    char            m_oString[LogParamStringLength];
    double          m_oValue;
    bool            m_oStringType;
    bool            m_oValid;
};


/**
 * @brief Logger message class - represents a single message, which is mapped into an array in a shared memory region.
 */
class LogMessage
{
private:

    template <typename I>
    void extractParams( unsigned int cnt, I start, I index )
    {
        // base case: if parameter pack is empty, do nothing. 
    }

    template <typename I, typename T>
    void extractParams( unsigned int cnt, I start, I index, T&& val )
    {
        m_oParams[cnt] = LogParam( std::forward<T>(val) );
    }

    template <typename I, typename T, typename... Ts>
    void extractParams( unsigned int cnt, I start, I index, T&& val, Ts&&... vals )
    {
        for(; *index != '\0' && std::distance( start, index ) < LogMessageLength && cnt < LogMessageParams; index++)
        {
            if (*index == '%')
            {
                m_oParams[cnt] = LogParam( std::forward<T>(val) );
                extractParams( ++cnt, start, index, std::forward<Ts>(vals)... );
                return;
            }
        }
    }

public:

	/**
	 * @brief Format the entire message into a string. The function will put all parameters into the message string and put also the time and module information into the resulting string.
	 * @return std::string object representing the message.
	 */
	std::string format();

    /**
     * @brief Put all the parameters into the message and generate a single string - this string cannot be translated by the gui anymore ...
     * @return std::string object.
     */
    std::string formatParams( );

    /**
     * @brief extract all arguments and store them into the LogMessage object.
     * @param p_oString std::string with the message and placeholders '%' for the arguments (e.g. "This text contains %s arguments").
     * @param vals parameter pack that holds the actual arguments.
     * NOTE: nothing will break if the number of "%" characters in the message does not match the number of arguments in vals.
     */
    template <typename... Ts>
    void extractParams( std::string p_oString, Ts&&... vals )
    {
        static_assert( typetraits::all_double_or_string<Ts...>::value, "All parameter types must be convertible to double or std::string" );

        for( std::size_t i = 0; i < LogMessageParams; ++i )
        {
            m_oParams[i] = LogParam();
        }

        extractParams( 0, p_oString.begin(), p_oString.begin(), std::forward<Ts>(vals)... );
    }

public:

    unsigned int        m_oType;
    unsigned int 		m_oErrorCode;
    Poco::Timestamp     m_oTimestamp;
    char                m_oModule[LogModuleNameLength];
    char                m_oBuffer[LogMessageLength];
    char                m_oIntKey[LogIntKeyLength];
    LogParam            m_oParams[LogMessageParams];
};


/**
 * @brief Logger shared memory content.
 */
class LogShMemContent
{
public:

    uint32_t        m_oWriteIndex;
    uint32_t        m_oReadIndex;
    uint32_t        m_oReadIndexGui;
    bool            m_bRollOver;
    bool            m_bRollOverGui;

    LogMessage      m_oMessages[LogMessageCapacity];
    bool increaseWriteIndex();
};


} // namespace precitec

#endif /* LOGMESSAGE_H_ */
