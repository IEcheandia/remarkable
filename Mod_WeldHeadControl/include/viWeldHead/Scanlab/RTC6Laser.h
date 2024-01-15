#pragma once

#include "common/systemConfiguration.h"

using precitec::interface::ScannerModel;

namespace RTC6
{
class Laser
{
    public:
        Laser();
        Laser(unsigned int p_oLaserMode, unsigned int p_oAnalogOutput, unsigned int p_oMinAnalogOutput, unsigned int p_oMaxAnalogOutput, long int p_oLaserOnDelay, long int p_oLaserOffDelay);
        ~Laser();
        void set_LaserMode(unsigned int p_oLaserMode);
        void set_AnalogOutput(unsigned int p_oAnalogOutput, unsigned int p_oMinAnalogOutput, unsigned int p_oMaxAnalogOutput);
        void set_LaserDelays(long int p_oLaserOnDelay, long int p_oLaserOffDelay);
        unsigned int get_LaserMode();
        void get_AnalogOutput(unsigned int& p_rAnalogOutput, unsigned int& p_rMinAnalogOutput, unsigned int& p_rMaxAnalogOutput) const;
        void get_LaserDelays(long int& p_rLaserOnDelay, long int& p_rLaserOffDelay) const;
        void init();
        void disable_LaserUnit();

        ScannerModel scannerModel()
        {
            return m_scannerModel;
        }
        void setScannerModel(ScannerModel newScannerModel);

        void setLaserControl();

    private:
        ScannerModel m_scannerModel = ScannerModel::ScanlabScanner;
        unsigned int m_oLaserMode;
        unsigned int m_oAnalogOutput;
        unsigned int m_oMinAnalogOutput;
        unsigned int m_oMaxAnalogOutput;
        long int m_oLaserOnDelay;
        long int m_oLaserOffDelay;
};
}
