#pragma once

#include "message/device.h"
#include "hardwareParameters.h"

#include <QPointer>
#include <QSortFilterProxyModel>

namespace precitec
{

namespace storage
{
class ParameterSet;
}

namespace gui
{

class DeviceProxyWrapper;

/**
 * Filter model to restrict the hardware parameters to show from a HardwareParameterModel.
 **/
class HardwareParameterFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The HardwareParameterModel::Key to filter on. Exposed as QVariantList property so that
     * the Qml side can set the filter.
     **/
    Q_PROPERTY(QVariantList filterKeys READ filterKeys WRITE setFilterKeys NOTIFY filterKeysChanged)
    /**
     * The device proxy to the service device required for checking whether the filter keys are enabled
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *deviceProxy READ deviceProxy WRITE setDeviceProxy NOTIFY deviceProxyChanged)
    /**
     * The device proxy to the weld head device required for check how many LEDs are available
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *weldHeadDeviceProxy READ weldHeadDeviceProxy WRITE setWeldHeadDeviceProxy NOTIFY weldHeadDeviceProxyChanged)
public:
    explicit HardwareParameterFilterModel(QObject *parent = nullptr);
    ~HardwareParameterFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    void setFilterKeys(const QVariantList &filterKeys);
    QVariantList filterKeys() const;
    bool filterLEDKey(HardwareParameters::Key key) const;

    DeviceProxyWrapper *deviceProxy() const;
    void setDeviceProxy(DeviceProxyWrapper *deviceProxy);

    DeviceProxyWrapper *weldHeadDeviceProxy() const;
    void setWeldHeadDeviceProxy(DeviceProxyWrapper *deviceProxy);

    /**
     * Filters the passed in @p parameterSet for the keys enabled in this HardwareParameterFilterModel.
     * Any hardware Parameter present in the @p parameterSet, but not enabled through the filter gets
     * removed.
     **/
    void filterPrameterSet(precitec::storage::ParameterSet *parameterSet) const;

Q_SIGNALS:
    void filterKeysChanged();
    void deviceProxyChanged();
    void weldHeadDeviceProxyChanged();
    void ledTypeChanged();

private:
    void setLedType(int type);

    int m_ledType = 0;
    QVector<HardwareParameters::Key> m_filterKeys;
    precitec::interface::Configuration m_deviceConfiguration;
    QPointer<DeviceProxyWrapper> m_deviceProxy;
    QPointer<DeviceProxyWrapper> m_weldHeadDeviceProxy;
};

}
}
