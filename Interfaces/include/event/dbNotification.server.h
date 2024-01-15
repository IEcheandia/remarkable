/*
 * dbNotification.server.h
 *
 *  Created on: 22.06.2010
 */

#ifndef DBNOTIFICATION_SERVER_H_
#define DBNOTIFICATION_SERVER_H_


#include <string>
#include <iostream>
#include <map> // wg HandlerList
#include "Poco/NamedMutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/Process.h"
#include "Poco/Path.h"

#include "event/dbNotification.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TDbNotification<EventServer> : public TDbNotification<AbstractInterface>
	{
	public:
		// Die Produktaten wurden geaendert.
		virtual void setupProduct(const Poco::UUID& productID)
		{
		}

		// Die Messaufgaben wurden geaendert (inkl. Graphen)
		virtual void setupMeasureTask(const UUID& measureTaskID)
		{
		}
		
		// die Filter Parameter wurden geaendert
		virtual void setupFilterParameter(const UUID& measureTaskID, const UUID& filterID)
		{
		}

		virtual void setupHardwareParameter(const UUID& hwParameterSatzID, const Key key)
		{
		}

		virtual void resetCalibration(const int sensorId)
		{
		}
	};


} // namespace interface
} // namespace precitec

#endif /* DBNOTIFICATION_SERVER_H_ */
