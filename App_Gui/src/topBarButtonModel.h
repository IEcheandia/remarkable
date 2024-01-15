#pragma once

#include <QAbstractListModel>
#include <QString>

namespace precitec
{

namespace gui
{

class TopBarButtonModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TopBarButtonModel( QObject* parent = nullptr);
    ~TopBarButtonModel() override;

    enum class TopBarButton {
        Overview,
        Login,
        Results,
        Statistics,
        Simulation,
        HeadMonitor,
        Wizard,
        Grapheditor,
        Configuration
    };
    Q_ENUM(TopBarButton);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE void setSimulationEnabled(bool value);

    Q_INVOKABLE QModelIndex indexForItem(precitec::gui::TopBarButtonModel::TopBarButton item) const;

private:
    QString text(TopBarButton topBarButton) const;
    QString iconSource(TopBarButton topBarButton) const;
    int permission(TopBarButton topBarButton) const;
    bool enabled(TopBarButton topBarButton) const;
    QString objectName(TopBarButton topBarButton) const;

    bool m_simulationEnabled{false};
};

}
}
Q_DECLARE_METATYPE(precitec::gui::TopBarButtonModel*)
