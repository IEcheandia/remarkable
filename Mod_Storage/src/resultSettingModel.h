#pragma once

#include "resultSetting.h"

#include <QAbstractListModel>

class QIODevice;

namespace precitec
{
namespace storage
{

/**
 * This model provides all ResultSettings.
 * ResultSettings are used for configuration of the results, mainly the plotter values
 */
class ResultSettingModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ResultSettingModel(QObject *parent = nullptr);
    ~ResultSettingModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void updateValue(const QModelIndex& modelIndex, const QVariant &data, precitec::storage::ResultSetting::Type target);

    Q_INVOKABLE void deleteResult(const QModelIndex& modelIndex, const QVariant &data);
    Q_INVOKABLE void createNewResult(const QString &name, const QVariant &enumType);
    Q_INVOKABLE int highestEnumValue();

    /**
     * Finds the index for the result configuration of the @p enumType.
     **/
    Q_INVOKABLE QModelIndex indexForResultType(int enumType) const;

    Q_INVOKABLE QString nameForResultType(int enumType) const;

    /**
     * @returns A QJsonObject representation for this object.
     **/
    QJsonObject toJson() const;

    /**
     * Writes this object as json to the @p device (the file).
     **/
    void toJson(QIODevice *device);

    /**
     * Check if the Data for the given result exists, create if not..
     * @returns A ResultSettings for configuration of the result plotter .
     **/
    ResultSetting *checkAndAddItem(int enumType, QString name);
    ResultSetting *getItem(int enumType) const;

    /**
     * Ensures that a ResultSettings for @p enumType exists.
     * This is the same as calling @link checkAndAddItem with the name parameter derived vrom @p enumType.
     **/
    Q_INVOKABLE void ensureItemExists(int enumType);

    /**
     * Imports the result configuration from the json file at @p path.
     **/
    void import(const QString &path);

    /**
     * Evaluates if a result enum is a lwm result type
     * Lwm results defined as 737, 738, 739, 740
     **/
    Q_INVOKABLE bool isLwmType(int enumType);

    const std::vector<ResultSetting*> &resultItems() const
    {
        return m_resultItems;
    }

private:
    void loadResults();
    void writeUpdatedList();
    std::vector<ResultSetting*> m_resultItems;
    QString m_resultStorageFile;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::ResultSettingModel*)
