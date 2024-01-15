#pragma once

#include <QAbstractListModel>
#include <QIODevice>
#include <QUuid>

#include <vector>

#include "resultSetting.h"

class TestErrorSettingModel;
class LatestProductErrorsModelTest;

namespace precitec
{
namespace storage
{

class ProductModel;

/**
 * This model provides all Errors defined by the user
 * Errors are used for adding errors to the product
 */
class ErrorSettingModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The ProductModel from which the Products are received
     **/
    Q_PROPERTY(precitec::storage::ProductModel *productModel READ productModel WRITE setProductModel NOTIFY productModelChanged)

public:
    explicit ErrorSettingModel(QObject *parent = nullptr);
    ~ErrorSettingModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void updateValue(const QModelIndex& modelIndex, const QVariant &data, precitec::storage::ResultSetting::Type target);
    Q_INVOKABLE void deleteError(const QModelIndex& modelIndex, const QVariant &data);
    Q_INVOKABLE void createNewError(const QString &name, const QVariant &enumType);
    Q_INVOKABLE int highestEnumValue();
    /**
     * Ensures that a ErrorSettings for @p enumType exists.
     * This is the same as calling @link checkAndAddItem with the name parameter derived from @p enumType and color black.
     **/
    Q_INVOKABLE void ensureItemExists(int enumType);

    /**
     * Finds the index for the result configuration of the @p enumType.
     **/
    Q_INVOKABLE QModelIndex indexForResultType(int enumType) const;

    ProductModel *productModel() const
    {
        return m_productModel;
    }
    void setProductModel(ProductModel *productModel);

    const std::vector<ResultSetting*> &errorItems() const
    {
        return m_errorItems;
    }

    ResultSetting *getItem(int enumType);

    /**
     * @returns A QJsonObject representation for this object.
     **/
    QJsonObject toJson() const;

    /**
     * Writes this object as json to the @p device (the file).
     **/
    void toJson(QIODevice *device);

Q_SIGNALS:
    void errorSettingModelChanged();
    void productModelChanged();
    void updateUsedFlagChanged();

private:
    ProductModel *m_productModel = nullptr;
    std::vector<ResultSetting*> m_errorItems;
    QString m_errorStorageFile;

    /**
     * Read the jsonfile and fill up the errorsList
     **/
    void loadErrors();

    /**
     * check the products and annotate the error whether it is used as error in a product
     * or is defined in a graph
     **/
    void updateUsedFlag();

    /**
     * check if the error from the graph is already in the list; create and add if not
     **/
    void checkAndAddNewError(const QUuid &uuid, int enumType);

    /**
     * Write the errorsList back to the jsonfile
     **/
    void writeUpdatedList();

    /**
     * Json file does not exist, create the default errors. Write the errorsList back into the file
     **/
    void createFileWithDefaultErrors();

    /**
     * Create a single default error
     **/
    void createAndAddItem(const QUuid &uuid, int enumType);

    friend TestErrorSettingModel;
    friend LatestProductErrorsModelTest;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::ErrorSettingModel*)

