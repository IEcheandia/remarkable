#pragma once

#include <QAbstractItemModel>

class QUuid;

namespace precitec
{
namespace storage
{

class Seam;
class ReferenceCurve;

}
namespace gui
{

/**
 * @brief Model which holds all @link{ReferenceCurve}s of a @link{Seam}.
 *
 * This is a small model which functions as the data holder for the
 * @link{ReferenceCurve}s of a @link{Seam}. It provides the functionality
 * to create, copy or remove @link{ReferenceCurve}s, as well as display them
 * through the model's data roles.
 **/

class ReferenceCurvesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Seam* currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

public:
    explicit ReferenceCurvesModel(QObject* parent = nullptr);
    ~ReferenceCurvesModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam* seam);

    Q_INVOKABLE precitec::storage::ReferenceCurve* createReferenceCurve(int type);

    Q_INVOKABLE precitec::storage::ReferenceCurve* copyReferenceCurve(precitec::storage::ReferenceCurve* curve);

    Q_INVOKABLE void removeReferenceCurve(precitec::storage::ReferenceCurve* curve);

Q_SIGNALS:
    void currentSeamChanged();
    void markAsChanged();

private:
    precitec::storage::Seam* m_currentSeam = nullptr;
    QMetaObject::Connection m_seamDestroyConnection;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ReferenceCurvesModel*)

