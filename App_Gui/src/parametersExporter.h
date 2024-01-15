#pragma once

#include <QObject>

namespace precitec
{

namespace storage
{
class Product;
class GraphModel;
class SubGraphModel;
class AttributeModel;
}

namespace gui
{

class Workbook;

/**
 * ParametersExporter exports hardware and graph parameters to an xlsx file.
 **/
class ParametersExporter : public QObject
{
    Q_OBJECT
    /**
     * The product is used as an information source for hardwareParametersWorksheet and productSeamGraphWorksheets
     **/
    Q_PROPERTY(precitec::storage::Product *product READ product WRITE setProduct NOTIFY productChanged)
    /**
     * The following graphModel, subGraphModel and attributeModel are used as an information source for productseamgraphworksheets
     **/
    Q_PROPERTY(precitec::storage::GraphModel *graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)
    Q_PROPERTY(precitec::storage::SubGraphModel *subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)
    /**
     * Whether the ParametersExporter is currently exporting parameters
     **/
    Q_PROPERTY(bool exporting READ isExporting WRITE setExporting NOTIFY exportingChanged);

public:
    ParametersExporter(QObject *parent = nullptr);
    ~ParametersExporter() = default;

    Q_INVOKABLE void performExport(const QString &dir);

    storage::Product *product() const;
    storage::GraphModel *graphModel() const;
    storage::SubGraphModel *subGraphModel() const;
    storage::AttributeModel *attributeModel() const;
    bool isExporting() const;

    void setProduct(storage::Product *product);
    void setGraphModel(storage::GraphModel *graphModel);
    void setSubGraphModel(storage::SubGraphModel *subGraphModel);
    void setExporting(bool isExporting);
    void setAttributeModel(storage::AttributeModel *attributeModel);

Q_SIGNALS:
    void productChanged();
    void graphModelChanged();
    void subGraphModelChanged();
    void attributeModelChanged();
    void exportingChanged();

private:
    void prepareWorkbook(const QString &directory, const QDateTime &date);

    storage::Product *m_product = nullptr;
    storage::GraphModel *m_graphModel = nullptr;
    storage::SubGraphModel *m_subGraphModel = nullptr;
    storage::AttributeModel *m_attributeModel = nullptr;
    bool m_isExporting = false;

    QMetaObject::Connection m_productDestroyed;
    QMetaObject::Connection m_graphModelDestroyed;
    QMetaObject::Connection m_subGraphModelDestroyed;
    QMetaObject::Connection m_attributeModelDestroyed;

    Workbook *m_workbook = nullptr;
};
} // namespace gui
} // namespace precitec