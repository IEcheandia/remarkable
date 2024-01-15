#pragma once
#include "abstractAnalogInOutController.h"

#include "common/ethercat.h"

#include <QObject>
#include <QModelIndex>
#include <QPointF>

#include <memory>

class QMutex;

namespace precitec
{
namespace gui
{

class ServiceToGuiServer;

namespace components
{
namespace ethercat
{

class SlaveInfoModel;

/**
 * The AnalogOutController takes ethercat data from a SlaveInfoModel (@link{model}) at the
 * given @link{index} and provides the output data of the two analog out channels.
 * Furthermore it provides API to update the value of the two channels.
 **/
class AnalogOutController : public AbstractAnalogInOutController
{
    Q_OBJECT
    /**
     * Current value of channel 1
     * In order to update the value on the EtherCAT slave use @link{setChannel1}.
     **/
    Q_PROPERTY(qreal channel1 READ channel1 NOTIFY channel1Changed)
    /**
     * Current value of channel 2
     * In order to update the value on the EtherCAT slave use @link{setChannel2}.
     **/
    Q_PROPERTY(qreal channel2 READ channel2 NOTIFY channel2Changed)
public:
    AnalogOutController(QObject *parent = nullptr);
    ~AnalogOutController() override;

    qreal channel1() const
    {
        return m_channel1;
    }
    qreal channel2() const
    {
        return m_channel2;
    }

    /**
     * Sets the new output @p value of channel 1.
     **/
    Q_INVOKABLE void setChannel1(qreal value);

    /**
     * Sets the new output @p value of channel 2.
     **/
    Q_INVOKABLE void setChannel2(qreal value);

Q_SIGNALS:
    void channel1Changed();
    void channel2Changed();

protected:
    void fetchData() override;

private:
    void setChannel(qreal value, int channel);
    qreal m_channel1 = 0.0;
    qreal m_channel2 = 0.0;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::AnalogOutController*)
