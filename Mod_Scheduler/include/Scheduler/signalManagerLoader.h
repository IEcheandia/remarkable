#pragma once

#include <memory>
#include <set>

#include "signalManager.h"

namespace precitec
{
namespace scheduler
{

class SignalManagerLoader
{
public:
    static std::set<SignalManager> load(std::istream &input);
};

}
}
