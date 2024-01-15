#pragma once
#include "errorSettingModel.h"
#include <QObject>
#include <QSortFilterProxyModel>

class QFileSystemWatcher;

namespace precitec
{
namespace storage
{

class ErrorSettingModel;

}

namespace gui {

class ErrorTemplateFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ErrorTemplateFilterModel(QObject *parent = nullptr);
    ~ErrorTemplateFilterModel() override;

    Q_PROPERTY(precitec::storage::ErrorSettingModel *errorSettingModel READ errorSettingModel WRITE setErrorSettingModel NOTIFY errorSettingModelChanged)

    Q_PROPERTY(bool qualityFaultCategory2 READ qualityFaultCategory2 WRITE setQualityFaultCategory2 NOTIFY qualityFaultCategory2Changed)

    storage::ErrorSettingModel *errorSettingModel() const
    {
        return m_errorSettingModel;
    }
    void setErrorSettingModel(storage::ErrorSettingModel *model);

    bool qualityFaultCategory2() const
    {
        return m_qualityFaultCategory2;
    }
    void setQualityFaultCategory2(bool value);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

Q_SIGNALS:
    void errorSettingModelChanged();
    void qualityFaultCategory2Changed();

private:
    storage::ErrorSettingModel *m_errorSettingModel = nullptr;
    bool m_qualityFaultCategory2 = false;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ErrorTemplateFilterModel*)

