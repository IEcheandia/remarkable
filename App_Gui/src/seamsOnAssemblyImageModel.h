#pragma once

#include <QAbstractListModel>
#include <QPointer>

namespace precitec
{

namespace storage
{
class Seam;
}

namespace gui
{

/**
 * This model provides information about the positions of all Seams in an assembly image.
 * Further more it is intended to be used for setting the position of a selected Seam.
 * For this the property @link{seam} needs to be specified. The model retrieves the information
 * for all seams in the product of the selected seam.
 *
 * To update the position of the seam use @link{setSeamPosition}, to save the changes use @link{saveChanges}.
 *
 * The model provides the following roles:
 * @li seamName: the name of the Seam
 * @li position: the position of the Seam in the assembly image (as QPointF), if not set it's @c -1x-1
 * @li current: boolean whether the Seam is the selected Seam of the model
 **/
class SeamsOnAssemblyImageModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The Seam for which the position in the assembly image should be set
     **/
    Q_PROPERTY(precitec::storage::Seam *seam READ seam WRITE setSeam NOTIFY seamChanged)
public:
    explicit SeamsOnAssemblyImageModel(QObject *parent = nullptr);
    ~SeamsOnAssemblyImageModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::Seam *seam() const
    {
        return m_seam;
    }
    void setSeam(precitec::storage::Seam *seam);

    /**
     * Sets the new position of the current Seam to @p x and @p y.
     **/
    Q_INVOKABLE void setSeamPosition(qreal x, qreal y);

Q_SIGNALS:
    void seamChanged();
    void markAsChanged();

private:
    void initSeams();
    precitec::storage::Seam *m_seam = nullptr;
    std::vector<QPointer<precitec::storage::Seam>> m_allSeams;
    bool m_changes{false};
    QMetaObject::Connection m_productConnection;
    QMetaObject::Connection m_seamDestroyedConnection;
};

}
}
