#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace storage
{

class LaserControlPreset;

}
namespace gui
{

class LaserControlPresetModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::LaserControlPreset* preset READ preset WRITE setPreset NOTIFY presetChanged)

    Q_PROPERTY(Channel displayChannel READ displayChannel WRITE setDisplayChannel NOTIFY displayChannelChanged)

public:
    explicit LaserControlPresetModel(QObject *parent = nullptr);
    ~LaserControlPresetModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    enum class Channel {
        One,
        Two,
        Both
    };
    Q_ENUM(Channel)

    precitec::storage::LaserControlPreset* preset() const
    {
        return m_preset;
    }
    void setPreset(precitec::storage::LaserControlPreset* preset);

    Channel displayChannel() const
    {
        return m_displayChannel;
    }
    void setDisplayChannel(Channel displayChannel);

Q_SIGNALS:
    void presetChanged();
    void displayChannelChanged();

private:
    Channel m_displayChannel = Channel::One;
    precitec::storage::LaserControlPreset* m_preset = nullptr;
    QMetaObject::Connection m_destroyedConnection;
    QMetaObject::Connection m_powerConnection;
    QMetaObject::Connection m_offsetConnection;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::LaserControlPresetModel*)
