#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * @brief Filter proxy model to display only the LWM Result Types
 **/
class LwmResultFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    LwmResultFilterModel(QObject* parent = nullptr);
    ~LwmResultFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LwmResultFilterModel*)


