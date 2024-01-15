#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

/**
 * Filter model for SlaveInfoModel to filter out ethercat slaves we are not interested in. E.g. EK1100 or Accelnet.
 **/
class SlaveInfoFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SlaveInfoFilterModel(QObject *parent = nullptr);
    ~SlaveInfoFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::SlaveInfoFilterModel*)
