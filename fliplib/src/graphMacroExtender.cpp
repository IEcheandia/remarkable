#include "fliplib/graphMacroExtender.h"
#include "fliplib/macroUUIDMapping.h"

#include "Poco/UUIDGenerator.h"

using Poco::UUID;
using Poco::UUIDGenerator;

namespace fliplib
{

void GraphMacroExtender::setMacroAndCorrespondentMacroGraph(const Macro *macro, std::unique_ptr<GraphContainer> macroGraph)
{
    m_macro = macro;
    m_macroGraph.swap(macroGraph);
}

void GraphMacroExtender::extendTargetGraph(GraphContainer *targetGraph)
{
    m_targetGraph = targetGraph;
    if (!m_macro || !m_macroGraph || !m_targetGraph)
    {
        return;
    }
    extendAndUniqueFilterDescriptions(m_targetGraph, m_macroGraph->filterDescriptions);
    extendInstanceFilters(m_targetGraph, m_macroGraph->instanceFilters,
                          m_macro);
    extendPipes(m_targetGraph);
    // TODO: cleanup macro in user class after extension?
    // TODO: Let us think about XML file fields:
    //     <MesswerttypList/>
    //     <FehlertypList/>
    //     <SensorTypList/>
    // TODO: Should we add the following entities to the targetGraphContainer and how:
    //    m_macroGraphContainer->errors;
    //    m_macroGraphContainer->parameterSet;
    //    m_macroGraphContainer->results;
    //    m_macroGraphContainer->sensors;
}

void GraphMacroExtender::extendAndUniqueFilterDescriptions(GraphContainer *targetGraphContainer,
                                                           std::vector<FilterDescription> &filterDescriptions)
{
    targetGraphContainer->filterDescriptions.reserve(targetGraphContainer->filterDescriptions.size() + filterDescriptions.size());
    targetGraphContainer->filterDescriptions.insert(targetGraphContainer->filterDescriptions.end(),
                                                    std::make_move_iterator(filterDescriptions.begin()),
                                                     std::make_move_iterator(filterDescriptions.end()));
    auto last = std::unique(targetGraphContainer->filterDescriptions.begin(),targetGraphContainer->filterDescriptions.end());
    targetGraphContainer->filterDescriptions.erase(last, targetGraphContainer->filterDescriptions.end());
    targetGraphContainer->filterDescriptions.shrink_to_fit();
}

void GraphMacroExtender::extendInstanceFilters(GraphContainer *targetGraphContainer,
                                               std::vector<InstanceFilter> &instanceFilters,
                                               const fliplib::Macro *macro)
{
    std::for_each(instanceFilters.begin(), instanceFilters.end(),
                [macro](auto &instanceFilter)
                {
                    // TODO Should we change position, ... and other parameters?
                    instanceFilter.id = MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(macro->id, instanceFilter.id);
                    instanceFilter.group = macro->group;
                    for (auto &attribute : instanceFilter.attributes)
                    {
                        attribute.instanceAttributeId = MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(macro->id, attribute.instanceAttributeId);
                    }
                });
    targetGraphContainer->instanceFilters.reserve(targetGraphContainer->instanceFilters.size() + instanceFilters.size());
    targetGraphContainer->instanceFilters.insert(targetGraphContainer->instanceFilters.end(),
                                                 std::make_move_iterator(instanceFilters.begin()),
                                                 std::make_move_iterator(instanceFilters.end()));
}

void GraphMacroExtender::extendPipes(GraphContainer *targetGraphContainer)
{
    std::vector<Pipe> transformedPipes;

    addTargetGraphPipesWithoutConnectors(targetGraphContainer, m_macro, transformedPipes);
    addMacroGraphPipesWithoutConnectors(m_macroGraph.get(), m_macro, transformedPipes);
    addExtendedInConnectorPipesAndPipesFromAnotherMacros(transformedPipes);
    addExtendedOutConnectorPipesAndPipesFromAnotherMacros(transformedPipes);

    targetGraphContainer->pipes = std::move(transformedPipes);
}

void GraphMacroExtender::addTargetGraphPipesWithoutConnectors(const GraphContainer *targetGraphContainer,
                                                              const Macro *macro,
                                                              std::vector<Pipe> &pipes)
{
    for (const auto &pipe : targetGraphContainer->pipes)
    {
        const auto hasInConnectorEnd =
            std::any_of(macro->inConnectors.begin(), macro->inConnectors.end(),
                        [&pipe](const auto &connector) { return connector.id == pipe.receiverConnectorId; });
        const auto hasOutConnectorBegin =
            std::any_of(macro->outConnectors.begin(), macro->outConnectors.end(),
                        [&pipe](const auto &connector) { return connector.id == pipe.senderConnectorId; });
        if (!hasInConnectorEnd && !hasOutConnectorBegin)
        {
            pipes.emplace_back(pipe);
        }
    }
}

void
GraphMacroExtender::addMacroGraphPipesWithoutConnectors(const GraphContainer *macroGraphContainer, const Macro *macro,
                                                        std::vector<Pipe> &pipes)
{
    for (const auto &pipe : macroGraphContainer->pipes)
    {
        const auto hasPipeInConnectorBegin =
            std::any_of(macro->inConnectors.begin(), macro->inConnectors.end(),
                        [&pipe](const auto &connector) { return connector.id == pipe.senderConnectorId; });
        const auto hasPipeOutConnectorEnd =
            std::any_of(macro->outConnectors.begin(), macro->outConnectors.end(),
                        [&pipe](const auto &connector) { return connector.id == pipe.receiverConnectorId; });
        if (!hasPipeInConnectorBegin && !hasPipeOutConnectorEnd)
        {
            auto innerPipe = pipe;
            innerPipe.receiver = MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(macro->id, pipe.receiver);
            innerPipe.sender = MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(macro->id, pipe.sender);
            pipes.emplace_back(std::move(innerPipe));
        }
    }
}

void GraphMacroExtender::addExtendedInConnectorPipesAndPipesFromAnotherMacros(std::vector<Pipe> &pipes)
{
    for (const auto &connector : m_macro->inConnectors)
    {
        for (const auto &pipe : m_targetGraph->pipes)
        {
            if (pipe.receiverConnectorId == connector.id)
            {
                if (pipe.receiver == m_macro->id)
                {
                    addExtendedPipesFromInConnectorToMacroFilters(pipe, connector, pipes);
                }
                else
                {
                    pipes.emplace_back(pipe);
                }
            }
        }
    }
}

void GraphMacroExtender::addExtendedPipesFromInConnectorToMacroFilters(const Pipe &targetContainerPipe,
                                                                       const Macro::Connector &connector,
                                                                       std::vector<Pipe> &pipes)
{
    for (const auto &currentPipe : m_macroGraph->pipes)
    {
        if (currentPipe.senderConnectorId == connector.id)
        {
            auto extendedPipe = targetContainerPipe;
            extendedPipe.id = UUIDGenerator::defaultGenerator().createRandom();
            extendedPipe.receiver = MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(m_macro->id, currentPipe.receiver);
            extendedPipe.receiverConnectorId = currentPipe.receiverConnectorId;
            extendedPipe.receiverConnectorGroup = currentPipe.receiverConnectorGroup;
            extendedPipe.receiverConnectorName = currentPipe.receiverConnectorName;
            // Do we need the following 2 lines?
            extendedPipe.extensions.reserve(extendedPipe.extensions.size() + currentPipe.extensions.size());
            extendedPipe.extensions.insert(extendedPipe.extensions.end(), currentPipe.extensions.begin(),
                                           currentPipe.extensions.end());
            pipes.emplace_back(std::move(extendedPipe));
        }
    }
}

void GraphMacroExtender::addExtendedOutConnectorPipesAndPipesFromAnotherMacros(std::vector<Pipe> &pipes)
{
    for (const auto &connector : m_macro->outConnectors)
    {
        for (const auto &pipe : m_targetGraph->pipes)
        {
            if (pipe.senderConnectorId == connector.id)
            {
                if (pipe.sender == m_macro->id)
                {
                    addExtendedPipesFromOutConnectorToTargetGraphFilters(pipe, connector, pipes);
                }
                else
                {
                    pipes.emplace_back(pipe);
                }

            }
        }
    }

}

void GraphMacroExtender::addExtendedPipesFromOutConnectorToTargetGraphFilters(const Pipe &macroContainerPipe,
                                                                              const Macro::Connector &connector,
                                                                              std::vector<Pipe> &pipes)
{
    for (const auto &currentPipe : m_macroGraph->pipes)
    {
        if (currentPipe.receiverConnectorId == connector.id)
        {
            auto extendedPipe = macroContainerPipe;
            extendedPipe.id = UUIDGenerator::defaultGenerator().createRandom();
            extendedPipe.sender = MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(m_macro->id, currentPipe.sender);
            extendedPipe.senderConnectorId = currentPipe.senderConnectorId;
            extendedPipe.senderConnectorName = currentPipe.senderConnectorName;
            // Do we need the following 2 lines?
            extendedPipe.extensions.reserve(extendedPipe.extensions.size() + currentPipe.extensions.size());
            extendedPipe.extensions.insert(extendedPipe.extensions.end(), currentPipe.extensions.begin(),
                                           currentPipe.extensions.end());
            pipes.emplace_back(std::move(extendedPipe));
        }
    }
}

} // namespace fliplib
