#pragma once
#include "abstractNodeController.h"
#include <Poco/UUID.h>

namespace qan
{
class Node;
}

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{
class FilterPort;
class FilterGroup;

class PlausibilityController : public AbstractNodeController
{
    Q_OBJECT
public:
    enum class InvalidIDType
    {
        Filter = 0,
        Attribute,
        Variant,
        Pipe,
        Port,
        Sensor,
        Error,
        Result,
        Macro
    };
    Q_ENUM(InvalidIDType)

    PlausibilityController(QObject *parent = nullptr);
    ~PlausibilityController() override;

    const std::vector<qan::Node*> &invalidModel()
    {
        return m_invalidNodes;
    }

    const std::vector<std::pair<Poco::UUID, InvalidIDType>> &invalidIDs()
    {
        return m_invalidIDs;
    }

    //Plausibility check to check if the graph makes sense before they are saved (Check if input-pipes and output-pipes are connected (Filter); Check if ports are connected (Ports))
    Q_INVOKABLE bool plausibilityCheck(bool checkInsignificantStuff = false);
    Q_INVOKABLE bool checkIDs();

    bool checkGroup(FilterGroup *group);

private:
    bool checkFilter(qan::Node* node) const;
    bool checkNodeInGroup(qan::Node* node) const;
    FilterPort* checkPort(qan::Node* node, bool checkInsignificantStuff = false);
    void addIDToInvalidIDs(const Poco::UUID &id, InvalidIDType type);
    void checkFilterAndAttributeIDs();
    template <class InputIterator> void checkInstanceID(InputIterator first, InputIterator last, InvalidIDType type);

    std::vector<qan::Node*> m_invalidNodes;
    std::vector<std::pair<Poco::UUID, InvalidIDType>> m_invalidIDs;
};

}
}
}
}
