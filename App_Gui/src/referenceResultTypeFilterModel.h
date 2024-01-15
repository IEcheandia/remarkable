#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

class ReferenceResultTypeFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int resultType READ resultType WRITE setResultType NOTIFY resultTypeChanged)
public:
    explicit ReferenceResultTypeFilterModel(QObject *parent = nullptr);
    ~ReferenceResultTypeFilterModel() override;

    int resultType() const
    {
        return m_resultType;
    }
    void setResultType(int type);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void resultTypeChanged();

private:
    int m_resultType = -1;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ReferenceResultTypeFilterModel*)
