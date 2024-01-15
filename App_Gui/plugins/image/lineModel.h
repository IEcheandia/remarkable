#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace gui
{

class LineModel : public QAbstractListModel
{
    Q_OBJECT

public:
    LineModel(QObject *parent = nullptr);
    ~LineModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LineModel*)
