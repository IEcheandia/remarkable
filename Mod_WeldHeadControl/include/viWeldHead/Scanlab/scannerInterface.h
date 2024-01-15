#pragma once

#include <string>
#include "common/systemConfiguration.h"

struct InitData
{
    std::string ipAddress;          //Add ipAddress for RTC6 part in constructor too?
    std::string correctionFile;     //Is custom correction file selected?
    precitec::interface::LensType lens;
    precitec::interface::ScannerModel scannerModel;
    precitec::interface::ScannerModel scannerController;
};
