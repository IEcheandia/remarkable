#pragma once
#include "abstractDeviceKeyModel.h"
#include "message/device.interface.h"

#include <QAbstractListModel>
#include <QPointer>

#include <vector>

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TDevice<precitec::interface::AbstractInterface>> DeviceProxy;

namespace storage
{
class AttributeModel;
class ParameterSet;
}


namespace gui
{

class DeviceNotificationServer;
class DeviceProxyWrapper;

/**
 * The DeviceKeyModel holds all Device key-value pairs of a Device.
 *
 * It provides the following roles:
 * @li display - the key
 * @li comment - a comment describing the value
 * @li parameter - The Parameter describing current value of a key-value
 * @li attribute - The Attribute describing the key-value
 **/
class DeviceKeyModel : public AbstractDeviceKeyModel
{
    Q_OBJECT
    /**
     * Whether the model is currently loading the device keys
     **/
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

    /**
     * AttributeModel for hardware parameters (key value attributes).
     **/
    Q_PROPERTY(precitec::storage::AttributeModel* attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

public:
    explicit DeviceKeyModel(QObject *parent = nullptr);
    ~DeviceKeyModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isLoading() const
    {
        return m_loading;
    }

    enum class Types {
        Char,
        Byte,
        Int,
        UInt,
        Bool,
        Float,
        Double,
        String
    };
    Q_ENUM(Types)

    /**
     * Updates the KeyValue identified by @p row to the @p data value.
     **/
    Q_INVOKABLE void updateValue(int row, const QVariant &data);

    storage::AttributeModel* attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(storage::AttributeModel* model);

Q_SIGNALS:
    void loadingChanged();
    void restartRequired();
    void attributeModelChanged();

private:
    void init();
    void setLoading(bool set);
    void keyValueChanged(const QUuid &id, const precitec::interface::SmpKeyValue &kv);
    std::vector<precitec::interface::SmpKeyValue> m_keyValues;
    bool m_loading = false;
    QPointer<DeviceNotificationServer> m_notificationServer;
    QMetaObject::Connection m_notificationConnection;
    storage::ParameterSet* m_parameterSet;
    storage::AttributeModel* m_attributeModel{nullptr};
    QMetaObject::Connection m_attributeModelDestroyed;
};

}
}
