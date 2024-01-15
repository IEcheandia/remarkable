#pragma once
#include <memory>

namespace precitec
{

namespace interface
{


class ActionInterface
{
public:
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual ~ActionInterface() = default;
};

using ActionInterface_sp = std::shared_ptr<ActionInterface>;

} // namepsace interface
} // namepsace precitec
