#pragma once

#include "simpleErrorModel.h"

namespace precitec
{
namespace storage
{

class SeamError;

}
namespace gui
{

class SeamErrorModel : public SimpleErrorModel
{
    Q_OBJECT

public:
    explicit SeamErrorModel(QObject *parent = nullptr);
    ~SeamErrorModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Creates a new SeamError
     * @returns new created SeamError
     **/
    Q_INVOKABLE precitec::storage::SeamError *createError(ErrorType errorType);

    /**
     * Delete selected SeamError
     **/
    Q_INVOKABLE void removeError(precitec::storage::SeamError *error);
};

}
}
Q_DECLARE_METATYPE(precitec::gui::SeamErrorModel*)
