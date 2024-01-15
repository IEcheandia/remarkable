#include "scanTrackerInformation.h"
#include "weldHeadServer.h"

#include <QTimer>

#include <functional>

namespace precitec
{
namespace gui
{

ScanTrackerInformation::ScanTrackerInformation(QObject* parent)
    : QObject(parent)
    , m_pollHeadInfo(new QTimer{this})
{
    m_pollHeadInfo->setInterval(std::chrono::milliseconds(100));
    connect(m_pollHeadInfo, &QTimer::timeout, this,
        [this]
        {
            if (!m_weldHeadSubscribeProxy)
            {
                return;
            }
            m_weldHeadSubscribeProxy->RequestHeadInfo(precitec::interface::eAxisTracker);
        }
    );
}

ScanTrackerInformation::~ScanTrackerInformation() = default;

void ScanTrackerInformation::setWeldHeadServer(precitec::gui::WeldHeadServer* server)
{
    if (m_weldHeadServer == server)
    {
        return;
    }
    disconnect(m_weldHeadServerDestroyed);
    disconnect(m_trackerChanged);
    m_weldHeadServer = server;
    if (m_weldHeadServer)
    {
        m_weldHeadServerDestroyed = connect(m_weldHeadServer, &QObject::destroyed, this, std::bind(&ScanTrackerInformation::setWeldHeadServer, this, nullptr));
        m_trackerChanged = connect(m_weldHeadServer, &WeldHeadServer::scanTrackerChanged, this,
            [this]
            {
                if (!m_weldHeadServer)
                {
                    return;
                }
                const auto &headInfo = m_weldHeadServer->scanTracker();
                if (headInfo.m_oSoftLimitsActive != m_expertMode)
                {
                    m_expertMode = headInfo.m_oSoftLimitsActive;
                    emit expertModeChanged();
                }
                if (headInfo.m_oScanWidthUMSent != m_scanWidthUm)
                {
                    m_scanWidthUm = headInfo.m_oScanWidthUMSent;
                    emit scanWidthUmChanged();
                }
                if (headInfo.m_oScanWidthVoltSent != m_scanWidthVolt)
                {
                    m_scanWidthVolt = headInfo.m_oScanWidthVoltSent;
                    emit scanWidthVoltChanged();
                }
                if (headInfo.m_oScanPosUMSent != m_scanPosUm)
                {
                    m_scanPosUm = headInfo.m_oScanPosUMSent;
                    emit scanPosUmChanged();
                }
                if (headInfo.m_oScanPosVoltSent != m_scanPosVolt)
                {
                    m_scanPosVolt = headInfo.m_oScanPosVoltSent;
                    emit scanPosVoltChanged();
                }
                if ((headInfo.m_oAxisStatusBits & 0x04) != m_scanWidthLimited)
                {
                    m_scanWidthLimited = headInfo.m_oAxisStatusBits & 0x04;
                    emit scanWidthLimitedChanged();
                }
                if ((headInfo.m_oAxisStatusBits & 0x08) != m_scanPosLimited)
                {
                    m_scanPosLimited = headInfo.m_oAxisStatusBits & 0x08;
                    emit scanPosLimitedChanged();
                }
            }, Qt::QueuedConnection);
    } else
    {
        m_weldHeadServerDestroyed = QMetaObject::Connection{};
        m_trackerChanged = QMetaObject::Connection{};
    }
    emit weldHeadServerChanged();

}

void ScanTrackerInformation::setWeldHeadSubscribeProxy(const WeldHeadSubscribeProxy &proxy)
{
    if (m_weldHeadSubscribeProxy == proxy)
    {
        return;
    }
    m_weldHeadSubscribeProxy = proxy;
    emit weldHeadSubscribeProxyChanged();
}

bool ScanTrackerInformation::isPollingHeadInfo() const
{
    return m_pollHeadInfo->isActive();
}

void ScanTrackerInformation::setPollHeadInfo(bool set)
{
    if (m_pollHeadInfo->isActive() == set)
    {
        return;
    }
    set ? m_pollHeadInfo->start() : m_pollHeadInfo->stop();
    emit pollHeadInfoChanged();
}

}
}
