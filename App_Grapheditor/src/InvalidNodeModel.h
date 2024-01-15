#pragma once

#include <QAbstractListModel>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{
class PlausibilityController;

class InvalidNodeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::PlausibilityController* plausibilityController READ plausibilityController WRITE setPlausibilityController NOTIFY plausibilityControllerChanged)

public:
    explicit InvalidNodeModel(QObject *parent = nullptr);
    ~InvalidNodeModel() override;

    PlausibilityController* plausibilityController() const;
    void setPlausibilityController(PlausibilityController* plausibilityController);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

Q_SIGNALS:
    void plausibilityControllerChanged();

private:
    PlausibilityController *m_plausibilityController;
    QMetaObject::Connection m_destroyConnection;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::InvalidNodeModel*)
