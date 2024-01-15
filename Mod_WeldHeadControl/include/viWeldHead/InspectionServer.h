#ifndef INSPECTIONSERVER_H_
#define INSPECTIONSERVER_H_

#include "event/inspection.handler.h"
#include "WeldingHeadControl.h"

namespace precitec
{

namespace ethercat
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
		InspectionServer(WeldingHeadControl& p_rWeldingHeadControl);
		virtual ~InspectionServer();

		/**
		 * Automatikbetrieb Start - Inspektion Bauteil aktiv, Grafen werden Produktspezifisch aufgebaut
		 * @param producttype producttype
		 * @param productnumber productnumber
		 */
		virtual void startAutomaticmode(uint32_t producttype, uint32_t productnumber, const std::string& p_rExtendedProductInfo)
		{
			m_rWeldingHeadControl.startAutomaticmode(producttype, productnumber);
		}

		/**
		 * Automatikbetrieb Stop
		 */
		virtual void stopAutomaticmode()
		{
			m_rWeldingHeadControl.stopAutomaticmode();
		}

		/**
		 * Startsignal fuer Naht
		 * @param seamnumber Nahtnummer
		 */
		virtual void start(int seamnumber)
		{
			m_rWeldingHeadControl.start(seamnumber);
		}

		/**
		 * Stopsignal fuer Naht
		 * @param seamnumber Nahtnummer
		 */
		virtual void end(int seamnumber)
		{
			m_rWeldingHeadControl.end(seamnumber);
		}

		/**
		 * Startsignal fuer Nahtfolge
		 * @param seamsequence Nahtfolgenummer
		 */
		virtual void info(int seamsequence)
		{
			m_rWeldingHeadControl.info(seamsequence);
		}

		/**
		 * Lininelaser Ein- bzw. Ausschalten
		 * @param onoff
		 */
		virtual void linelaser (bool onoff)
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
		virtual void seamPreStart (int seamnumber )
		{
		}

	private:
		WeldingHeadControl &m_rWeldingHeadControl;
};

} // namespace ethercat

} // namespace precitec

#endif // INSPECTIONSERVER_H_

