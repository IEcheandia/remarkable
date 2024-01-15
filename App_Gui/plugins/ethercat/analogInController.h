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

namespace plotter
{
class DataSet;
}

namespace ethercat
{

class SlaveInfoModel;

/**
 * The AnalogInController takes ethercat data from a SlaveInfoModel (@link{model}) at the
 * given @link{index} and fills two DataSet's with the analog data.
 **/
class AnalogInController : public AbstractAnalogInOutController
{
    Q_OBJECT
    /**
     * DataSet for first channel
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet *channel1 READ channel1 CONSTANT)
    /**
     * DataSet for second channel
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet *channel2 READ channel2 CONSTANT)
    /**
     * Whether this controller is monitoring an oversampling EtherCAT slave.
     **/
    Q_PROPERTY(bool oversampling READ isOversampling NOTIFY oversamplingChanged)
    /**
     * The latest value for channel 1
     **/
    Q_PROPERTY(qreal channel1CurrentValue READ channel1CurrentValue NOTIFY channel1CurrentValueChanged)
    /**
     * The latest value for channel 2
     **/
    Q_PROPERTY(qreal channel2CurrentValue READ channel2CurrentValue NOTIFY channel2CurrentValueChanged)
public:
    AnalogInController(QObject *parent = nullptr);
    ~AnalogInController() override;

    components::plotter::DataSet *channel1() const
    {
        return m_channel1DataSet;
    }
    components::plotter::DataSet *channel2() const
    {
        return m_channel2DataSet;
    }

    bool isOversampling() const
    {
        return m_oversampling;
    }

    qreal channel1CurrentValue() const
    {
        return m_channel1CurrentValue;
    }

    qreal channel2CurrentValue() const
    {
        return m_channel2CurrentValue;
    }

    /**
     * Clears the data in the channels' DataSet
     **/
    Q_INVOKABLE void clear();

Q_SIGNALS:
    void oversamplingChanged();
    void channel1CurrentValueChanged();
    void channel2CurrentValueChanged();
    /**
     * internal signal
     **/
    void newData();

protected:
    void fetchData() override;

private:
    void fetchNormal();
    void fetchOversampling();
    void updateDataSet();
    quint32 m_timeStamp = 0;
    std::vector<QVector2D> m_channel1;
    std::vector<QVector2D> m_channel2;
    components::plotter::DataSet *m_channel1DataSet;
    components::plotter::DataSet *m_channel2DataSet;
    bool m_oversampling = false;
    qreal m_channel1CurrentValue = 0.0;
    qreal m_channel2CurrentValue = 0.0;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::AnalogInController*)
