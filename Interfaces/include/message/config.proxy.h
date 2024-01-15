#ifndef CONFIG_PROXY_H_
#define CONFIG_PROXY_H_

#include <string> // std::string
#include <map>		// std::map

#include "Poco/DynamicAny.h"

#include "server/proxy.h"
#include "message/config.interface.h"

namespace precitec
{
	using namespace  message;
	
namespace interface
{

	template <>
	class TConfig<MsgProxy> : public Server<MsgProxy>, TConfig<AbstractInterface>
	{
	public:
		/// beide Basisklassen muessen geeignet initialisiert werden
		TConfig() : PROXY_CTOR(TConfig), TConfig<AbstractInterface>()
		{
		} 
		/// normalerweise wird das Protokoll gleich mitgeliefert
		TConfig(SmpProtocolInfo &p) : PROXY_CTOR1(TConfig,  p), TConfig<AbstractInterface>()
		{
		}
		/// der DTor muss virtuell sein 
		virtual ~TConfig() {}
	public:
			
		virtual void set(PvString key, DynamicAny value) {
			//typedef TConfig<Messages>::MESSAGE_NAME2(setParam, PvString, DynamicAny) Msg;
			INIT_MESSAGE2(TConfig, setParam, PvString, DynamicAny);
			// Value wird als PvString verschickt.
			sender().marshal( key);
			sender().marshal( value.convert<PvString>() );
			sender().send();
		}
		
		virtual void init(ParameterList const& params)
		{
			// typedef TConfig<Messages>::MESSAGE_NAME1(init, ParameterList) Msg; 
			INIT_MESSAGE1(TConfig, init, ParameterList);
			sender().marshal(params.size());
/*			for(ParameterList::iterator it= params.begin(); it<params.end(); it++) {
				receiver().marshal( it->first );
				receiver().marshal( it->second.convert<PvString>() );
			}*/
			sender().send();
		}
	};

} // namespace interface
} // namespace precitec


#endif /*CONFIG_PROXY_H_*/
