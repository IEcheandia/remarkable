/*
 * dbNotificationServer.h
 *
 *  Created on: 29.06.2010
 *      Author: Administrator
 */

#ifndef DBNOTIFICATIONSERVER_H_
#define DBNOTIFICATIONSERVER_H_

#include "event/dbNotification.interface.h"
#include "TCPCommunication/TCPCommunication.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

    // Datenbankaenderungen werden ueber diese Schnittstelle gemeldet.
    class DbNotificationServer : public TDbNotification<AbstractInterface>
    {
        public:
            DbNotificationServer(TCPCommunication& p_rTCPCommunication);
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
            TCPCommunication& m_rTCPCommunication;

    };

} // tcpcommunication
} // precitec

#endif /* DBNOTIFICATIONSERVER_H_ */

