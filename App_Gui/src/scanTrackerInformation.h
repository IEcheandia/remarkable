#pragma once

#include <QObject>
#include "event/viWeldHeadSubscribe.interface.h"

class QTimer;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TviWeldHeadSubscribe<precitec::interface::AbstractInterface>> WeldHeadSubscribeProxy;

namespace gui
{

class WeldHeadServer;

class ScanTrackerInformation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::WeldHeadServer *weldHeadServer READ weldHeadServer WRITE setWeldHeadServer NOTIFY weldHeadServerChanged)
    Q_PROPERTY(precitec::WeldHeadSubscribeProxy weldHeadSubscribeProxy READ weldHeadSubscribeProxy WRITE setWeldHeadSubscribeProxy NOTIFY weldHeadSubscribeProxyChanged)
    Q_PROPERTY(int scanPosUm READ scanPosUm NOTIFY scanPosUmChanged)
    Q_PROPERTY(int scanPosVolt READ scanPosVolt NOTIFY scanPosVoltChanged)
    Q_PROPERTY(int scanWidthUm READ scanWidthUm NOTIFY scanWidthUmChanged)
    Q_PROPERTY(int scanWidthVolt READ scanWidthVolt NOTIFY scanWidthVoltChanged)
    Q_PROPERTY(bool expertMode READ isExpertMode NOTIFY expertModeChanged)
    Q_PROPERTY(bool scanPosLimited READ scanPosLimited NOTIFY scanPosLimitedChanged)
    Q_PROPERTY(bool scanWidthLimited READ scanWidthLimited NOTIFY scanWidthLimitedChanged)
    /**
     * When set to @c true the head info gets polled every 100 msec.
     **/
    Q_PROPERTY(bool pollHeadInfo READ isPollingHeadInfo WRITE setPollHeadInfo NOTIFY pollHeadInfoChanged)
public:
    ScanTrackerInformation(QObject *parent = nullptr);
    ~ScanTrackerInformation() override;

    WeldHeadServer *weldHeadServer() const
    {
        return m_weldHeadServer;
    }
    void setWeldHeadServer(WeldHeadServer *server);

    int scanPosUm() const
    {
        return m_scanPosUm;
    }
    int scanPosVolt() const
    {
        return m_scanPosVolt;
    }
    int scanWidthUm() const
    {
        return m_scanWidthUm;
    }
    int scanWidthVolt() const
    {
        return m_scanWidthVolt;
    }

    bool isExpertMode() const
    {
        return m_expertMode;
    }

    bool scanWidthLimited() const
    {
        return m_scanWidthLimited;
    }

    bool scanPosLimited() const
    {
        return m_scanPosLimited;
    }

    WeldHeadSubscribeProxy weldHeadSubscribeProxy() const
    {
        return m_weldHeadSubscribeProxy;
    }
    void setWeldHeadSubscribeProxy(const WeldHeadSubscribeProxy &proxy);

    bool isPollingHeadInfo() const;
    void setPollHeadInfo(bool set);

Q_SIGNALS:
    void weldHeadServerChanged();
    void scanPosUmChanged();
    void scanPosVoltChanged();
    void scanWidthUmChanged();
    void scanWidthVoltChanged();
    void expertModeChanged();
    void scanPosLimitedChanged();
    void scanWidthLimitedChanged();
    void weldHeadSubscribeProxyChanged();
    void pollHeadInfoChanged();

private:
    QTimer *m_pollHeadInfo;
    WeldHeadServer *m_weldHeadServer = nullptr;
    WeldHeadSubscribeProxy m_weldHeadSubscribeProxy;
    QMetaObject::Connection m_weldHeadServerDestroyed;
    QMetaObject::Connection m_trackerChanged;
    bool m_expertMode = false;
    int m_scanPosUm = 0;
    int m_scanPosVolt = 0;
    int m_scanWidthUm = 0;
    int m_scanWidthVolt = 0;
    bool m_scanWidthLimited = false;
    bool m_scanPosLimited = false;
};

}
}
