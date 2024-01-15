#include "xlsxSeamGraphParametersWorksheet.h"

#include <QUuid>

#include "graphFunctions.h"
#include "product.h"
#include "graphModel.h"
#include "subGraphModel.h"
#include "attributeModel.h"

#include "permissions.h"
#include <precitec/userManagement.h>

#include "attribute.h"
#include "seam.h"
#include "seamSeries.h"
#include "parameterSet.h"
#include "parameter.h"

#include "xlsxWorkbook.h"

#include "xlsxwriter/workbook.h"
#include "xlsxwriter/worksheet.h"

#include "../App_Storage/src/compatibility.h"
#include "../plugins/general/languageSupport.h"

#include <algorithm>
#include <string>

using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::Parameter;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SubGraphModel;
using precitec::storage::graphFunctions::getGraphFromModel;
using precitec::storage::graphFunctions::hasMatchingModel;

using precitec::gui::SeamGraphParametersWorksheet;

using precitec::gui::LanguageSupport;

namespace precitec::gui
{

SeamGraphParametersWorksheet::SeamGraphParametersWorksheet(QObject *parent)
    : AbstractWorksheet(parent)
{
}

void SeamGraphParametersWorksheet::formWorksheet()
{
    if (m_product == nullptr || m_seam == nullptr || !hasMatchingModel(m_seam, m_graphModel, m_subGraphModel))
    {
        return;
    }

    const auto &filterParameterSets = m_product->filterParameterSets();
    auto filterParameterSetIterator =
        std::find_if(filterParameterSets.begin(), filterParameterSets.end(),
                     [this](auto item) { return item->uuid() == this->m_seam->graphParamSet(); });
    if (filterParameterSetIterator == filterParameterSets.end())
    {
        writeToTable(0, 0, QStringLiteral("Can't_find_filter_settings_for_the_seam"));
        return;
    }

    writeToTable(0, 0, QStringLiteral("Filter_instance_parameter_values"));

    writeToTable(1, 0, QStringLiteral("Filter_group"));
    writeToTable(1, 1, QStringLiteral("Filter_label"));
    writeToTable(1, 2, QStringLiteral("Filter_type"));
    writeToTable(1, 3, QStringLiteral("Filter_id"));
    writeToTable(1, 4, QStringLiteral("Parameter"));
    writeToTable(1, 5, QStringLiteral("Parameter_value"));
    writeToTable(1, 6, QStringLiteral("User_level"));

    const auto &parameters = (*filterParameterSetIterator)->parameters();
    std::vector<std::pair<InstanceInfo, Parameter *>> filterParameters;

    std::transform(parameters.begin(), parameters.end(), std::back_inserter(filterParameters),
                   [this](auto parameter) -> std::pair<InstanceInfo, Parameter *> {
                       auto instanceInfo = getInstanceInfo(parameter->filterId(), parameter->uuid());
                       if (instanceInfo.parameterName.isEmpty())
                       {
                           instanceInfo.parameterName = parameter->name();
                       }
                       return std::make_pair(instanceInfo, parameter);
                   });

    std::sort(filterParameters.begin(), filterParameters.end(), [](auto lParameter, auto rParameter) {
        const auto zeroCase = lParameter.first.filterGroup < rParameter.first.filterGroup;
        const auto equalGroups = (lParameter.first.filterGroup == rParameter.first.filterGroup);
        const auto firstCase = equalGroups && (lParameter.first.filterName < rParameter.first.filterName);
        const auto equalNames = (lParameter.first.filterName == rParameter.first.filterName);
        const auto equalIds = (lParameter.first.filterId == rParameter.first.filterId);
        const auto secondCase =
            (equalGroups && equalNames && equalIds && lParameter.first.parameterName < rParameter.first.parameterName);
        const auto thirdCase = (equalGroups && equalNames && lParameter.first.filterId < lParameter.first.filterId);
        return zeroCase || firstCase || secondCase || thirdCase;
    });

    std::size_t currentRowIndex = 2;
    for (const auto &parameter : filterParameters)
    {
        if (checkUserLevel(parameter.first.userLevel) && !parameter.first.filterId.isNull())
        {
            writeToTable(currentRowIndex, 0, parameter.first.filterGroup);
            writeToTable(currentRowIndex, 1, parameter.first.filterName);
            writeToTable(currentRowIndex, 2, parameter.first.filterType);
            writeToTable(currentRowIndex, 3, parameter.first.filterId.toString(QUuid::WithoutBraces));
            writeToTable(currentRowIndex, 4, parameter.first.parameterName);
            writeToTable(currentRowIndex, 5, parameter.second->value());
            writeToTable(currentRowIndex, 6, parameter.first.userLevel);
            currentRowIndex++;
        }
    }

    --currentRowIndex;
    std::size_t lastRowIndex = 6;
    addBoxBorders(0, 0, 0, 0); // Filter instance parameter values
    addBoxBorders(1, 0, 1, 3); // Filter header info borders
    addBoxBorders(1, 4, 1, lastRowIndex); // Parameter header borders

    addBoxBorders(1, 0, 1, 3); // Filter header info borders
    addBoxBorders(1, 4, 1, lastRowIndex); // Parameter borders

    addBoxBorders(1, 0, currentRowIndex, 3); // Filter info borders
    addBoxBorders(1, 4, currentRowIndex, lastRowIndex); // Parameter borders
    addBoxBorders(1, lastRowIndex, currentRowIndex, lastRowIndex); // User-level borders

    centerAllCellsInBox(1, 0, currentRowIndex, lastRowIndex);
    fromTableToXlsx();
    fitColumnsWidth();
}

SeamGraphParametersWorksheet::InstanceInfo SeamGraphParametersWorksheet::getInstanceInfo(const QUuid &filterId,
                                                                                         const QUuid &parameterId) const
{
    fliplib::GraphContainer graphContainer = getGraphFromModel(m_seam, m_graphModel, m_subGraphModel);
    const auto &filters = graphContainer.instanceFilters;
    const auto uuid = precitec::storage::compatibility::toPoco(filterId);
    auto filterIterator =
        find_if(filters.begin(), filters.end(), [&uuid](const auto &filter) { return uuid == filter.id; });

    if (filterIterator != filters.end())
    {
        const auto &filter = *filterIterator;
        // Group Information
        const auto &filterGroups = graphContainer.filterGroups;
        const auto groupIt = std::find_if(filterGroups.begin(), filterGroups.end(),
                                          [filter](const auto &group) { return group.number == filter.group; });
        const auto groupInfo =
            (groupIt != filterGroups.end()) ? QString::fromStdString(groupIt->name) : QStringLiteral("not_grouped");
        // Type information
        const auto &filterDescriptions = graphContainer.filterDescriptions;

        auto filterTypeIt =
            std::find_if(filterDescriptions.begin(), filterDescriptions.end(),
                         [filter](const auto &filterDescription) { return filter.filterId == filterDescription.id; });
        const auto filterType = (filterTypeIt != filterDescriptions.end())
                                    ? QString::fromStdString(lastNameFromNestedNamespace((*filterTypeIt).name))
                                    : QString();

        const auto instanceAttributeId{precitec::storage::compatibility::toPoco(parameterId)};
        const auto attributeIt = std::find_if(filter.attributes.begin(), filter.attributes.end(),
                                              [&instanceAttributeId](const auto &attribute) {
                                                  return attribute.instanceAttributeId == instanceAttributeId;
                                              });
        const auto attribute =
            m_attributeModel->findAttribute(precitec::storage::compatibility::toQt(attributeIt->attributeId));
        auto parameterName = QString();
        auto userLevel = 1; // it is for testing (in reality we have to have correspondent attribute file)
        if (attribute != nullptr)
        {
            parameterName = precitec::gui::LanguageSupport::instance()->getString(attribute->contentName());
            userLevel = attribute->userLevel();
        }

        return {QString::fromStdString(filter.name),
                groupInfo,
                filterType,
                parameterName,
                precitec::storage::compatibility::toQt(filter.id),
                userLevel};
    }
    return {};
}

std::string SeamGraphParametersWorksheet::lastNameFromNestedNamespace(const std::string &nestedNamespace)
{
    const auto firstCharacterAfterTheLastColonIt =
        (std::find_if(nestedNamespace.rbegin(), nestedNamespace.rend(),
                      [](auto item)
                      {
                          return ':' == item;
                      }).base());
    return std::string(firstCharacterAfterTheLastColonIt, nestedNamespace.end());
}

bool SeamGraphParametersWorksheet::checkUserLevel(int userLevel)
{
    using precitec::gui::components::user::UserManagement;
    switch (userLevel)
    {
    case 0: // Administrator
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin));
    case 1: // SuperUser
    {
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterSuperUser));
    };
    case 2: // GroupLeader
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterSuperUser)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterGroupLeader));
    case 3: // Operator
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterSuperUser)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterGroupLeader)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterOperator));
    default:
        return false;
    }
}

void SeamGraphParametersWorksheet::setSeam(Seam *seam)
{
    if (seam == m_seam)
    {
        return;
    }

    m_seam = seam;

    disconnect(m_seamDestroyed);
    if (m_seam)
    {
        m_seamDestroyed = connect(m_seam, &storage::Seam::destroyed, this,
                                  std::bind(&SeamGraphParametersWorksheet::setSeam, this, nullptr));
    }
    else
    {
        m_seamDestroyed = QMetaObject::Connection{};
    }
}
void SeamGraphParametersWorksheet::setProduct(Product *product)
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
                                     std::bind(&SeamGraphParametersWorksheet::setProduct, this, nullptr));
    }
    else
    {
        m_productDestroyed = QMetaObject::Connection{};
    }
}

void SeamGraphParametersWorksheet::setGraphModel(GraphModel *graphModel)
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
                                        std::bind(&SeamGraphParametersWorksheet::setGraphModel, this, nullptr));
    }
    else
    {
        m_graphModelDestroyed = QMetaObject::Connection{};
    }
}

void SeamGraphParametersWorksheet::setSubGraphModel(SubGraphModel *subGraphModel)
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
                                           std::bind(&SeamGraphParametersWorksheet::setSubGraphModel, this, nullptr));
    }
    else
    {
        m_subGraphModelDestroyed = QMetaObject::Connection{};
    }
}

void SeamGraphParametersWorksheet::setAttributeModel(AttributeModel *attributeModel)
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
                                            std::bind(&SeamGraphParametersWorksheet::setAttributeModel, this, nullptr));
    }
    else
    {
        m_attributeModelDestroyed = QMetaObject::Connection{};
    }
}

} // namespace precitec::gui