#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QUuid>

#include <vector>

namespace precitec
{
namespace interface
{
class ResultArgs;
}

namespace storage
{

/**
 * The ResultsSeriesLoader is able to load results for a given SeamSearies in a given product instance directory.
 * It reads in every @c *.result file in that directory in an async manner and loads the ResultArgs in those files, paired with
 * with the seam numbers.
 **/
class ResultsSeriesLoader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int seamSeries READ seamSeries WRITE setSeamSeries NOTIFY seamSeriesChanged)

    Q_PROPERTY(QFileInfo productInstance READ productInstance WRITE setProductInstance NOTIFY productInstanceChanged)

    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

public:
    explicit ResultsSeriesLoader(QObject *parent = nullptr);
    ~ResultsSeriesLoader() override;

    int seamSeries() const
    {
        return m_seamSeries;
    }
    void setSeamSeries(int series);

    QFileInfo productInstance() const
    {
        return m_productInstance;
    }
    void setProductInstance(const QFileInfo &info);

    bool isLoading() const;

    struct SeamData
    {
        int number;
        QUuid uuid;
    };

    /**
     * @returns The loaded results
     **/
    const std::vector<std::pair<SeamData, std::vector<precitec::interface::ResultArgs>>>& results() const
    {
        return m_results;
    }

    /**
     * Takes the loaded results and transfers them to the caller.
     **/
    std::vector<std::pair<SeamData, std::vector<precitec::interface::ResultArgs>>>&& takeResults();

Q_SIGNALS:
    void seamSeriesChanged();
    void productInstanceChanged();
    void loadingChanged();
    void resultsLoaded();

private:
    void update();
    void decrementLoadCounter();

    QFileInfo m_productInstance;
    int m_seamSeries = -1;

    int m_loadCounter = 0;
    std::vector<std::pair<SeamData, std::vector<precitec::interface::ResultArgs>>> m_results;
    std::vector<QUuid> m_processedSeams;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsSeriesLoader*)

