#include "xlsxHardwareParametersWorksheet.h"
#include "product.h"

#include <xlsxwriter/workbook.h>

#include "seamSeries.h"
#include "seam.h"
#include "linkedSeam.h"

#include "parameterSet.h"
#include "parameter.h"

using precitec::storage::LinkedSeam;
using precitec::storage::ParameterSet;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

namespace precitec::gui
{
HardwareParametersWorksheet::HardwareParametersWorksheet(QObject *parent)
    : AbstractWorksheet(parent){};

void HardwareParametersWorksheet::formWorksheet()
{
    if (m_product == nullptr)
    {
        return;
    }
    if (m_product->hardwareParameters() != nullptr)
    {
        initParameterSet(m_product->hardwareParameters());
    }

    for (auto series : m_product->seamSeries())
    {
        initSeamSeries(series);
    }

    writeToTable(1, 0, QStringLiteral("Seam_Series_(number)"));
    writeToTable(2, 0, QStringLiteral("Seam_(number)"));
    writeToTable(1, 1, QLatin1String(""));
    writeToTable(2, 1, QLatin1String(""));
    writeToTable(0, 1, QStringLiteral("Hardware_parameters"));

    writeToTable(0, 2, QStringLiteral("Product"));
    writeToTable(1, 2, QLatin1String(""));
    writeToTable(2, 2, QLatin1String(""));
    const size_t rowHeadSize = 3;
    const size_t columnHeadSize = 3;
    // row names (parameters)
    std::size_t hardwareParameterKeyIndex = rowHeadSize;
    for (const auto &hardwareParameterKey : m_hardwareParameterKeys)
    {
        writeToTable(hardwareParameterKeyIndex, 1, hardwareParameterKey);
        hardwareParameterKeyIndex++;
    }

    // column names (seams)
    std::size_t taskKey = columnHeadSize;
    for (auto task : m_measureTasks)
    {
        if (auto seam = qobject_cast<Seam *>(task))
        {
            writeToTable(1, taskKey,
                         QString("%1_(%2)").arg(seam->seamSeries()->name(),
                                                QString::number(seam->seamSeries()->visualNumber())));
            writeToTable(2, taskKey, QString("%1_(%2)").arg(task->name(), QString::number(task->visualNumber())));
        }
        else if (auto seamSeries = qobject_cast<SeamSeries *>(task))
        {
            writeToTable(1, taskKey,
                         QString("%1_(%2)").arg(seamSeries->name(), QString::number(seamSeries->visualNumber())));
            writeToTable(2, taskKey, QLatin1String(""));
            colorAllCellInBox(2, taskKey, 2, taskKey);
        }
        taskKey++;
    }

    // product column
    std::size_t productRowIndex = rowHeadSize;
    for (const auto &hardwareParameterKey : m_hardwareParameterKeys)
    {
        if (m_product->hardwareParameters() != nullptr)
        {
            const auto &parameters = m_product->hardwareParameters()->parameters();
            auto parameterIterator =
                find_if(parameters.begin(), parameters.end(), [&hardwareParameterKey](const auto parameter) {
                    return parameter->name() == hardwareParameterKey;
                });
            if (parameterIterator != parameters.end())
            {
                const auto &parameter = (*parameterIterator)->value();
                writeToTable(productRowIndex, 2, parameter);
            }
            else
            {
                writeToTable(productRowIndex, 2, QLatin1String(""));
            }
        }
        productRowIndex++;
    }

    // table (seam and seam series, product parameters)
    std::size_t rowIndex = rowHeadSize;
    for (const auto &hardwareParameterKey : m_hardwareParameterKeys)
    {
        std::size_t columnIndex = columnHeadSize;
        for (auto task : m_measureTasks)
        {
            if (task->hardwareParameters() != nullptr)
            {
                const auto &parameters = task->hardwareParameters()->parameters();
                auto parameterIterator =
                    find_if(parameters.begin(), parameters.end(), [&hardwareParameterKey](const auto parameter) {
                        return parameter->name() == hardwareParameterKey;
                    });
                if (parameterIterator != parameters.end())
                {
                    const auto &parameter = (*parameterIterator)->value();
                    writeToTable(rowIndex, columnIndex, parameter);
                }
                else
                {
                    writeToTable(rowIndex, columnIndex, QLatin1String(""));
                }
            }
            else
            {
                writeToTable(rowIndex, columnIndex, QStringLiteral("Not_specified"));
            }
            columnIndex++;
        }
        rowIndex++;
    }

    addBoxBorders(1, 0, 2, 0); // box for seam series and seams
    addBoxBorders(0, 1, 0, 1); // box for "Hardware parameters"
    addBoxBorders(0, 2, 0, 2); // box for "Product"
    addBoxBorders(1, 2, 2, 2); // box under "Product"

    addBoxBorders(rowHeadSize, 1, rowHeadSize + m_hardwareParameterKeys.size() - 1,
                  1); // box for row heads (hardware parameters)
    addBoxBorders(1, 0, rowHeadSize - 1, columnHeadSize + m_measureTasks.size() - 1); // box for column headers
    // box for the seam/hardware parameters table
    addBoxBorders(rowHeadSize, columnHeadSize - 1, rowHeadSize + m_hardwareParameterKeys.size() - 1,
                  columnHeadSize + m_measureTasks.size() - 1);
    centerAllCellsInBox(0, columnHeadSize - 1, rowHeadSize + m_hardwareParameterKeys.size() - 1,
                        columnHeadSize + m_measureTasks.size() - 1);
    colorAllCellInBox(1, 1, 2, 2);

    fromTableToXlsx();
    fitColumnsWidth();
}

void HardwareParametersWorksheet::initSeamSeries(SeamSeries *seamSeries)
{
    m_measureTasks.push_back(seamSeries);

    if (seamSeries->hardwareParameters() != nullptr)
    {
        initParameterSet(seamSeries->hardwareParameters());
    }

    for (auto seam : seamSeries->seams())
    {
        initSeam(seam);
    }
}

void HardwareParametersWorksheet::initParameterSet(ParameterSet *set)
{
    if (set == nullptr)
    {
        return;
    }
    for (auto parameter : set->parameters())
    {
        m_hardwareParameterKeys.insert(parameter->name());
    }
}

void HardwareParametersWorksheet::initSeam(Seam *seam)
{
    if (seam->metaObject()->inherits(&precitec::storage::LinkedSeam::staticMetaObject))
    {
        // ignore linked seams
        return;
    }
    m_measureTasks.push_back(seam);
    initParameterSet(seam->hardwareParameters());
}

void HardwareParametersWorksheet::setProduct(storage::Product *product)
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
                                     std::bind(&HardwareParametersWorksheet::setProduct, this, nullptr));
    }
    else
    {
        m_productDestroyed = QMetaObject::Connection{};
    }
}

} // namespace precitec::gui
