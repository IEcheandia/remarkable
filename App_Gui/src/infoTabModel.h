#pragma once

#include <QAbstractListModel>

#include "abstractGraphModel.h"
#include "overlay/overlayPrimitive.h"

namespace precitec
{
namespace gui
{

class InfoTabModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(std::vector<precitec::image::OverlayText> data READ data WRITE setData NOTIFY dataChanged)

    Q_PROPERTY(std::vector<fliplib::InstanceFilter> filterInstances READ filterInstances WRITE setFilterInstances NOTIFY filterInstancesChanged)

public:
    explicit InfoTabModel(QObject *parent = nullptr);
    ~InfoTabModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    std::vector<precitec::image::OverlayText> data() const
    {
        return m_data;
    }
    void setData(const std::vector<precitec::image::OverlayText> data);

    std::vector<fliplib::InstanceFilter> filterInstances() const
    {
        return m_filterInstances;
    }
    void setFilterInstances(const std::vector<fliplib::InstanceFilter> instances);

Q_SIGNALS:
    void dataChanged();
    void filterInstancesChanged();

private:
    void updateModel();

    QString filterName(const QUuid &id);
    QStringList m_texts;

    std::vector<precitec::image::OverlayText> m_data;
    std::vector<fliplib::InstanceFilter> m_filterInstances;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::InfoTabModel*)
Q_DECLARE_METATYPE(std::vector<fliplib::InstanceFilter>)



