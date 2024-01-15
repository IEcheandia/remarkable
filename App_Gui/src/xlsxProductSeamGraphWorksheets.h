#pragma once

#include "xlsxAbstractWorksheet.h"

namespace precitec
{

namespace storage
{
class Product;
class GraphModel;
class SubGraphModel;
class AttributeModel;
class Seam;
}

namespace gui
{
class Workbook;

class ProductSeamGraphWorksheets : public AbstractWorksheet
{
    Q_OBJECT
public:
    ProductSeamGraphWorksheets(QObject *parent = nullptr);
    void setProduct(storage::Product *product);
    void setGraphModel(storage::GraphModel *mGraphModel);
    void setSubGraphModel(storage::SubGraphModel *subGraphModel);
    void setAttributeModel(storage::AttributeModel *mAttributeModel);
    void setTargetWorkbook(Workbook *mWorkbook);

private:
    void formWorksheet() override;
    QString getGraphName(storage::Seam *seam) const;

    storage::Product *m_product = nullptr;
    storage::GraphModel *m_graphModel = nullptr;
    storage::SubGraphModel *m_subGraphModel = nullptr;
    storage::AttributeModel *m_attributeModel = nullptr;
    Workbook *m_targetWorkbook = nullptr;

    QMetaObject::Connection m_productDestroyed;
    QMetaObject::Connection m_graphModelDestroyed;
    QMetaObject::Connection m_subGraphModelDestroyed;
    QMetaObject::Connection m_attributeModelDestroyed;
    QMetaObject::Connection m_targetWorkbookDestroyed;
};

} // namespace gui
} // namespace precitec
