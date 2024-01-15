#ifndef DEVICE_HANDLER_H_
#define DEVICE_HANDLER_H_

#include  "Poco/NamedMutex.h"
#include  "Poco/ScopedLock.h"
#include  "Poco/Process.h"

#include  "message/device.h"
#include  "server/handler.h"
#include  "message/device.interface.h"

namespace precitec
{
namespace interface
{

	template <>
	class TDevice<MsgHandler> : public Server<MsgHandler>, public TDeviceMessageDefinition
	{
	public:
		MSG_HANDLER(TDevice );
	public:

		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_MESSAGE(Initialize, initialize);
			REGISTER_MESSAGE(Uninitialize, uninitialize);
			REGISTER_MESSAGE(Reinitialize, reinitialize);

			//REGISTER2(TDevice, set,  KeyValue, int);
            REGISTER_MESSAGE(SetKeyValue, setKey);
            REGISTER_MESSAGE(SetConfiguration, setConfiguration);
            REGISTER_MESSAGE(GetKey, get<Key>);
            REGISTER_MESSAGE(GetHandle, get<KeyHandle>);
            REGISTER_MESSAGE(GetConfiguration, getConfiguration);

		}

		void initialize(Receiver &receiver)
		{
			Configuration configuration; receiver.deMarshal(configuration);
			int subDevice; receiver.deMarshal(subDevice);
			receiver.marshal(server_->initialize(configuration, subDevice));
			receiver.reply();
		}
		void uninitialize(Receiver &receiver)
		{
			server_->uninitialize();
			receiver.reply();
		}
		void reinitialize(Receiver &receiver)
		{
			server_->reinitialize();
			receiver.reply();
		}

        template <typename T>
		void get(Receiver &receiver)
		{
			T key; receiver.deMarshal(key);
			int subDevice; receiver.deMarshal(subDevice);
			receiver.marshal(server_->get(key, subDevice));
			receiver.reply();
		}

		void getConfiguration(Receiver &receiver)
		{
			int subDevice; receiver.deMarshal(subDevice);

			//std::cout << "TDevice<MsgHandler>::get subdevice demarshalled:" << subDevice << std::endl;
			//Configuration conf(server_->get(subDevice));
			//std::cout << "TDevice<MsgHandler>::get server called " << conf<< std::endl;
			//receiver.marshal(conf);

			receiver.marshal(server_->get(subDevice));
			//std::cout << "TDevice<MsgHandler>::get result marshaled" << std::endl;
			receiver.reply();
		}

		//void CALLBACK2(set, KeyValue, int)(Receiver &receiver)
		void setKey(Receiver &receiver)
		{
			//KeyValue keyValue; receiver.deMarshal(keyValue);
			SmpKeyValue keyValue; receiver.deMarshal(keyValue);
			int subDevice; receiver.deMarshal(subDevice);
			receiver.marshal(server_->set(keyValue, subDevice));
			receiver.reply();
		}

		void setConfiguration(Receiver &receiver)
		{
			Configuration configuration; receiver.deMarshal(configuration);
			int subDevice; receiver.deMarshal(subDevice);
			server_->set(configuration, subDevice);
			receiver.reply();
		}

	};

} // namespace interface
} // namespace precitec

#endif /*DEVICE_HANDLER_H_*/
