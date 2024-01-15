#include "hardwareParametersOverviewSortFilterModel.h"

namespace precitec
{
namespace gui
{

HardwareParametersOverviewSortFilterModel::HardwareParametersOverviewSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(Qt::UserRole);
    sort(0);
}

HardwareParametersOverviewSortFilterModel::~HardwareParametersOverviewSortFilterModel() = default;

bool HardwareParametersOverviewSortFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    static const QRegularExpression numberAtEndOfKey{"\\d+$"};

    const auto label_left = sourceModel()->headerData(source_left.row(), Qt::Vertical, Qt::DisplayRole).toString();

    const auto label_right = sourceModel()->headerData(source_right.row(), Qt::Vertical, Qt::DisplayRole).toString();

    const auto& left_match = numberAtEndOfKey.match(label_left);
    const auto& right_match = numberAtEndOfKey.match(label_right);

    if (left_match.hasMatch() && right_match.hasMatch())
    {
        const auto& number_left = left_match.captured(0);
        const auto& number_right = right_match.captured(0);

        auto string_left = label_left;
        string_left.remove(number_left);

        auto string_right = label_right;
        string_right.remove(number_right);

        if (string_left == string_right)
        {
            return number_left.toInt() < number_right.toInt();
        }
    }

    return label_left.compare(label_right, Qt::CaseInsensitive) <= 0;
}

}
}
