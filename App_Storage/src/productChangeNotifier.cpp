#include "productChangeNotifier.h"
#include "compatibility.h"

#include <QTimer>

namespace precitec
{
namespace storage
{

ProductChangeNotifier::ProductChangeNotifier(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer{this})
{
    m_timer->setSingleShot(true);
    m_timer->setInterval(std::chrono::milliseconds{100});
    connect(m_timer, &QTimer::timeout, this, &ProductChangeNotifier::process);
}

ProductChangeNotifier::~ProductChangeNotifier() = default;

void ProductChangeNotifier::process()
{
    QMutexLocker lock{&m_mutex};
    if (m_uuids.empty())
    {
        return;
    }
    const auto id{compatibility::toPoco(m_uuids.front())};
    m_uuids.pop_front();
    if (!m_uuids.empty())
    {
        m_timer->start();
    }
    lock.unlock();
    m_dbNotification->setupProduct(id);
}

void ProductChangeNotifier::queue(const QUuid& id)
{
    QMutexLocker lock{&m_mutex};
    if (std::find(m_uuids.begin(), m_uuids.end(), id) != m_uuids.end())
    {
        return;
    }
    m_uuids.push_back(id);
    if (!m_timer->isActive())
    {
        QMetaObject::invokeMethod(m_timer, "start", Qt::QueuedConnection);
    }
}


void ProductChangeNotifier::setDbNotification(const std::shared_ptr<precitec::interface::TDbNotification<precitec::interface::EventProxy>> &db)
{
    m_dbNotification = db;
}

}
}
