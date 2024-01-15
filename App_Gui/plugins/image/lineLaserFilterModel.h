#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

class LineLaserFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool lineLaser1Available READ lineLaser1Available WRITE setLineLaser1Available NOTIFY lineLaser1AvailableChanged)

    Q_PROPERTY(bool lineLaser2Available READ lineLaser2Available WRITE setLineLaser2Available NOTIFY lineLaser2AvailableChanged)

    Q_PROPERTY(bool lineLaser3Available READ lineLaser3Available WRITE setLineLaser3Available NOTIFY lineLaser3AvailableChanged)
public:
    LineLaserFilterModel(QObject *parent = nullptr);
    ~LineLaserFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool lineLaser1Available() const
    {
        return m_lineLaser1Available;
    }
    void setLineLaser1Available(bool set);

    bool lineLaser2Available() const
    {
        return m_lineLaser2Available;
    }
    void setLineLaser2Available(bool set);

    bool lineLaser3Available() const
    {
        return m_lineLaser3Available;
    }
    void setLineLaser3Available(bool set);

Q_SIGNALS:
    void lineLaser1AvailableChanged();
    void lineLaser2AvailableChanged();
    void lineLaser3AvailableChanged();

private:
    bool m_lineLaser1Available = false;
    bool m_lineLaser2Available = false;
    bool m_lineLaser3Available = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LineLaserFilterModel*)

