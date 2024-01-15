#ifndef CONFIG_INTERFACE_H_
#define CONFIG_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"
#include  "message/config.h"

/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
	using namespace  system;
	using namespace  message;
	
namespace interface
{

	template <int CallType> 
	class TConfig;
	
	// Schnittstelle Result Event
	template <>
	class TConfig<AbstractInterface>
	{
	public:
		TConfig() {}
		virtual ~TConfig() {}
	public:
		virtual void setParam(PvString key, DynamicAny value) = 0;		// einen Parameter setzen
		virtual void init(ParameterList const& params) = 0;		// eine Parameterliste initialisieren
	};
	
	//----------------------------------------------------------
	template <>
	class TConfig<Messages> : public Server<Messages>
	{
	public:
		TConfig<Messages>() : info(system::module::Config, sendBufLen, replyBufLen, NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes };
	public:
		MESSAGE_LIST2(
			system::module::Config,
			MESSAGE_NAME2(setParam, PvString, DynamicAny),  
			MESSAGE_NAME1(init, ParameterList)  
		);
	public:
		DEFINE_MSG1(void, init, ParameterList);
		DEFINE_MSG2(void, setParam, PvString, DynamicAny);
	};


} // namespace interface
} // namespace precitec


#endif /*CONFIG_INTERFACE_H_*/
