#pragma once

#include "productStatistics.h"

#include <QObject>

namespace precitec
{
namespace storage
{

class Product;

class ResultsStatisticsController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Product* currentProduct READ currentProduct WRITE setCurrentProduct NOTIFY currentProductChanged)

    Q_PROPERTY(QString resultsStoragePath READ resultsStoragePath WRITE setResultsStoragePath NOTIFY resultsStoragePathChanged)

    Q_PROPERTY(bool calculating READ calculating NOTIFY calculatingChanged)

    Q_PROPERTY(bool empty READ empty NOTIFY update)

    Q_PROPERTY(unsigned int io READ io NOTIFY update)

    Q_PROPERTY(unsigned int nio READ nio NOTIFY update)

    Q_PROPERTY(double ioInPercent READ ioInPercent NOTIFY update)

    Q_PROPERTY(double nioInPercent READ nioInPercent NOTIFY update)

public:
    explicit ResultsStatisticsController(QObject* parent = nullptr);

    ~ResultsStatisticsController() override;

    Product* currentProduct() const
    {
        return m_currentProduct;
    }
    void setCurrentProduct(Product* product);

    QString resultsStoragePath() const
    {
        return m_resultsStoragePath;
    }
    void setResultsStoragePath(const QString& path);

    bool calculating() const
    {
        return m_calculating;
    }

    const ProductStatistics& productStatistics() const
    {
        return m_productStatistics;
    }

    bool empty() const
    {
        return m_productStatistics.empty();
    }

    unsigned int io() const
    {
        return m_productStatistics.ioCount();
    }

    unsigned int nio() const
    {
        return m_productStatistics.nioCount();
    }

    double ioInPercent() const
    {
        return m_productStatistics.ioInPercent();
    }

    double nioInPercent() const
    {
        return m_productStatistics.nioInPercent();
    }

    /**
     * Recursively iterates through product instances between @p startTime and @p endTime, their seam series and seams and accumulates io/nio information.
     **/
    Q_INVOKABLE void calculate(const QDate& startTime, const QDate& endTime);

Q_SIGNALS:
    void currentProductChanged();
    void resultsStoragePathChanged();
    void calculatingChanged();
    void update();

private:
    void setCalculating(bool calculating);
    void clear();

    Product* m_currentProduct = nullptr;
    QMetaObject::Connection m_productDestroyedConnection;

    bool m_calculating = false;

    ProductStatistics m_productStatistics;

    QString m_resultsStoragePath;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsStatisticsController*)
