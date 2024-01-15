#ifndef INSPECTIONCMDSERVER_H_
#define INSPECTIONCMDSERVER_H_

#include "event/inspectionCmd.handler.h"
#include "WeldingHeadControl.h"

namespace precitec
{

namespace ethercat
{

/**
 * InspectionCmdServer
 **/
class InspectionCmdServer : public TInspectionCmd<AbstractInterface>
{
	public:

		/**
		 * Ctor.
		 * @param _service Service
		 * @return void
		 **/
		InspectionCmdServer(WeldingHeadControl& p_rWeldingHeadControl);
		virtual ~InspectionCmdServer();

		/**
		 * Kritischer Fehler -> gehe in NotReady Zustand
		 * @param relatedException Ursache fuer notReady Zustand
		 */
		virtual void signalNotReady(int relatedException)
		{
		}

		/**
		 * Kalibration anfordern
		 * @param type Typ der angeforderten Kalibration
		 */
		virtual void requestCalibration(int type)
		{
		}

		/**
		 * Livebetrieb start
		 * @param ProductId ProductId
		 * @param seam Nahtnummer
		 * @param seamseries Nahtfolgenummer
		 */
		virtual void startLivemode(const Poco::UUID& ProductId, int seam, int seamseries)
		{
		}

		/**
		 * Livebetrieb stop
		 */
		virtual void stopLivemode()
		{
		}

		/**
		 * Simulation start
		 * @param ProductId ProductId
		 * @param mode mode
		 */
		virtual void startSimulation(const Poco::UUID& ProductId, int mode)
		{
		}

		/**
		 * Simulation stop
		 */
		virtual void stopSimulation()
		{
		}

		/**
		 * Start ProductTeachInMode
		 */
		virtual void startProductTeachInMode()
		{
		}

		/**
		 * Abort ProductTeachInMode
		 */
		virtual void abortProductTeachInMode()
		{
		}

		/**
		 * anstehenden Systemfehler quittieren
		 */
		virtual void quitSystemFault()
		{
		}

		/**
		 * Reset Summenfehler
		 */
		virtual void resetSumErrors()
		{
		}

		/**
		 * Notaus aktiviert
		 */
		virtual void emergencyStop()
		{
		}

		/**
		 * Notaus deaktiviert
		 */
		virtual void resetEmergencyStop()
		{
		}

        void signalReady(int /*relatedException*/) override
        {
        }

	private:
		WeldingHeadControl &m_rWeldingHeadControl;
};

} // namespace ethercat

} // namespace precitec

#endif // INSPECTIONCMDSERVER_H_

