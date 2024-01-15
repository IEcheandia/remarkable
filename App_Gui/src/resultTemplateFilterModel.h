#pragma once
#include "resultSettingModel.h"
#include <QObject>
#include <QSortFilterProxyModel>

class QFileSystemWatcher;

namespace precitec
{
    namespace storage
    {

        class ResultSettingModel;

    }

    namespace gui {

        class ResultTemplateFilterModel : public QSortFilterProxyModel
        {
            Q_OBJECT

        public:
            ResultTemplateFilterModel(QObject *parent = nullptr);
            ~ResultTemplateFilterModel() override;

            Q_PROPERTY(precitec::storage::ResultSettingModel *resultSettingModel READ resultSettingModel WRITE setResultSettingModel NOTIFY resultSettingModelChanged)

            Q_PROPERTY(bool qualityFaultCategory2 READ qualityFaultCategory2 WRITE setQualityFaultCategory2 NOTIFY qualityFaultCategory2Changed)

            storage::ResultSettingModel *resultSettingModel() const
            {
                return m_resultSettingModel;
            }
            void setResultSettingModel(storage::ResultSettingModel *model);

            bool qualityFaultCategory2() const
            {
                return m_qualityFaultCategory2;
            }
            void setQualityFaultCategory2(bool value);

        protected:
            virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        Q_SIGNALS:
            void resultSettingModelChanged();
            void qualityFaultCategory2Changed();

        private:
            storage::ResultSettingModel *m_resultSettingModel = nullptr;
            bool m_qualityFaultCategory2 = false;
        };

    }
}
Q_DECLARE_METATYPE(precitec::gui::ResultTemplateFilterModel*)

