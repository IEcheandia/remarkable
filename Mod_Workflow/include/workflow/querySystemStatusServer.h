#pragma once

#include "event/querySystemStatus.interface.h"

namespace precitec
{
namespace workflow
{
class StateContext;

class QuerySystemStatusServer : public interface::TQuerySystemStatus<interface::AbstractInterface>
{
public:
    void requestOperationState() override;

    void setStateContext(const Poco::SharedPtr<StateContext> &context)
    {
        m_stateContext = context;
    }

private:
    Poco::SharedPtr<StateContext> m_stateContext;
};

}
}
