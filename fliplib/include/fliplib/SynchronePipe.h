///////////////////////////////////////////////////////////
//  SynchronePipe.h
//  Implementation of the Class SynchronePipe
//  Created on:      30-Okt-2007 14:19:36
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EB_39DCF860_3604_4adc_9AB6_6A1D742D41AA__INCLUDED_)
#define EB_39DCF860_3604_4adc_9AB6_6A1D742D41AA__INCLUDED_

#include <string>
#include <typeinfo>
#include <cassert>
#include <array>

#include "Poco/SharedPtr.h"
#include "Poco/BasicEvent.h"
#include "Poco/AbstractDelegate.h"

#include "fliplib/PipeEventArgs.h"
#include "fliplib/BasePipe.h"
#include "fliplib/BaseFilterInterface.h"

#include "common/defines.h"

namespace fliplib
{
	/**
	 * TEMPLATE SyncrhonePipe
	 *
	 * Die SynchronePipe speichert ein typisierters Ergebnis und signalisiert die Empfaengern das neue Daten vorhanden sind.
	 * Es koennen beliebig viele Empfaenger auf ein Ereignis warten.

	 * Folgenden Teilnehmer verwenden die Pipe:
	 * - Publisher Filter. Veroeffentlicht seine Ergebnisse ueber eine Pipe
	 * - Subscriber Filter. Abonnieren die Ergebnisse eines Publisher Filters oder fordern sie an.
	 *
	 * Mit der SynchronePipe kann sowohl das Push als auch das Pull Konzept implementiert werden. Beim Push Konzept berechnet der
	 * Publisher Filter erst das Ergebnis und signalisiert danach dem Subscriber Filter das neue Daten bereit stehen.
	 * Beim Pull Konzept fordert der Subscriber die Daten beim Publisher Filter erst an, dieser fuehrt die Berechnungen durch und
	 * benachrichtigt danach die Subscriber das neue Daten bereitstehen.
	 *
	 * Methoden fuer Push:
	 * write / signal
	 *
	 * Methoden fuer Pull:
	 * requestData / write / signal

	 * Um das Pull Konzept zu implementieren, muss der Publisher einen requestDataHandler implementieren und im Konstruktor
	 * der Pipe installieren.
	 *
	 */
	template <class TArgs>
	class SynchronePipe : public fliplib::BasePipe
	{
		public:
			typedef SynchronePipe TT;
			/**
			 * Konstruktor
			 *
			 * \param [in] parent Zeiger auf Absenderfilter
			 * \param [in] name Eindeutiger Name innerhalb eines Filters
			 */
			SynchronePipe(BaseFilterInterface* parent, const std::string& name)
				: BasePipe(parent, name), consumer_(0)
			{
				// Request Event wird nicht benoetigt
				requestEvent_.disable();

                init();
			}

			/**
			 * Konstruktor
			 *
			 * \param [in] parent Zeiger auf Absenderfilter
			 * \param [in] name Eindeutiger Name innerhalb eines Filters
			 * \param [in] contentType	Type der Pipe. Bsp: ImageFrame
		   * \param [in] channel		Fuer GraphEditor. Reihenfolge der Pipe im Filter
			 */
			SynchronePipe(BaseFilterInterface* parent, const std::string& name, const std::string& contentType, int channel = -1)
				: BasePipe(parent, name, contentType, channel), consumer_(0)
			{
				// Request Event wird nicht benoetigt
				requestEvent_.disable();

                init();
			}

			/**
			 * Konstruktor
			 *
			 * \param [in] parent Zeiger auf Absenderfilter
			 * \param [in] name Eindeutiger Name innerhalb eines Filters
			 * \param [in] requestDataHandler Wird aufgerufen wenn ein Subscriber requestData aufruft
			 */
			SynchronePipe(BaseFilterInterface* parent, const std::string& name, const NotificationHandler& requestDataHandler)
				: BasePipe(parent, name), data_(0)
			{
				requestEvent_ += requestDataHandler;

                init();
			}

			/**
			 * Destruktor
			 */
			virtual ~SynchronePipe()
			{
				requestEvent_.clear();
				signalEvent_.clear();
				//data_ = NULL;
			}

			/**
			 * Der Empfaenger installiert seine Handlermethode in der Pipe. Sobald der Sender signalisiert, dass
			 * neue Daten in der Pipe stehen, wird der SignalHandler aufgerufen. Der Signalhandler wird als private
			 * Methode im Empfaenger implementiert und muss folgende Signatur haben. void(const void* sender, PipeEventArgs& e).
			 * Der Parameter void* sender zeigt auf den Absenderfilter. (siehe BasePipe)
			 *
			 * \param [in] signalHandler	Referenz auf die Handlermethode
			 */
			void install(const NotificationHandler& signalHandler)
			{
				signalEvent_ += signalHandler;
				++consumer_;
			}

			/**
			 * Deinstalliert den NotificationHandler fuer diese Pipe
			 *
			 * \param [in] signalHandler Referenz auf eine Funktion mit folgender Signatur void(const void* sender, PipeEventArgs& e).
			 */
			void uninstall(const NotificationHandler& signalHandler)
			{
                if (consumer_ == 0)
                {
                    return;
                }

				signalEvent_ -= signalHandler;
				--consumer_;
			}

			/**
			 * Deinstalliert alle NotificationHandler fuer diese Pipe
			 */
			void uninstallAll()
			{
				signalEvent_.clear();
				consumer_ = 0;
			}

			/**
			 * Signalisiert den Subscriber, dass Daten in der Pipe stehen und mit read() gelesen werden koennen.
			 *
			 * Caller: Publisher
			 */
			virtual void signal(int	 p_oImgNb)
			{
				// on each send, we signal that the send was done. dataSend_ will be set to false in the read() function
				dataAvailable_[p_oImgNb % g_oNbPar] = true;
				fliplib::PipeEventArgs eventArgs(this, p_oImgNb);
				signalEvent_.notify(parent_, eventArgs);
			}

			/**
			 * Schreibt erst die Daten in die Pipe und signalisiert danach den Subscriber, das neue Daten aus der Pipe gelesen
			 * werden koennen.
			 *
			 * Caller: Publisher
			 */
			void signal (const TArgs & packet)
			{
                if (linked() == false)
                {
                    return;
                }

				write (packet);
				signal(packet.context().imageNumber());
			}

			/**
			 * Signalisiert explizit ein NIO-Ereignis
			 *
			 * Caller: Publisher
			 */
			void signalNIO(const TArgs &packet)
			{
                if (linked() == false)
                {
                    return;
                }

				write (packet);
				signal(packet.context().imageNumber());
			}

			/**
			 * Ein Subscriber fordert vom Publisher neue Daten an.
			 *
			 * Caller: Subscriber
			 */
			virtual void requestData()
			{
				if (requestEvent_.isEnabled())
				{
					fliplib::PipeEventArgs eventArgs(this, 0);  // DUMMY
                    throw NotImplementedException{ "Parallelism only implemented for push concept" };
					requestEvent_.notify(parent_, eventArgs);
				}
			}

			/**
			 * Speichert ein Zeiger auf ein typisiertes Datum
			 *
			 * Caller: Publisher
			 *
			 * \param [in] data	Zeiger auf das Ergebnis
			 */
			void write (const TArgs & data)
			{              
                const auto oImgNb                   = data.context().imageNumber();
                const auto oIdx                     = oImgNb % g_oNbPar; // choose the right slot depending on the image number

				data_[oIdx]     = data;
                //precitec::wmLog(precitec::eInfo, "%i %s %s WRITE at %i.\n", oImgNbNew, parent_->name().c_str(), name_.c_str(), oIdx); // debug
			}

			/**
			 * Writes the same @p data to all parallelization slots.
			 *
			 */
			void writeToAllSlots (const TArgs & data)
			{
				for (std::size_t i = 0; i < g_oNbPar; i++)
				{
					data_[i] = data;
				}
			}

			/**
			 * 	Ein Datenpacket wird aus der Pipe gelesen. Die Referenz auf die Daten wird nicht geloescht.
			 * 	Die Daten duerfen nur einmal gelesen werden.
			 */
			//TArgs read() const
			//{
   //             dataAvailable_ = false;
   //             assert(false);
   //             return data_[0]; // REMOVE
			//}

			const TArgs& read(int p_oImgNb) const
			{    
                dataAvailable_[p_oImgNb % g_oNbPar] = false;

                const auto oIdx                     = p_oImgNb % g_oNbPar; // choose the right slot depending on the image number

                //precitec::wmLog(precitec::eInfo, "%i %s %s READ at %i.\n", p_oImgNb, parent_->name().c_str(), name_.c_str(), p_oImgNb % g_oNbPar); // debug
                return data_[oIdx];
			}

			/**
			* Prueft, ob Daten abgeholt werden koennen/ worden sind.
			*/
			bool dataAvailable(int p_oImageNumber) const
			{
				return dataAvailable_[p_oImageNumber % g_oNbPar];
			}

			/***
			 *
			 * Prueft ob tatsaechlich ein Konsument an der Pipe horcht. (true)
			 */
			bool linked() const
			{
				return (consumer_ != 0);
			}

			/***
			*
			* Gibt die Bildnummer zu den in der Pipe enthaltenen Daten aus. Entsprechend p_oCounter wird die pipe ausgewaehlt.
			*/
			int getImageNumber(int p_oCounter) const { return data_[p_oCounter % g_oNbPar].context().imageNumber(); }

		private:

            void init()
            {
                for (std::size_t i = 0; i < g_oNbParMax; i++)
                {
                    dataAvailable_[i] = false;
                }
            }

			Poco::BasicEvent<PipeEventArgs> 	signalEvent_;
			Poco::BasicEvent<PipeEventArgs> 	requestEvent_;
			std::array<TArgs, g_oNbParMax>  	data_;
			int 								consumer_; // Anzahl Konsumenten
			mutable std::array<bool, g_oNbParMax> dataAvailable_; ///< Will be set to false after each read. A signal makes this value true, until the next read changes it again...

		protected:
			const std::type_info& getType()	const {	return typeid(TArgs); }
			// Methode wird von der Basisklasse aufgerufen, wenn nach dem Type dieser Pipe gefragt wird
	};

}

#endif // !defined(EB_39DCF860_3604_4adc_9AB6_6A1D742D41AA__INCLUDED_)

