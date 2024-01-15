#pragma once

#include <QObject>
#include <QMutex>
#include <QUuid>

#include "event/dbNotification.proxy.h"

class QTimer;

namespace precitec
{
namespace storage
{

class ProductChangeNotifier : public QObject
{
    Q_OBJECT
public:
    ProductChangeNotifier(QObject *parent = nullptr);
    ~ProductChangeNotifier() override;

    void queue(const QUuid &id);

    void setDbNotification(const std::shared_ptr<precitec::interface::TDbNotification<precitec::interface::EventProxy>> &db);

private:
    void process();
    std::list<QUuid> m_uuids;
    QMutex m_mutex;
    QTimer *m_timer;
    std::shared_ptr<precitec::interface::TDbNotification<precitec::interface::EventProxy>> m_dbNotification;
};

}
}
