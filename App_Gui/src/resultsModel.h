#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace gui
{

class ResultsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int level READ level NOTIFY levelChanged)

public:
    enum class ResultsComponent {
        Product,
        Instance,
        Series,
        Seam,
        Results
    };
    Q_ENUM(ResultsComponent)

    ResultsModel(QObject *parent = nullptr);
    ~ResultsModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    int level() const
    {
        return int(m_activeLevel);
    }

    Q_INVOKABLE void setActiveLevel(ResultsComponent level);

Q_SIGNALS:
    void levelChanged();

private:
    QString name(ResultsComponent component) const;
    QString iconName(ResultsComponent component) const;

    ResultsComponent m_activeLevel = ResultsComponent::Product;
};

}
}

