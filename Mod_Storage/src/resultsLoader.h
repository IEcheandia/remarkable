#pragma once

#include <QAbstractListModel>
#include <QFileInfo>

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
 * The ResultsLoader is able to load results for a given Seam in a given product instance directory.
 * It reads in every @c *.result file in that directory in an async manner and loads the ResultArgs
 * in those files.
 **/
class ResultsLoader : public QObject
{
    Q_OBJECT
    /**
     * The Seam for which to load results in the product instance directory.
     **/
    Q_PROPERTY(int seam READ seam WRITE setSeam NOTIFY seamChanged)
    Q_PROPERTY(int seamSeries READ seamSeries WRITE setSeamSeries NOTIFY seamSeriesChanged)
    /**
     * The directory of the product instance.
     **/
    Q_PROPERTY(QFileInfo productInstance READ productInstance WRITE setProductInstance NOTIFY productInstanceChanged)
    /**
     * Whether this ResultsLoader is currently loading results
     **/
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
public:
    explicit ResultsLoader(QObject *parent = nullptr);
    ~ResultsLoader() override;

    void setSeam(int seam);
    int seam() const
    {
        return m_seam;
    }

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

    /**
     * @returns The loaded results
     **/
    const std::vector<std::vector<precitec::interface::ResultArgs>> &results() const
    {
        return m_results;
    }

    /**
     * Takes the loaded results and transfers them to the caller.
     **/
    std::vector<std::vector<precitec::interface::ResultArgs>> &&takeResults();

Q_SIGNALS:
    void seamChanged();
    void seamSeriesChanged();
    void productInstanceChanged();
    void loadingChanged();
    /**
     * Emitted once the results are fully loaded
     **/
    void resultsLoaded();

private:
    void update();
    void decrementLoadCounter();
    QFileInfo m_productInstance;
    int m_seam = -1;
    int m_loadCounter = 0;
    std::vector<std::vector<precitec::interface::ResultArgs>> m_results;
    int m_seamSeries = -1;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsLoader*)
