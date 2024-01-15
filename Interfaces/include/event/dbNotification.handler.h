/*
 * dbNotification.handler.h
 *
 *  Created on: 22.06.2010
 */

#ifndef DBNOTIFICATION_HANDLER_H_
#define DBNOTIFICATION_HANDLER_H_

#include  "Poco/UUID.h"
#include  "Poco/NamedMutex.h"
#include  "Poco/ScopedLock.h"
#include  "Poco/Process.h"

#include  "server/handler.h"
#include  "event/dbNotification.interface.h"

namespace precitec
{
namespace interface
{
	template <>
	class TDbNotification<EventHandler> : public Server<EventHandler>, public TDbNotificationMessageDefinition
	{
	public:
		EVENT_HANDLER(TDbNotification );

		typedef Poco::UUID	PocoUUID;

		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(SetupProduct, setupProduct);
			REGISTER_EVENT(SetupMeasureTask, setupMeasureTask);
			REGISTER_EVENT(SetupFilterParameter, setupFilterParameter);
			REGISTER_EVENT(SetupHardwareParameter, setupHardwareParameter);
			REGISTER_EVENT(ResetCalibration, resetCalibration);
		}

		void setupProduct(Receiver &receiver)
		{
			PocoUUID productID;
			try {
				receiver.deMarshal(productID);
			} catch (...) {
				wmLog( eError, "TDbNotification<MsgHandler>::setupProduct deMarshal caught exception\n" );
			}
			try {
				getServer()->setupProduct( productID);
			} catch (...) {
			    wmLog( eError, "TDbNotification<MsgHandler>::setupProduct caught exception\n" );
			}
			try {
				receiver.reply();
			} catch (...) {
				wmLog( eError, "TDbNotification<MsgHandler>::setupProduct reply caught exception\n" );
			}
		}

		void setupMeasureTask(Receiver &receiver)
		{
			try {
				PocoUUID measureTaskID; receiver.deMarshal(measureTaskID);
				getServer()->setupMeasureTask( measureTaskID );
				receiver.reply();
			} catch (...) {
				wmLog( eError, "TDbNotification<MsgHandler>::setupMeasureTask caught exception\n" );
			}
		}

		void setupFilterParameter(Receiver &receiver)
		{
			try {
				PocoUUID measureTaskID; receiver.deMarshal(measureTaskID);
				PocoUUID filterID; receiver.deMarshal(filterID);
				getServer()->setupFilterParameter( measureTaskID, filterID );
				receiver.reply();
			} catch (...) {
				wmLog( eError, "TDbNotification<MsgHandler>::setupFilterParameter caught exception\n" );
			}
		}

		void setupHardwareParameter(Receiver &receiver)
		{
			try {
				PocoUUID hwParametersatzID; receiver.deMarshal(hwParametersatzID);
				Key key; receiver.deMarshal(key);
				getServer()->setupHardwareParameter( hwParametersatzID, key );
				receiver.reply();
			} catch (...) {
				wmLog( eError, "TDbNotification<MsgHandler>::setupHardwareParameter caught exception\n" );
			}
		}

		void resetCalibration(Receiver &receiver)
		{
			int sensorId;
			try {
				receiver.deMarshal(sensorId);
			} catch (...) {
				wmLog( eError, "TDbNotification<MsgHandler>::resetCalibration deMarshal caught exception\n" );
			}
			try {
				getServer()->resetCalibration(sensorId);
			} catch (...) {
			    wmLog( eError, "TDbNotification<MsgHandler>::resetCalibration caught exception\n" );
			}
			try {
				receiver.reply();
			} catch (...) {
				wmLog( eError, "TDbNotification<MsgHandler>::resetCalibration reply caught exception\n" );
			}
		}


	private:
		TDbNotification<AbstractInterface> * getServer()
		{
			return server_;
		}




	};

} // namespace interface
} // namespace precitec


#endif /* DBNOTIFICATION_HANDLER_H_ */
