#ifndef INSPECTIONSERVER_H_
#define INSPECTIONSERVER_H_

#include "event/inspection.handler.h"
#include "CHRCommunication/CHRCommunication.h"

namespace precitec
{

namespace grabber
{

/**
 * InspectionServer
 **/
class InspectionServer : public TInspection<AbstractInterface>
{
public:
    /**
        * Ctor.
        * @param _service Service
        * @return void
        **/
    InspectionServer(CHRCommunication& p_rCHRCommunication);
    virtual ~InspectionServer();

    /**
        * Automatikbetrieb Start - Inspektion Bauteil aktiv, Grafen werden Produktspezifisch aufgebaut
        * @param producttype producttype
        * @param productnumber productnumber
        */
    virtual void startAutomaticmode(uint32_t producttype, uint32_t productnumber, const std::string& p_rExtendedProductInfo)
    {
        m_rCHRCommunication.startAutomaticmode(producttype, productnumber);
    }

    /**
        * Automatikbetrieb Stop
        */
    virtual void stopAutomaticmode()
    {
        m_rCHRCommunication.stopAutomaticmode();
    }

    /**
        * Startsignal fuer Naht
        * @param seamnumber Nahtnummer
        */
    virtual void start(int seamnumber)
    {
        m_rCHRCommunication.start(seamnumber);
    }

    /**
        * Stopsignal fuer Naht
        * @param seamnumber Nahtnummer
        */
    virtual void end(int seamnumber)
    {
        m_rCHRCommunication.end(seamnumber);
    }

    /**
        * Startsignal fuer Nahtfolge
        * @param seamsequence Nahtfolgenummer
        */
    virtual void info(int seamsequence)
    {
    }

    /**
        * Lininelaser Ein- bzw. Ausschalten
        * @param onoff
        */
    virtual void linelaser(bool onoff)
    {
    }

    /**
        * Startsignal fuer Kalibration
        */
    virtual void startCalibration()
    {
    }

    /**
        * Stopsignal fuer Kalibration
        */
    virtual void stopCalibration()
    {
    }

    /**
        * Naht Vor-Start
        * @param action
        */
    virtual void seamPreStart(int seamnumber)
    {
    }

private:
    CHRCommunication& m_rCHRCommunication;
};

} // namespace grabber

} // namespace precitec

#endif // INSPECTIONSERVER_H_
