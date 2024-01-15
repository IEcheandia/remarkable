#pragma once

#include "abstractState.h"

namespace precitec
{
namespace workflow
{

/**
 * The UpdateLiveMode state is only used from LiveMode::updateLiveMode().
 * It stops live mode and performs the DBCheck.
 **/
class UpdateLiveMode : public AbstractState
{
public:

    UpdateLiveMode(StateContext* p_pContext);

    void startLive(const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam) override;
};

} // namespace workflow
} // namespace precitec
