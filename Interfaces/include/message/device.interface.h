#ifndef DEVICE_INTERFACE_H_
#define DEVICE_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"
#include  "message/device.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <int CallType>
	class TDevice;

	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	template <>
	class TDevice<AbstractInterface>
	{
	public:
		TDevice() {}
		virtual ~TDevice() {}
	public:
		// aehnlich dem PoCo-Subsystem-Interface
		// 1. Ini- wird nur einmal durchgefuehrt
		virtual int initialize(Configuration const& config, int subDevice=0) = 0;
		/// die Reset-Taste
		virtual void uninitialize() = 0;
		/// kann beliebig oft durchgefuehrt werden
		virtual void reinitialize() = 0;

		/// spezifischen Parameter setzen
		//virtual KeyHandle set(KeyValue const& keyValue, int subDevice=0) = 0;
		virtual KeyHandle set(SmpKeyValue keyValue, int subDevice=0) = 0;

		/// meherere Parameter setzen
		virtual void set(Configuration config, int subDevice=0) = 0;
		/// spezifischen Parameter auslesen
		virtual SmpKeyValue get(Key key, int subDevice=0) = 0;
		/// spezifischen Parameter auslesen
		virtual SmpKeyValue get(KeyHandle handle, int subDevice=0) = 0;
		/// alle Parameter auslesen
		virtual Configuration get(int subDevice=0) = 0;
	};

    struct TDeviceMessageDefinition
    {
		MESSAGE(int, Initialize, Configuration, int);
		MESSAGE(void, Uninitialize, void);
		MESSAGE(void, Reinitialize, void);
		MESSAGE(KeyHandle, SetKeyValue, SmpKeyValue, int);
		MESSAGE(void, SetConfiguration, Configuration, int);
		MESSAGE(KeyValue, GetKey, Key, int);
		MESSAGE(KeyValue, GetHandle, KeyHandle, int);
		MESSAGE(Configuration, GetConfiguration, int);

		MESSAGE_LIST(
			Initialize,
			Uninitialize,
			Reinitialize,
			SetKeyValue,
			SetConfiguration,
			GetKey,
			GetHandle,
			GetConfiguration
		);

    };

	template <>
	class TDevice<Messages> : public Server<Messages>, public TDeviceMessageDefinition
	{
	public:
		TDevice() : info(system::module::Device, sendBufLen, replyBufLen, MessageList::NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 10*KBytes, replyBufLen = 2*MBytes }; // von 5000 auf 10000 erhoeht
	};

} // namespace interface
} // namespace precitec


#endif /*DEVICE_INTERFACE_H_*/
