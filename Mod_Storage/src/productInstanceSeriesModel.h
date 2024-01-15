#pragma once

#include <QAbstractListModel>
#include <QFileInfo>

#include <vector>

namespace precitec
{
namespace storage
{

class ProductInstanceSeriesModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * QFileInfo pointing to the directory of the product instance for which all Seams should be found
     **/
    Q_PROPERTY(QFileInfo productInstance READ productInstance WRITE setProductInstance NOTIFY productInstanceChanged)

public:
    explicit ProductInstanceSeriesModel(QObject *parent = nullptr);
    ~ProductInstanceSeriesModel() override;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QFileInfo productInstance() const
    {
        return m_productInstance;
    }
    void setProductInstance(const QFileInfo &info);

    enum class State {
        Unknown,
        Nio,
        Io
    };
    Q_ENUM(State)

Q_SIGNALS:
    void productInstanceChanged();

private:
    void update();
    void loadMetaData(const QFileInfo &dir, int seamSeries);
    QFileInfo m_productInstance;
    struct SeriesData {
        int number = 0;
        State nio = State::Unknown;
        bool nioResultsSwitchedOff = false;
    };
    std::vector<SeriesData> m_series;
};

}
}

