#ifndef FIELDBUSOUTPUTSSERVER_H_
#define FIELDBUSOUTPUTSSERVER_H_

#include "event/ethercatOutputs.handler.h"
#include "Fieldbus/Fieldbus.h"

namespace precitec
{

namespace ethercat
{

/**
 * FieldbusOutputsServer
 **/
class FieldbusOutputsServer : public TEthercatOutputs<AbstractInterface>
{
    public:

        /**
         * Ctor.
         * @param _service Service
         * @return void
         **/
        FieldbusOutputsServer(Fieldbus& p_rFieldbus);
        virtual ~FieldbusOutputsServer();

        virtual void ecatDigitalOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t value, uint8_t mask)
        {
        }

        virtual void ecatAnalogOut(EcatProductIndex productIndex, EcatInstance instance, EcatChannel channel, uint16_t value)
        {
        }

        virtual void ecatGatewayOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask)
        {
            m_rFieldbus.ecatGatewayOut(productIndex, instance, size, data, mask);
        }

        virtual void ecatEncoderOut(EcatProductIndex productIndex, EcatInstance instance, uint16_t command, uint32_t setCounterValue)
        {
        }

        virtual void ecatAxisOut(EcatProductIndex productIndex, EcatInstance instance, EcatAxisOutput axisOutput)
        {
        }

        virtual void ecatRequestSlaveInfo(void)
        {
            m_rFieldbus.ecatRequestSlaveInfo();
        }

        void sendAllData(bool enable) override
        {
            m_rFieldbus.sendAllData(enable);
        }

        virtual void ecatFRONTENDOut(EcatProductIndex productIndex, EcatInstance instance, EcatFRONTENDOutput frontendOutput)
        {
        }

    private:
        Fieldbus &m_rFieldbus;
};

} // namespace ethercat

} // namespace precitec

#endif // FIELDBUSOUTPUTSSERVER_H_

