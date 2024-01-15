#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

/**
 * Model providing input/output signal information for a digital ethercat slave (EL1018/EL2008).
 * Each row is one bit.
 *
 * The model provides the following roles:
 * @li bit: the boolean state of the bit at the given index
 **/
class DigitalSlaveModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The input/output data for this slave.
     **/
    Q_PROPERTY(QByteArray byteData READ byteData WRITE setByteData NOTIFY byteDataChanged)
public:
    DigitalSlaveModel(QObject *parent = nullptr);
    ~DigitalSlaveModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    QByteArray byteData() const
    {
        return m_data;
    }
    void setByteData(const QByteArray &data);

Q_SIGNALS:
    void byteDataChanged();

private:
    QByteArray m_data;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::DigitalSlaveModel*)
