#pragma once

#include "event/productTeachIn.h"
#include "event/productTeachIn.interface.h"


namespace precitec
{

using namespace precitec::interface;

namespace Simulation
{

class ProductTeachInServer : public TProductTeachIn<AbstractInterface>
{
public:
    ProductTeachInServer(){}
    ~ProductTeachInServer(){}

    void start(int, int) override {}
    void end(xLong) override {}
    void startAutomatic(int) override {}
    void stopAutomatic() override {}
};

}
}
