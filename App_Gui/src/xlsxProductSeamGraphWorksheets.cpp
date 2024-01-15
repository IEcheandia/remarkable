#include "xlsxProductSeamGraphWorksheets.h"
#include <QUuid>

#include "attributeModel.h"
#include "graphFunctions.h"
#include "product.h"
#include "graphModel.h"
#include "subGraphModel.h"
#include "seam.h"
#include "seamSeries.h"
#include "parameterSet.h"

#include "xlsxWorkbook.h"

#include "xlsxwriter/workbook.h"
#include "xlsxwriter/worksheet.h"

#include "xlsxSeamGraphParametersWorksheet.h"

using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SubGraphModel;
using precitec::storage::graphFunctions::getGraphFromModel;

using precitec::gui::SeamGraphParametersWorksheet;

namespace precitec::gui
{

void ProductSeamGraphWorksheets::formWorksheet()
{
    if (m_product == nullptr)
    {
        return;
    }

    std::size_t currentColumnIndex = 0;

    const auto &seamSeries = m_product->seamSeries();
    writeToTable(0, currentColumnIndex, QStringLiteral("Seam_name"));
    writeToTable(1, currentColumnIndex, QStringLiteral("Seam_number"));
    writeToTable(2, currentColumnIndex, QStringLiteral("Seam_Series"));
    writeToTable(3, currentColumnIndex, QStringLiteral("Graph_name"));
    writeToTable(4, currentColumnIndex, QStringLiteral("Link"));
    currentColumnIndex++;
    auto filterParameterSets = m_product->filterParameterSets();

    for (auto series : seamSeries)
    {
        const auto &seams = series->seams();
        const auto seriesName = QString("%1_(%2)").arg(series->name(), QString::number(series->visualNumber()));
        for (auto seam : seams)
        {
            writeToTable(0, currentColumnIndex, seam->name());
            writeToTable(1, currentColumnIndex, seam->visualNumber());
            writeToTable(2, currentColumnIndex, seriesName);
            writeToTable(3, currentColumnIndex, getGraphName(seam));
            const auto name = QString("seam_%1_of_seam_series_%2")
                                  .arg(QString::number(seam->visualNumber()), QString::number(series->visualNumber()));
            if (m_targetWorkbook != nullptr)
            {
                // initialization
                auto productSeamGraphWorksheets = std::make_unique<SeamGraphParametersWorksheet>();
                productSeamGraphWorksheets->setSeam(seam);
                productSeamGraphWorksheets->setProduct(m_product);
                productSeamGraphWorksheets->setGraphModel(m_graphModel);
                productSeamGraphWorksheets->setSubGraphModel(m_subGraphModel);
                productSeamGraphWorksheets->setAttributeModel(m_attributeModel);

                m_targetWorkbook->addWorksheet(std::move(productSeamGraphWorksheets), name);

                worksheet_write_url(worksheet(), 4, currentColumnIndex, QString("internal:%1").arg(name).toUtf8(),
                                    nullptr);
                writeToTable(4, currentColumnIndex, QStringLiteral("Graph_parameters"));
            }
            else
            {
                writeToTable(4, currentColumnIndex, QStringLiteral("Error!_Target_worksheet_is_not_given!"));
            }
            currentColumnIndex++;
        }
    }

    const std::size_t rowLastIndex = 4;
    --currentColumnIndex;

    addBoxBorders(0, 0, 0, 0);
    addBoxBorders(1, 0, rowLastIndex, 0);
    addBoxBorders(1, 0, rowLastIndex, currentColumnIndex);

    for (std::size_t i = 0; i < rowLastIndex; ++i)
    {
        addBoxBorders(i, 0, i, currentColumnIndex);
    }

    centerAllCellsInBox(0, 1, rowLastIndex, currentColumnIndex);

    fromTableToXlsx();
    fitColumnsWidth();
}

ProductSeamGraphWorksheets::ProductSeamGraphWorksheets(QObject *parent)
    : AbstractWorksheet(parent)
{
}

QString ProductSeamGraphWorksheets::getGraphName(Seam *seam) const
{
    fliplib::GraphContainer graph = getGraphFromModel(seam, m_graphModel, m_subGraphModel);
    return QString::fromStdString(graph.name);
}

void ProductSeamGraphWorksheets::setProduct(Product *product)
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
                                     std::bind(&ProductSeamGraphWorksheets::setProduct, this, nullptr));
    }
    else
    {
        m_productDestroyed = QMetaObject::Connection{};
    }
}

void ProductSeamGraphWorksheets::setGraphModel(GraphModel *graphModel)
{
    if (m_graphModel == graphModel)
    {
        return;
    }
    m_graphModel = graphModel;

    disconnect(m_graphModelDestroyed);
    if (m_graphModel)
    {
        m_graphModelDestroyed = connect(m_graphModel, &storage::GraphModel::destroyed, this,
                                        std::bind(&ProductSeamGraphWorksheets::setGraphModel, this, nullptr));
    }
    else
    {
        m_graphModelDestroyed = QMetaObject::Connection{};
    }
}

void ProductSeamGraphWorksheets::setSubGraphModel(SubGraphModel *subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }

    m_subGraphModel = subGraphModel;
    disconnect(m_subGraphModelDestroyed);
    if (m_graphModel)
    {
        m_subGraphModelDestroyed = connect(m_subGraphModel, &storage::SubGraphModel::destroyed, this,
                                           std::bind(&ProductSeamGraphWorksheets::setSubGraphModel, this, nullptr));
    }
    else
    {
        m_subGraphModelDestroyed = QMetaObject::Connection{};
    }
}

void ProductSeamGraphWorksheets::setAttributeModel(AttributeModel *attributeModel)
{
    if (m_attributeModel == attributeModel)
    {
        return;
    }
    m_attributeModel = attributeModel;

    disconnect(m_attributeModelDestroyed);
    if (m_attributeModel)
    {
        m_attributeModelDestroyed = connect(m_attributeModel, &storage::AttributeModel::destroyed, this,
                                            std::bind(&ProductSeamGraphWorksheets::setAttributeModel, this, nullptr));
    }
    else
    {
        m_attributeModelDestroyed = QMetaObject::Connection{};
    }
}

void ProductSeamGraphWorksheets::setTargetWorkbook(Workbook *workbook)
{
    if (m_targetWorkbook == workbook)
    {
        return;
    }

    m_targetWorkbook = workbook;

    disconnect(m_targetWorkbookDestroyed);
    if (m_targetWorkbook)
    {
        m_targetWorkbookDestroyed = connect(m_targetWorkbook, &gui::Workbook::destroyed, this,
                                            std::bind(&ProductSeamGraphWorksheets::setTargetWorkbook, this, nullptr));
    }
    else
    {
        m_targetWorkbookDestroyed = QMetaObject::Connection{};
    }
}

} // namespace precitec::gui