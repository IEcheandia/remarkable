#pragma once
#include "abstractDeviceKeyModel.h"
#include "message/device.interface.h"

namespace precitec
{
namespace gui
{

/**
 * The LineLaserModel provides information about line lasers available in the system
 * and allows to modify them.
 *
 * It provides the following roles:
 * @li display
 * @li enabled
 * @li intensity
 * @li minimum
 * @li maximum
 **/
class LineLaserModel : public AbstractDeviceKeyModel
{
    Q_OBJECT
public:
    explicit LineLaserModel(QObject *parent = nullptr);
    ~LineLaserModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Enables the line laser at @p index.
     * @param enable Whether the line laser should be enabled or disabled
     **/
    Q_INVOKABLE void enable(int index, bool enable);
    /**
     * Sets the @p intensity of the line laser at @p index.
     **/
    Q_INVOKABLE void setIntensity(int index, int intensity);

private:
    void init();
    void keyValueChanged(const QUuid &id, const precitec::interface::SmpKeyValue &kv);
    struct Data {
        precitec::interface::SmpKeyValue toggle;
        precitec::interface::SmpKeyValue intensity;
        QString name;
        bool updating = false;
    };
    std::vector<Data> m_data;
    QMetaObject::Connection m_notificationConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LineLaserModel*)
