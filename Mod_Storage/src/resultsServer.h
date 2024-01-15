#pragma once
#include "event/results.interface.h"
#include "product.h"
#include "seam.h"

#include <QObject>
#include <QPointer>

Q_DECLARE_METATYPE(precitec::interface::ResultArgs)
Q_DECLARE_METATYPE(std::vector<precitec::interface::ResultDoubleArray>)

namespace precitec
{

namespace storage
{
class ProductModel;

class ResultsServer : public QObject, public precitec::interface::TResults<precitec::interface::AbstractInterface>
{
    Q_OBJECT
public:
    explicit ResultsServer(QObject *parent = nullptr);
    ~ResultsServer() override;

    void result(precitec::interface::ResultIntArray const& value) override;
    void result(precitec::interface::ResultDoubleArray const& value) override;
    void result(precitec::interface::ResultRangeArray const& value) override;
    void result(precitec::interface::ResultRange1dArray const& value) override;
    void result(precitec::interface::ResultPointArray const& value) override;
    void result(const std::vector<precitec::interface::ResultDoubleArray> &results) override;
    void nio(precitec::interface::NIOResult const& result) override;
    void inspectionStarted(Poco::UUID productID, Poco::UUID instanceProductID, uint32_t productNr, precitec::interface::MeasureTaskIDs measureTaskIDs, precitec::interface::IntRange range, int p_oSeamNo, int triggerDeltaInMicrons, Poco::UUID seamId, const std::string &seamLabel) override;
    void inspectionEnded(precitec::interface::MeasureTaskIDs measureTaskIDs) override;
    void inspectionAutomaticStart(Poco::UUID productID, Poco::UUID instanceProductID, const std::string &extendedProductInfo) override;
    void inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID) override;

    /**
     * Sets the ProductModel this ResultsServer uses for mapping the events to Products and Seams.
     **/
    void setProducts(const std::shared_ptr<precitec::storage::ProductModel> &products)
    {
        m_products = products;
    }

Q_SIGNALS:
    /**
     * Emitted whenever a new result was received
     **/
    void resultsReceived(const precitec::interface::ResultArgs &);

    /**
     * Emitted whenever multiple results mere received together
     **/
    void combinedResultsReceived(const std::vector<precitec::interface::ResultDoubleArray> &results);

    /**
     * Emitted whenever a new nio result was received
     **/
    void nioReceived(const precitec::interface::ResultArgs &);
    /**
     * Emitted whenever inspectionStarted was invoked.
     *
     * If the Seam is not known the signal is not emitted
     *
     * @param seam The Seam whose inspection got started
     * @param productInstance The instance of the product which is going to be inspected
     * @param serialNumber The serialNumber of the product instance
     *
     * @see setProducts
     **/
    void seamInspectionStarted(QPointer<precitec::storage::Seam> seam, const QUuid &productInstance, quint32 serialNumber);
    /**
     * Emitted whenever an inspectionEnded was invoked.
     **/
    void seamInspectionEnded();

    /**
     * Emitted whenever inspectionAutomaticStart was invoked.
     *
     * If the Product is not known the signal is not emitted
     *
     * @param product The Product whose inspection got started
     * @param productInstance The instance of the product which is going to be inspected
     * @param extendedProductInfo The extended product information such as serial number or product type
     *
     * @see setProduct
     **/
    void productInspectionStarted(QPointer<precitec::storage::Product> product, const QUuid &productInstance, const QString &extendedProductInfo);

    /**
     * Emitted whenever inspectionAutomaticStop was invoked.
     **/
    void productInspectionStopped(QPointer<precitec::storage::Product> product);

private:
    precitec::storage::Product *findProduct(const Poco::UUID &id) const;
    std::shared_ptr<precitec::storage::ProductModel> m_products;

};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsServer*)
