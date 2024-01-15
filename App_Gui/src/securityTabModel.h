#pragma once

#include <QAbstractListModel>

#include <vector>

namespace precitec
{
namespace gui
{

/**
 * Base model to set security aware tab names.
 **/
class SecurityTabModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ~SecurityTabModel() override;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

protected:
    struct Data {
        QString name;
        QString icon;
        std::vector<int> permissions;
        bool available = true;
    };
    explicit SecurityTabModel(std::initializer_list<Data> init, QObject *parent = nullptr);

private:
    std::vector<Data> m_elements;
};

}
}
