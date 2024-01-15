#include "parametersExporter.h"

#include <QDir>
#include <QDateTime>
#include <QtConcurrent>

#include "product.h"
#include "graphModel.h"
#include "subGraphModel.h"
#include "attributeModel.h"

#include "xlsxWorkbook.h"
#include "xlsxHardwareParametersWorksheet.h"
#include "xlsxProductSeamGraphWorksheets.h"

#include <precitec/notificationSystem.h>

using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::Parameter;
using precitec::storage::Product;
using precitec::storage::SubGraphModel;

namespace precitec::gui
{

ParametersExporter::ParametersExporter(QObject *parent)
    : QObject(parent)
{
}

Product *ParametersExporter::product() const
{
    return m_product;
}

GraphModel *ParametersExporter::graphModel() const
{
    return m_graphModel;
}

SubGraphModel *ParametersExporter::subGraphModel() const
{
    return m_subGraphModel;
}

bool ParametersExporter::isExporting() const
{
    return m_isExporting;
}

void ParametersExporter::setProduct(Product *product)
{
    if (product == m_product)
    {
        return;
    }

    m_product = product;

    disconnect(m_productDestroyed);
    if (m_product)
    {
        m_productDestroyed = connect(m_product, &storage::Product::destroyed, this,
                                     std::bind(&ParametersExporter::setProduct, this, nullptr));
    }
    else
    {
        m_productDestroyed = QMetaObject::Connection{};
    }

    emit productChanged();
}

void ParametersExporter::setGraphModel(GraphModel *graphModel)
{
    if (m_graphModel == graphModel)
    {
        return;
    }
    m_graphModel = graphModel;

    disconnect(m_graphModelDestroyed);
    if (m_graphModel)
    {
        m_graphModelDestroyed = connect(m_product, &storage::GraphModel::destroyed, this,
                                        std::bind(&ParametersExporter::setGraphModel, this, nullptr));
    }
    else
    {
        m_graphModelDestroyed = QMetaObject::Connection{};
    }

    emit graphModelChanged();
}

void ParametersExporter::setSubGraphModel(SubGraphModel *subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }

    m_subGraphModel = subGraphModel;
    disconnect(m_subGraphModelDestroyed);
    if (m_graphModel)
    {
        m_subGraphModelDestroyed = connect(m_product, &storage::SubGraphModel::destroyed, this,
                                           std::bind(&ParametersExporter::setSubGraphModel, this, nullptr));
    }
    else
    {
        m_subGraphModelDestroyed = QMetaObject::Connection{};
    }

    emit subGraphModelChanged();
}

void ParametersExporter::setAttributeModel(storage::AttributeModel *attributeModel)
{
    if (m_attributeModel == attributeModel)
    {
        return;
    }
    m_attributeModel = attributeModel;

    disconnect(m_attributeModelDestroyed);
    if (m_attributeModel)
    {
        m_attributeModelDestroyed = connect(m_product, &storage::AttributeModel::destroyed, this,
                                            std::bind(&ParametersExporter::setAttributeModel, this, nullptr));
    }
    else
    {
        m_attributeModelDestroyed = QMetaObject::Connection{};
    }

    emit attributeModelChanged();
}

void ParametersExporter::setExporting(bool isExporting)
{
    if (isExporting == m_isExporting)
    {
        return;
    }
    m_isExporting = isExporting;

    emit exportingChanged();
}

void ParametersExporter::performExport(const QString &dir)
{
    // check setup
    if (m_isExporting)
    {
        gui::components::notifications::NotificationSystem::instance()->warning(
            tr("Parameters are exporting. Please wait!"));
        return;
    }

    prepareWorkbook(dir, QDateTime::currentDateTime());

    if (m_product == nullptr || m_graphModel == nullptr || m_subGraphModel == nullptr || m_workbook == nullptr || m_workbook == nullptr)
    {
        gui::components::notifications::NotificationSystem::instance()->error(
            tr("Internal error during Excel export."));
        return;
    }

    setExporting(true);
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, [this]() { this->setExporting(false); });
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    Product *productCopy = storage::Product::fromJson(m_product->filePath(), watcher);

    watcher->setFuture(QtConcurrent::run([this, productCopy] {
        // initialisation of hardware parameters worksheet
        auto hardwareParametersWorksheet = std::make_unique<HardwareParametersWorksheet>();
        hardwareParametersWorksheet->setProduct(productCopy);
        m_workbook->addWorksheet(std::move(hardwareParametersWorksheet), QString("Hardware parameters overview"));

        // initialisation of graph parameters worksheet
        auto productSeamGraphWorksheets = std::make_unique<ProductSeamGraphWorksheets>();
        productSeamGraphWorksheets->setProduct(productCopy);
        productSeamGraphWorksheets->setGraphModel(m_graphModel);
        productSeamGraphWorksheets->setSubGraphModel(m_subGraphModel);
        productSeamGraphWorksheets->setAttributeModel(m_attributeModel);
        productSeamGraphWorksheets->setTargetWorkbook(m_workbook);
        m_workbook->addWorksheet(std::move(productSeamGraphWorksheets), QString("Graph parameters overview"));

        // save all worksheets
        m_workbook->save(); // this line cleanup the workbook and all related worksheets

        gui::components::notifications::NotificationSystem::instance()->information(
            tr("Product exported successfully."));
    }));
}

void ParametersExporter::prepareWorkbook(const QString &directory, const QDateTime &date)
{
    QDir{}.mkpath(directory);
    if (m_product != nullptr && QDir(directory).exists())
    {
        m_workbook = new Workbook(this);
        const auto fileName = QStringLiteral("product_%1_parameters_%2.xlsx")
                                  .arg(m_product->name(), date.toString(QStringLiteral("yyyyMMdd-HHmmss")));
        auto title = QStringLiteral("Parameters of product \"%1\" ").arg(m_product->name());
        m_workbook->prepareWorkbook(directory, fileName, title);
    }
}

storage::AttributeModel *ParametersExporter::attributeModel() const
{
    return m_attributeModel;
}

} // namespace precitec::gui