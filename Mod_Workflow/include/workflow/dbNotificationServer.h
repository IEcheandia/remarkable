/*
 * dbNotificationServer.h
 *
 *  Created on: 29.06.2010
 *      Author: Administrator
 */

#ifndef DBNOTIFICATIONSERVER_H_
#define DBNOTIFICATIONSERVER_H_

#include "Mod_Workflow.h"

#include "event/dbNotification.interface.h"

#include "stateMachine/stateContext.h"

namespace precitec
{
	using namespace interface;

namespace workflow
{
	// Datenbankaenderungen werden ueber diese Schnittstelle gemeldet.
	class MOD_WORKFLOW_API DbNotificationServer : public TDbNotification<AbstractInterface>
	{
		public:
			DbNotificationServer(SmStateContext stateContext);
			virtual ~DbNotificationServer();

		public:
			// Die Produktaten wurden geaendert.
			virtual void setupProduct(const Poco::UUID& productID);
		
			// Die Messaufgaben wurden geaendert (inkl. Graphen)
			virtual void setupMeasureTask(const Poco::UUID& measureTaskID);

			// die Filter Parameter wurden geaendert
			virtual void setupFilterParameter(const Poco::UUID& measureTaskID, const Poco::UUID& filterID);

			virtual void setupHardwareParameter(const Poco::UUID& hwParameterSatzID, const Key key);

			virtual void resetCalibration(const int sensorId);

		private:
			SmStateContext stateContext_;

	};

}	// workflow
}	// precitec


#endif /* DBNOTIFICATIONSERVER_H_ */
