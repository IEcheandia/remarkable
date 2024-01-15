#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace storage
{

class ErrorTemplate;

}
namespace gui
{

/**
 * This model provides a template for Errors
 */
class ErrorTemplateModel : public QAbstractListModel {

    Q_OBJECT

public:
    explicit ErrorTemplateModel(QObject *parent = nullptr);
    ~ErrorTemplateModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void createErrorEntries();
    std::vector<precitec::storage::ErrorTemplate*> m_errorItems;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ErrorTemplateModel*)


