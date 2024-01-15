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

class InvalidIDModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::PlausibilityController* plausibilityController READ plausibilityController WRITE setPlausibilityController NOTIFY plausibilityControllerChanged)

public:
    explicit InvalidIDModel(QObject *parent = nullptr);
    ~InvalidIDModel() override;

    PlausibilityController* plausibilityController() const
    {
        return m_plausibilityController;
    }
    void setPlausibilityController(PlausibilityController* plausibilityController);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

Q_SIGNALS:
    void plausibilityControllerChanged();

private:
    PlausibilityController* m_plausibilityController;
    QMetaObject::Connection m_destroyedConnection;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::InvalidIDModel*)

