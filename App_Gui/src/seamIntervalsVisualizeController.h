#pragma once

#include <QAbstractListModel>
#include <QColor>

namespace precitec
{
namespace storage
{

class Seam;
class SeamInterval;

}
namespace gui
{

class SeamIntervalsVisualizeController : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Seam *currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

public:
    explicit SeamIntervalsVisualizeController(QObject* parent = nullptr);
    ~SeamIntervalsVisualizeController() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam *seam);

    Q_INVOKABLE QColor colorForLevel(int level) const;

Q_SIGNALS:
    void currentSeamChanged();

private:
    void updateModel();
    double accumulatedLength(int index) const;

    std::vector<precitec::storage::SeamInterval*> m_intervals;

    precitec::storage::Seam* m_currentSeam = nullptr;
    QMetaObject::Connection m_seamDestroyConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::SeamIntervalsVisualizeController*)
