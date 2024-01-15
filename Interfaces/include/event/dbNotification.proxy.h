/*
 * dbNotification.proxy.h
 *
 *  Created on: 22.06.2010
 */

#ifndef DBNOTIFICATION_PROXY_H_
#define DBNOTIFICATION_PROXY_H_


#include  "event/dbNotification.interface.h"
#include  "server/eventProxy.h"


namespace precitec
{
namespace interface
{
	template <>
	class TDbNotification<EventProxy> : public Server<EventProxy>, public TDbNotification<AbstractInterface>, public TDbNotificationMessageDefinition
	{
	public:
		TDbNotification() : EVENT_PROXY_CTOR(TDbNotification), TDbNotification<AbstractInterface>()
		{
		}

		/// der DTor muss virtuell sein
		virtual ~TDbNotification() {}
	public:

		virtual void setupProduct(const Poco::UUID& productID)
		{
			INIT_EVENT(SetupProduct);
			signaler().marshal(productID);
			signaler().send();
		}

		virtual void setupMeasureTask(const Poco::UUID& measureTaskID)
		{
			INIT_EVENT(SetupMeasureTask);
			signaler().marshal(measureTaskID);
			signaler().send();
		}
		
		virtual void setupFilterParameter(const Poco::UUID& measureTaskID, const Poco::UUID& filterID)
		{
			INIT_EVENT(SetupFilterParameter);
			signaler().marshal(measureTaskID);
			signaler().marshal(filterID);
			signaler().send();
		}

		virtual void setupHardwareParameter(const Poco::UUID& hwParameterSatzID, const Key key)
		{
			INIT_EVENT(SetupHardwareParameter);
			signaler().marshal(hwParameterSatzID);
			signaler().marshal(key);
			signaler().send();
		}
		
		virtual void resetCalibration(const int sensorId)
		{
			INIT_EVENT(ResetCalibration);
			signaler().marshal(sensorId);
			signaler().send();
		}
		
		

	};


} // namespace interface
} // namespace precitec

#endif /* DBNOTIFICATION_PROXY_H_ */
