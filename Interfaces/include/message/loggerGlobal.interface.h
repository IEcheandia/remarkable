/**
 *  @file
 *  @copyright      Precitec Vision GmbH & Co. KG
 *  @author         Stefan Birmanns (SB)
 *  @date           03.05.2012
 *  @brief          This is the interface of the wm logger. It is called by the GUI / wmHost to retrieve the next batch of log messages from the QNX side.
 */
#ifndef LOGGER_GLOBAL_INTERFACE_H_
#define LOGGER_GLOBAL_INTERFACE_H_

// WM includes
#include "server/interface.h"
#include "module/interfaces.h"
#include "protocol/protocol.info.h"
// Poco includes
#include "Poco/Timestamp.h"

namespace precitec
{
	using namespace system;
	using namespace message;
	using system::module::Modules;
	using Poco::Timestamp;

namespace interface
{

	/**
	 * @brief A single logger parameter.
	 */
	class wmLogParam : public Serializable
	{
	public:

		/**
		 * @brief CTor - constructs an object that is not a valid parameter yet - please use the setString or setValue functions after constructing the object.
		 */
		wmLogParam() :
			m_oValid( false )
		{}
		/**
		 * @brief CTor - constructs a number parameter.
		 */
		wmLogParam( double p_oValue ) :
			m_oValue( p_oValue ),
			m_oStringType( false ),
			m_oValid( true )
		{}

		/**
		 * @brief CTor - constructs a string parameter.
		 */
		wmLogParam( std::string p_oString )
		{
			setString( p_oString );
		}

		/// Retrieve the (double) value from the param object.
		double value() const                             { return m_oValue; }
		/// Set the value of the param object - this object is now a valid number parameter.
		void setValue( double p_oValue )            { m_oValue = p_oValue; m_oStringType = false; m_oValid = true; }
		/// Get the string stored in this param object.
		std::string string() const                       { return m_oString; }
		/**
		 * @brief Set the string stored in this object - this object is now a valid string parameter.
		 * @param p_oString std::string object, but be aware that only the first 40 characters are stored.
		 */
		void setString( std::string p_oString )
		{
			m_oString = p_oString;
			m_oStringType = true; m_oValid = true;
		}
		/// Is this object a string container or a number container?
		bool isString()                             { return m_oStringType; }
		/// Is this object valid.
		bool isValid()                              { return m_oValid; }

        /**
         * @brief Serialize a log message.
         * @param p_rBuffer Reference to the message buffer, into which the message gets serialized.
         */
        virtual void serialize ( MessageBuffer& p_rBuffer ) const
        {
            marshal( p_rBuffer, m_oString );
            marshal( p_rBuffer, m_oValue );
            marshal( p_rBuffer, m_oStringType );
            marshal( p_rBuffer, m_oValid );
        }

        /**
         * @brief Deserialize a log message.
         * @param p_rBuffer Reference to the message buffer, from where the message gets deserialized.
         */
        virtual void deserialize( MessageBuffer const&p_rBuffer )
        {
            deMarshal( p_rBuffer, m_oString );
            deMarshal( p_rBuffer, m_oValue );
            deMarshal( p_rBuffer, m_oStringType );
            deMarshal( p_rBuffer, m_oValid );
        }

	private:

		std::string     m_oString;
		double          m_oValue;
		bool            m_oStringType;
		bool            m_oValid;
	};

    /**
     * @brief A single log message.
     */
    class wmLogItem : public Serializable
    {
    public:

        /**
         * @brief CTor.
         * @param p_oMessage Message string.
         * @param p_oMessageKey Is the message a real string, or is it a key that is used for internationalization. In that case the GUI will use it to look-up the actual message string in the language file. True if the message is a language-file-key, false if the message has to be printed directly.
         * @param p_oType Type of log-message (see wmLogger.h for details).
         */
        wmLogItem( Poco::Timestamp p_oTimestamp = Poco::Timestamp(), unsigned int p_oIndex =0, std::string p_oMessage = std::string(""), std::string p_oKey = std::string(""), std::string p_oModule = std::string("DummyModule"), const int p_oType =1 ) : Serializable(),
            m_oTimestamp( p_oTimestamp ),
            m_oIndex( p_oIndex ),
            m_oMessage( p_oMessage ),
            m_oKey( p_oKey ),
            m_oModule( p_oModule ),
            m_oType( p_oType )
        {
        } // CTor

    public:

        /**
         * @brief Serialize a log message.
         * @param p_rBuffer Reference to the message buffer, into which the message gets serialized.
         */
        virtual void serialize ( MessageBuffer& p_rBuffer ) const
        {
            marshal( p_rBuffer, m_oTimestamp );
            marshal( p_rBuffer, m_oMessage );
            marshal( p_rBuffer, m_oKey );
            marshal( p_rBuffer, m_oModule );
			marshal( p_rBuffer, m_oType );
			marshal( p_rBuffer, m_oParams );

        } // serialize

        /**
         * @brief Deserialize a log message.
         * @param p_rBuffer Reference to the message buffer, from where the message gets deserialized.
         */
        virtual void deserialize( MessageBuffer const&p_rBuffer )
        {
            deMarshal( p_rBuffer, m_oTimestamp );
            deMarshal( p_rBuffer, m_oMessage );
            deMarshal( p_rBuffer, m_oKey );
            deMarshal( p_rBuffer, m_oModule );
			deMarshal( p_rBuffer, m_oType );
			deMarshal( p_rBuffer, m_oParams );

        } // deserialize

        /**
         * @brief Extract the message string from the wmLogItem.
         * @return std::string object containing the message.
         */
        std::string message() const
        {
            return m_oMessage;

        } // message

        /**
         * @brief Name of the module that has generated the log message.
         * @return std::string object with name of module.
         */
        std::string moduleName() const
        {
            return m_oModule;

        } // moduleName

        /**
         * @brief Retrieve the type of the wmLogItem.
         * @return int with the type of the log-item. See wmLogger.h for details.
         */
		int type() const
		{
		    return m_oType;

		} // type

		/**
		 * @brief Get the internationalization key - if the length of the key is 0, the key is not valid and the message string has to be used directly instead (no internationalization). The GUI will use the key to look-up the actual message string in the language file.
		 * @return std::string containing the key.
		 */
		std::string key() const
		{
			return m_oKey;

		} // key

		/**
		 * @brief Get the timestamp of the message - it is generated at the time of the qnx wmLog call ...
		 */
		Poco::Timestamp timestamp() const
		{
		    return m_oTimestamp;

		} // timestamp

		unsigned int index()
		{
		    return m_oIndex;

		} // index

		/**
		 * @brief Add parameter to the wmLogItem.
		 * @param p_oParam wmLogParam object.
		 */
		void addParam( wmLogParam p_oParam )
		{
			m_oParams.push_back( p_oParam );

		} // addParam

		/**
		 * @brief Get all the parameters attached to the wmLogItem.
		 * @return std::vector with all the wmLogParams.
		 */
		const std::vector<wmLogParam> &getParams() const
		{
			return m_oParams;

		} // getParams

    private:

        Poco::Timestamp m_oTimestamp;   	///< Timestamp of message.
        unsigned int m_oIndex;          	///< ReadIndex - the timestamp only has a microsecond resolution, to resolve conflicts we need to have access to the index ...
        std::string m_oMessage;         	///< Message string.
        std::string m_oKey;					///< Internationalization key.
        std::string m_oModule;          	///< Name of the module that has generated the log message.
		int m_oType;                    	///< Type int, see moduleLogger.h.
		std::vector<wmLogParam> m_oParams;	///< Log-parameter.

    };

    /**
     * @brief Comparison class for log items
     */
    class wmLogItemComp
    {
    public:
        bool operator()(wmLogItem p_oItem1, wmLogItem p_oItem2) const
        {
            if( p_oItem1.timestamp() == p_oItem2.timestamp() )
            {
                if (p_oItem1.index() <= p_oItem2.index())
                    return true;
                else
                    return false;
            }

            if( p_oItem1.timestamp() < p_oItem2.timestamp() )
                return true;
            else
                return false;
        }
    };

    /**
     * @brief A vector of log items - the interface will group the log items and send a batch of items to windows.
     */
    typedef std::vector< wmLogItem > wmLogBuffer;


} // interface
} // precitec

#endif /*LOGGER_GLOBAL_INTERFACE_H_*/
