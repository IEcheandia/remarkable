#pragma once

#include <QAbstractItemModel>

namespace precitec
{

namespace gui
{

/**
 * Model for the four error groups
 **/
class ErrorGroupModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ErrorGroupModel(QObject *parent = nullptr);
    ~ErrorGroupModel() override;

    enum class ErrorGroup {
        LengthOutsideBoundary,
        LengthInsideBoundary,
        AreaOutsideBoundary,
        PeakOutsideBoundary
    };
    Q_ENUM(ErrorGroup)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ErrorGroupModel*)


