#pragma once

#include <QAbstractListModel>
#include <QPointer>
#include <vector>

#include "product.h"
#include "seam.h"

namespace precitec
{

namespace storage
{
class ResultsServer;
}

namespace gui
{

/**
 * Model providing information about seams being inspected.
 * Requires the @link{resultsServer} to be set. Once an inspection is started provides the
 * currently inspected @link{product} and the model exposes information about the seams.
 *
 * The model provides the following roles:
 * @li seamName: the name of the Seam
 * @li position: the position of the Seam in the assembly image (as QPointF), if not set it's @c -1x-1
 * @li current: boolean whether the Seam is currently being inspected
 * @li state: AssemblyImageInspectionModel::SeamState
 *
 **/
class AssemblyImageInspectionModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The currently inspected or last inspected (if there is currently no inspection) Product
     **/
    Q_PROPERTY(precitec::storage::Product *product READ product NOTIFY productChanged)
    /**
     * The ResultsServer to get the results from.
     **/
    Q_PROPERTY(precitec::storage::ResultsServer *resultsServer READ resultsServer WRITE setResultsServer NOTIFY resultsServerChanged)
public:

    /**
     * The state of the Seam
     **/
    enum class SeamState
    {
        /**
         * Not yet inspected
         **/
        Uninspected,
        /**
         * No NIO
         **/
        Success,
        /**
         * Nio
         **/
        Failure
    };
    Q_ENUM(SeamState)

    explicit AssemblyImageInspectionModel(QObject *parent = nullptr);
    ~AssemblyImageInspectionModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setResultsServer(precitec::storage::ResultsServer *server);
    precitec::storage::ResultsServer *resultsServer() const
    {
        return m_resultsServer;
    }

    precitec::storage::Product *product() const
    {
        return m_product;
    }

    struct Data {
        QPointer<precitec::storage::Seam> seam;
        SeamState state;
    };

Q_SIGNALS:
    void resultsServerChanged();
    void productChanged();

private:
    void init(QPointer<precitec::storage::Product> product);
    void seamInspectionStarted(QPointer<precitec::storage::Seam> seam);
    void seamInspectionEnded();
    void nio();
    QPointer<precitec::storage::Product> m_product;
    QPointer<precitec::storage::Seam> m_currentInpsectedSeam;
    std::vector<Data> m_seams;
    precitec::storage::ResultsServer *m_resultsServer = nullptr;
    QMetaObject::Connection m_resultsServerDestroyedConnection;
    QMetaObject::Connection m_productInstanceStarted;
    QMetaObject::Connection m_nio;
    QMetaObject::Connection m_seamInspectionStarted;
    QMetaObject::Connection m_seamInspectionEnded;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::AssemblyImageInspectionModel*)
