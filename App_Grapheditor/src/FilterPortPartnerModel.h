#pragma once

#include <QAbstractListModel>
#include "FilterPort.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterPortPartnerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::FilterPort* actualPort READ actualPort WRITE setActualPort NOTIFY actualPortChanged)

public:
    explicit FilterPortPartnerModel(QObject *parent = nullptr);
    ~FilterPortPartnerModel() override;

    FilterPort* actualPort() const;
    void setActualPort(FilterPort* port);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QObject* getPartner(int index);

Q_SIGNALS:
    void actualPortChanged();

private:
    void init();

    QMetaObject::Connection m_destroyConnection;
    FilterPort* m_actualPort = nullptr;
    std::vector<FilterPort*> m_partners;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterPortPartnerModel*)
