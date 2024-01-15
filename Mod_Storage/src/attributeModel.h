#pragma once

#include <QAbstractItemModel>

#include <Poco/SharedPtr.h>

#include <vector>

#include <QIODevice>


namespace precitec
{

namespace interface
{
class KeyValue;
}

namespace storage
{
class Attribute;


/**
 * This model provides all Attributes.
 * By default it has an empty model and one needs to invoke load or loadDefault.
 */
class AttributeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AttributeModel(QObject *parent = nullptr);
    ~AttributeModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Loads the Attributes from @p path.
     * Loading is performed in an async way.
     **/
    Q_INVOKABLE void load(const QString &path);

    /**
     * Loads the Attributes from a default location.
     * Loading is performed in an async way.
     **/
    Q_INVOKABLE void loadDefault();

    /**
     * Loads the KeyValue Attributes from a default location.
     * Loading is performed in an async way.
     **/
    Q_INVOKABLE void loadDefaultKeyValue();

    /**
     * Finds the Attribute with the given @p id
     **/
    Q_INVOKABLE precitec::storage::Attribute *findAttribute(const QUuid &id) const;

    /**
     * Finds all Attributes by the given @p variantId
     **/
    Q_INVOKABLE std::vector<Attribute*> findAttributesByVariantId(const QUuid &variantId) const;

    precitec::storage::Attribute *findAttributeByName(const QString &name) const;

    /**
     * Creates an attribute from the given @p keyValue and adds it to the model.
     **/
    void createAttribute(const Poco::SharedPtr<interface::KeyValue>& keyValue, const QUuid &hardwareDevice);
    
    QJsonObject toJson() const;
    void toJsonFile(QIODevice *device) const;
    
    /**
     * @returns whether the AttributeModel is currently loading the key values.
     **/
    bool isLoading() const
    {
        return m_loading;
    }

private:
    void clear();
    std::vector<Attribute*> m_attributes;
    bool m_loading = false;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::AttributeModel*)
