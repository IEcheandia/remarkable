#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

class ScanfieldSeamModel;

/**
 * @brief Filter proxy model to filter out seams with no valid camera center
 **/
class ScanfieldSeamFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::gui::ScanfieldSeamModel* scanfieldSeamModel READ scanfieldSeamModel WRITE setScanfieldSeamModel NOTIFY scanfieldSeamModelChanged)

public:
    ScanfieldSeamFilterModel(QObject* parent = nullptr);
    ~ScanfieldSeamFilterModel() override;

    ScanfieldSeamModel* scanfieldSeamModel() const
    {
        return m_scanfieldSeamModel;
    }
    void setScanfieldSeamModel(ScanfieldSeamModel* scanfieldSeamModel);

Q_SIGNALS:
    void scanfieldSeamModelChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    ScanfieldSeamModel* m_scanfieldSeamModel = nullptr;
    QMetaObject::Connection m_scanfieldSeamModelDestroyed;
    QMetaObject::Connection m_scanfieldSeamModelChanged;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ScanfieldSeamFilterModel*)

