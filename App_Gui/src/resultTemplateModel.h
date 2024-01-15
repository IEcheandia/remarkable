#pragma once

#include <QAbstractListModel>

namespace precitec
{
    namespace storage
    {

        class ResultTemplate;
    }
    namespace gui
    {
        /**
         * This model provides a template for Results
         */
        class ResultTemplateModel : public QAbstractListModel {

            Q_OBJECT

        public:
            explicit ResultTemplateModel(QObject *parent = nullptr);
            ~ResultTemplateModel() override;

            int rowCount(const QModelIndex &parent = QModelIndex()) const override;
            virtual QVariant data(const QModelIndex &index, int role) const override;
            QHash<int, QByteArray> roleNames() const override;

        private:
            void createResultEntries();
            std::vector<precitec::storage::ResultTemplate*> m_resultItems;
        };

    }
}
Q_DECLARE_METATYPE(precitec::gui::ResultTemplateModel*)
