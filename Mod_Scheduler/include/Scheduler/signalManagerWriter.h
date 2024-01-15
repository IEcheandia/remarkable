#pragma once

#include <iostream>

#include <set>
#include <memory>

#include "signalManager.h"

namespace precitec
{
namespace scheduler
{

class SignalManagerWriter
{
public:
    bool write(std::ostream &output, const std::set<SignalManager> &SignalManagers);
};

}
}