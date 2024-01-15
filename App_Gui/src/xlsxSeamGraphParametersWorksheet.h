#pragma once

#include "xlsxAbstractWorksheet.h"

#include <QUuid>

#include <memory>

namespace precitec
{

namespace storage
{
class Seam;
class Product;
class GraphModel;
class SubGraphModel;
class AttributeModel;
}

namespace gui
{

class SeamGraphParametersWorksheet : public AbstractWorksheet
{
    Q_OBJECT
public:
    SeamGraphParametersWorksheet(QObject *parent = nullptr);
    ~SeamGraphParametersWorksheet() = default;

    void setSeam(storage::Seam *seam);
    void setProduct(storage::Product *product);
    void setGraphModel(storage::GraphModel *graphModel);
    void setSubGraphModel(storage::SubGraphModel *subGraphModel);
    void setAttributeModel(storage::AttributeModel *attributeModel);

private:
    void formWorksheet() override;

    struct InstanceInfo
    {
        QString filterName;
        QString filterGroup;
        QString filterType;
        QString parameterName;
        QUuid filterId;
        int userLevel;
    };

    SeamGraphParametersWorksheet::InstanceInfo getInstanceInfo(const QUuid &filterId, const QUuid &parameterId) const;
    bool checkUserLevel(int userLevel);
    static std::string lastNameFromNestedNamespace(const std::string &nestedNamespace);

    storage::Seam *m_seam = nullptr;
    storage::Product *m_product = nullptr;
    storage::GraphModel *m_graphModel = nullptr;
    storage::SubGraphModel *m_subGraphModel = nullptr;
    storage::AttributeModel *m_attributeModel = nullptr;

    QMetaObject::Connection m_seamDestroyed;
    QMetaObject::Connection m_productDestroyed;
    QMetaObject::Connection m_graphModelDestroyed;
    QMetaObject::Connection m_subGraphModelDestroyed;
    QMetaObject::Connection m_attributeModelDestroyed;
};

} // namespace gui
} // namespace precitec
