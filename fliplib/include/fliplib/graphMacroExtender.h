#pragma once

#include "graphContainer.h"

#include <Poco/UUID.h>

namespace fliplib
{

class GraphMacroExtender
{
public:
    GraphMacroExtender() = default;
    ~GraphMacroExtender() = default;

    void setMacroAndCorrespondentMacroGraph(const Macro *macro, std::unique_ptr<GraphContainer> macroGraph);

    void extendTargetGraph(GraphContainer *targetGraph);

private:
    static void extendAndUniqueFilterDescriptions(GraphContainer *targetGraphContainer,
                                                  std::vector<FilterDescription> &filterDescriptions);
    static void extendInstanceFilters(GraphContainer *targetGraphContainer,
                                      std::vector<InstanceFilter> &instanceFilters,
                                      const fliplib::Macro *macro);

    void extendPipes(GraphContainer *targetGraphContainer);
    static void addTargetGraphPipesWithoutConnectors(const GraphContainer *targetGraphContainer, const Macro *macro,
                                                     std::vector<Pipe> &pipes);
    static void addMacroGraphPipesWithoutConnectors(const GraphContainer *macroGraphContainer, const Macro *macro,
                                                    std::vector<Pipe> &pipes);
    void addExtendedInConnectorPipesAndPipesFromAnotherMacros(std::vector<Pipe> &pipes);
    void addExtendedPipesFromInConnectorToMacroFilters(const Pipe &targetContainerPipe,
                                                       const Macro::Connector &connector,
                                                       std::vector<Pipe> &pipes);
    void addExtendedOutConnectorPipesAndPipesFromAnotherMacros(std::vector<Pipe> &pipes);
    void addExtendedPipesFromOutConnectorToTargetGraphFilters(const Pipe &macroContainerPipe,
                                                              const Macro::Connector &connector,
                                                              std::vector<Pipe> &pipes);

    std::unique_ptr<GraphContainer> m_macroGraph = nullptr;
    const Macro *m_macro = nullptr;
    GraphContainer *m_targetGraph = nullptr;
};

}
