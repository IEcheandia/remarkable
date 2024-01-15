#pragma once

#include <QAbstractListModel>
#include <bitset>

namespace precitec
{
namespace gui
{

class WeldHeadServer;

/**
 * The AxisStatusModel is able to provide the status flags of axis status word.
 * For this the weldHeadServer needs to be provided as a property.
 *
 * The model provideds two roles:
 * @li display (name of the status bit flag)
 * @li flag (boolean indicating whether the flag is set)
 *
 * Several flags are swapped to provide more meaningful values for gui operations.
 **/
class AxisStatusModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::WeldHeadServer *weldHeadServer READ weldHeadServer WRITE setWeldHeadServer NOTIFY weldHeadServerChanged)
public:
    explicit AxisStatusModel(QObject *parent = nullptr);
    ~AxisStatusModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    WeldHeadServer *weldHeadServer() const
    {
        return m_weldHeadServer;
    }
    void setWeldHeadServer(WeldHeadServer *server);

Q_SIGNALS:
    void weldHeadServerChanged();

private:
    WeldHeadServer *m_weldHeadServer = nullptr;
    QMetaObject::Connection m_weldHeadServerDestroyed;
    QMetaObject::Connection m_yAxisChanged;
    std::bitset<16> m_status;

};

}
}

Q_DECLARE_METATYPE(precitec::gui::AxisStatusModel*)
