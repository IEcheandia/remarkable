
/**
 * @file
 * @brief  Implementierung des Interface des trigger moduls
 * @details iniziiert die Datenaufnahem via Framegrabber
 * @copyright    Precitec Vision GmbH & Co. KG
 *
 * @author KIR / JS
 * @date   20.05.10
 * @version 0.1
 *
 *
 */

#ifndef COMMANDSERVER_H_
#define COMMANDSERVER_H_

#include <vector>
#include "Poco/Bugcheck.h"
#include "system/exception.h"

#include "event/triggerCmd.h"
#include "event/triggerCmd.handler.h"

#include "event/trigger.h"
#include "event/trigger.proxy.h"

#include "event/sensor.h"

#include "triggerController.h"
//#include "common/loggerClient.h"

#include "common/systemConfiguration.h"


namespace precitec
{
namespace trigger
{

	/// Stellt Funktionen des Trigger Server zur Verfuegung
	class CommandServer : public TTriggerCmd<AbstractInterface>
	{

		public:
			CommandServer(TriggerServer& p_rTriggerServer) :
				m_oTriggerController( &p_rTriggerServer ),
				m_oIDSave(0),
			    m_oSourceSave(0),
			   	m_oTriggerDistanceSave(0),
				m_oNbTriggersSave(0)
			{
			}



		public:
			/// einzelner Trigger kann sofort abgesetzt werden
			virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& context)
			{
				const auto oSensorIdIsAnImageId	=	[](int p_oSensorId) {
					return p_oSensorId >= interface::eImageSensorMin && p_oSensorId <= interface::eImageSensorMax; };

				if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnImageId) == true) {
					return; // this is not the appropriate receiver - abort
				}

				// aktiver Trigger stoppen (Beispielsweise wenn endless)
				resetImageNumber(0); // reset image number (counter)
				m_oTriggerController.singleshot(p_rSensorIds, context);
			}


            /// initalisiert und startet die Triggerquelle ueber mehrere Abschnitte
            virtual void burst(const std::vector<int>& p_rSensorIds, TriggerContext const& context, int source, TriggerInterval const& interval)
            {
                if ((SystemConfiguration::instance().getBool("ImageTriggerViaEncoderSignals", false)) && (interval.state() == workflow::eAutomaticMode))
                {
                    // do nothing if image trigger is created via encoder signals and this burst belongs to a automatic mode
                    return;
                }

                const auto oSensorIdIsAnImageId	=	[](int p_oSensorId) {
                    return p_oSensorId >= interface::eImageSensorMin && p_oSensorId <= interface::eImageSensorMax; };

                if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnImageId) == true) {
                    return; // this is not the appropriate receiver - abort
                }

                m_oIDSave = 0;
                m_oTriggerDistanceSave  = interval.triggerDistance();
                m_oNbTriggersSave 		= interval.nbTriggers();
                // TriggerSource generieren
                m_oTriggerController.setTriggerSource(TriggerSource(source));
                // Trigger starten
                resetImageNumber(0); // reset image number (counter)
                m_oTriggerController.burst(context, interval);
                //std::cout<<"Command server Values nach burst Kommando: "<<m_oTriggerDistanceSave<<", "<<m_oNbTriggersSave<<std::endl;
            }


			/// unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden
			virtual void cancel(const std::vector<int>& p_rSensorIds)
			{
				// aktiver Trigger stoppen (Beispielsweise wenn endless)
				std::cout<<"Trigger stoppen"<<std::endl;
				m_oTriggerController.stop();

			}

			/// unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden. Gibt zurueck ob schon angehalten war
			int cancelSpecial(const std::vector<int>& p_rSensorIds)
			{
				// aktiver Trigger stoppen (Beispielsweise wenn endless)
				std::cout<<"Trigger stoppen"<<std::endl;
				const int status = m_oTriggerController.stop();

				//status == 0: trigger war schon angehalten
				//status == 1: trigger wurde angehalten
				return status;

			}

			///reset imageNumber
			virtual void resetImageNumber(int id)	{
				std::cout << "CommandServer<Server>::resetImageNumber id: " << id << std::endl;
				m_oTriggerController.resetImageNumber();
			}

			virtual void setImageNumber(int id, int imgNo)
			{
				std::cout << "CommandServer<Server>::setImageNumber: imgNo " << imgNo << "\tid " << id <<  std::endl;
				m_oTriggerController.setImageNumber(imgNo);
			}

			//Start live timer (SW trigger) mit den intern gehaltenen Variablen
			virtual void burstAgain(void)
			{
				m_oTriggerController.burstAgain(m_oTriggerDistanceSave,m_oNbTriggersSave);

			}

			void setTestImagesPath( std::string const & _testImagesPath )
			{
				m_oTriggerController.setTestImagesPath( _testImagesPath );
			}

            void setTestProductInstance(const Poco::UUID &productInstance)
            {
                m_oTriggerController.setTestProductInstance(productInstance);
            }

            

		private:
			TriggerController m_oTriggerController;  ///< TriggerController

			TriggerContext m_oTriggerContext;
			TriggerInterval m_oInterval;


			int m_oIDSave;
			int m_oSourceSave;

			unsigned int m_oTriggerDistanceSave;
			unsigned int m_oNbTriggersSave;


	};

}
}

#endif /*COMMANDSERVER_H_*/
