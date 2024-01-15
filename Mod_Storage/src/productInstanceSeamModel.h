#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QUuid>

#include <vector>

namespace precitec
{
namespace storage
{
class SeamSeries;

class ProductInstanceSeamModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * QFileInfo pointing to the directory of the product instance for which all Seams should be found
     **/
    Q_PROPERTY(QFileInfo productInstance READ productInstance WRITE setProductInstance NOTIFY productInstanceChanged)

    Q_PROPERTY(precitec::storage::SeamSeries *seamSeries READ seamSeries WRITE setSeamSeries NOTIFY seamSeriesChanged)
public:
    explicit ProductInstanceSeamModel(QObject *parent = nullptr);
    ~ProductInstanceSeamModel() override;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QFileInfo productInstance() const
    {
        return m_productInstance;
    }
    void setProductInstance(const QFileInfo &info);

    SeamSeries *seamSeries() const
    {
        return m_seamSeries;
    }
    void setSeamSeries(SeamSeries *series);

    enum class State {
        Unknown,
        Nio,
        Io
    };
    Q_ENUM(State)

Q_SIGNALS:
    void productInstanceChanged();
    void seamSeriesChanged();
    void metaDataLoadFinished();

private:
    void update();
    void loadMetaData(const QFileInfo &dir, int seamSeries, int seam);
    QFileInfo m_productInstance;
    SeamSeries *m_seamSeries = nullptr;
    QMetaObject::Connection m_seamSeriesDestroyedConnection;
    std::size_t m_metaDataLoadCount = 0;
    struct SeamData {
        int seamSeries = 0;
        int number = 0;
        State nio = State::Unknown;
        bool nioResultsSwitchedOff = false;
        int length = -1;
        QUuid uuid;
    };
    std::vector<SeamData> m_seams;
};

}
}
