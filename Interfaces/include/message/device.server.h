#ifndef DEVICE_SERVER_H_
#define DEVICE_SERVER_H_

#include  <map> // wg HandlerList
#include  "Poco/NamedMutex.h"
#include  "Poco/ScopedLock.h"
#include  "Poco/Process.h"
#include  "Poco/Path.h"

#include  "message/device.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TDevice<MsgServer> : public TDevice<AbstractInterface>
	{
	public:
		// aehnlich dem PoCo-Subsystem-Interface
		// 1. Ini- wird nur einmal durchgefuehrt
		virtual int initialize(Configuration const& config, int subDevice=0) {return -1;}
		/// die Reset-Taste
		virtual void uninitialize() {}
		/// kann beliebig oft durchgefuehrt werden
		virtual void reinitialize() {}

		/// spezifischen Parameter setzen
		virtual KeyHandle set(SmpKeyValue keyValue, int subDevice=0) { return KeyHandle(); }
		/// meherere Parameter setzen
		virtual void set(Configuration, int subDevice=0) {}

		/// spezifischen Parameter auslesen
		//virtual KeyValue get(Key key, int subDevice=0) { return KeyValue(); }
		virtual  SmpKeyValue get(Key key, int subDevice=0) { return SmpKeyValue(new KeyValue(TInt,"?",-1) ); }

		/// spezifischen Parameter auslesen
		//virtual KeyValue get(KeyHandle handle, int subDevice=0) { return KeyValue(); }
		virtual SmpKeyValue get(KeyHandle handle, int subDevice=0) { return SmpKeyValue(new KeyValue(TInt,"?",-1) );  }

		/// alle Parameter auslesen
		virtual Configuration get(int subDevice=0) { return Configuration(); }

	};


} // namespace interface
} // namespace precitec

#endif /*DEVICE_SERVER_H_*/
