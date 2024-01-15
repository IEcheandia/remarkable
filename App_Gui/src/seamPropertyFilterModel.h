#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

class SeamPropertyFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool hasScanlabScanner READ hasScanlabScanner WRITE setScanlabScanner NOTIFY scanlabScannerChanged)

    Q_PROPERTY(bool hasLaserControl READ hasLaserControl WRITE setLaserControl NOTIFY laserControlChanged)

public:
    SeamPropertyFilterModel(QObject *parent = nullptr);
    ~SeamPropertyFilterModel() override;

    bool hasLaserControl() const
    {
        return m_laserControlAvailable;
    }
    void setLaserControl(bool value);

    bool hasScanlabScanner() const
    {
        return m_scanlabScannerAvailable;
    }
    void setScanlabScanner(bool value);

    Q_INVOKABLE void selectAll();

    Q_INVOKABLE void selectNone();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

Q_SIGNALS:
    void laserControlChanged();
    void scanlabScannerChanged();

private:
    bool m_laserControlAvailable = false;
    bool m_scanlabScannerAvailable = false;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::SeamPropertyFilterModel*)
