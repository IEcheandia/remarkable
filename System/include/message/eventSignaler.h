#ifndef EVENT_SIGNALER_H_
#define EVENT_SIGNALER_H_
#pragma once

/**
* signaler.h
*
*  Created on: 10.05.2010
*      Author: Wolfgang Reichl
*/


#include <assert.h>

#include <string.h>
#include <vector>
#include <list>
#include <mutex>

#include "SystemManifest.h"

#include "Poco/SharedPtr.h"
#include "server/interface.h"
#include "protocol/protocol.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"
#include "system/sharedMem.h"
#include "module/interfaces.h"

#include <sys/types.h> // wg fifo
#include <sys/stat.h>  // wg fifo

namespace precitec
{
	namespace system
	{
	using module::Interfaces;
	using interface::MessageInfo;

		namespace message
		{

			/**
			* Der Signaler sorgt dafuer dass die Events auf die verschiedenen
			* Empfaenger (Subscriber) verteilt werden. Da jedes Event individuell
			* von Subscribern abbonniert werden kann, gibt es fuer jedes Event Sendelisten.
			*
			* Pro Subscriber wird eine Message abgeschickt.
			*/
			class SYSTEM_API EventSignaler	{
			public:
				/// Das Protokoll muss nicht uebergeben werden es sowieso dynamisch gesetzt wird
				EventSignaler(int numEvents, int maxMessagSize, int numBuffers);
				EventSignaler(MessageInfo const& info);
				/// loescht Listen
				~EventSignaler();
			public:
				void initSingleEvent(int eventNum, Protocol &subscriber);
				/// Das erste was der Proxy aufruft bevor er die Argumente marshalt
				void initMessage(int eventNum);

				/// Verpackt die Parameterwerte typsicher fuer Einzel-Objekte
				template <class T>
				void marshalSingle(T const& value, MessageBuffer &buffer);
				template <class T>
				void marshal(T const& value);
				/// Verpackt die Parameterwerte typsicher fuer Vektor-Objekte
				template <class T>
				void marshalSingle(T const& value, MessageBuffer &buffer, int length);
				template <class T>
				void marshal(T const& value, int length);
				/// Abschluss des Proxies. Das Event wird versandt
				void send();

                void lock()
                {
                    m_sendMutex.lock();
                }
                void unlock()
                {
                    m_sendMutex.unlock();
                }
			public:
				/// fuer internen Gebrauch etwa umm ShutdownMsg zu verschicken
				void sendEventToProtocol(int eventNum, Protocol &protocol);
				/// regelt die Pufferangelegenheiten, die Pro Schnittstelle anfallen nicht pro Event
				void addSubscriber(std::string p_oName, SmpProtocolInfo & protInfo);
				/// ein Send triggert ueber die Observer das Verschicken an verschiedene Subscriber
				void addSubscriber(int eventNum, SmpProtocolInfo & protInfo);
				/// Entfernt Subscriber aus Sendliste
				void removeSubscriber(int msgNum, SmpProtocolInfo const& protInfo);
				/// Ausgabe wg Nice-Class
				friend std::ostream &operator <<(std::ostream &os, EventSignaler const& s);
				/// Zugriff auf pasenden SendBuffer_
				MessageBuffer &sendBuffer(int protocolType) const { return *sendBuffer_[protocolType]; }
			private:
				// hier kann aus Performancegruenden ein angepasster SharedPtr ohne Mutex gewaehlt werden (hoffentlich)
				typedef Poco::SharedPtr<Protocol>		SmpSubscriber;
				typedef std::list<SmpSubscriber> 	 	SubscriberList;
				typedef SubscriberList::iterator		SubscriberIter;
				typedef SubscriberList::const_iterator		SubscriberCIter;
			private:

				/// finde Subscriber in SubscriberListe
				SubscriberIter findSubscriber(int eventNum, SmpSubscriber const&observer);
				/// finde Subscriber in SenderListe anhand der ProtokollInfo
				SubscriberIter findSubscriber(SmpProtocolInfo const& protInfo);
				/// finde Subscriber in SenderListe anhand der ProtokollInfo
				SubscriberIter findSubscriber(ProtocolType protType);

			private:
				/**
				 * Klasse die TypInformation fuer POD oder Klasse haelt (in TypeHolder::Type)
				 * Die Idee ist somit reine Typinformationen an Template-Funktionen uebergeben zu koennen
				 */
				template <class T> struct TypeHolder {
					typedef T Type;
					TypeHolder() {}
				};
				/// fuer jedes Event eine eigene Liste der Subscriber
				std::vector<SubscriberList> subscriberLists_;
				/// fuer jeden Protokolltyp (UDP, FTP, ...) ein Puffer, hier muss wohl auf inter/intra Prozess Optimierungen verzichtet werden
				MessageBuffer *sendBuffer_[NumActiveProtocols];
				/// Zwischenspeicher fuer Subscriberliste des akuellen Events
				SubscriberList *currList_;
				/// nur fuer das finale Aufraeumen
				SubscriberList allSubscribers_;
				/// Optimierung, verwenden mehrere Subscriber das gleiche Protokoll, braucht man nur einmal marshallen
				bool bufferProcessed_[NumActiveProtocols]; //
				/// SharedMem fuer qnx Pulse
				SharedMem	sharedMem_;
				/// Groesse der SharedMem in Bytes
				int	sendBufferSize_; ///< ggf wg Nachschauen im ShMem (fd) redundant
				int	maxMessageSize_; ///< max Groesse einer einzelnen MEssage = Groesse eines einzelnen Puffers
				/// fuer den einfachen Fall eines einzigen Empfaenger Cache fuer den richtigen Puffer
				MessageBuffer *singleBuffer_;
				/// info wg Debugangaben, insbesondere interfaceId
				MessageInfo info_;
                std::mutex m_sendMutex;
			}; // EventSignaler

			template <class T>
			void EventSignaler::marshalSingle(T const& value, MessageBuffer &buffer) {
				// Puffer von Subscribern, die das gleiche Protokoll verwernden, werden nur einmal gefuellt
				Serializer<T, FindSerializer<T, Single>::Value> ::serialize( buffer, value );
			}

			template <class T>
			void EventSignaler::marshal(T const& value) {
				if (currList_->size()==1) { marshalSingle(value, *singleBuffer_);	return;	}
				SubscriberIter subscriber = currList_->begin();
				for (; subscriber!=currList_->end(); ++subscriber)	{
					ProtocolType currPColType = (*subscriber)->protocolType();
					bufferProcessed_[currPColType] = false;
				}

				for (subscriber = currList_->begin(); subscriber!=currList_->end(); ++subscriber)	{
					ProtocolType currPColType = (*subscriber)->protocolType();
					if (!bufferProcessed_[currPColType]) {
						// Puffer von Subscribern, die das gleiche Protokoll verwernden, werden nur einmal gefuellt
						Serializer<T, FindSerializer<T, Single>::Value> ::serialize( *sendBuffer_[currPColType], value );
						bufferProcessed_[currPColType] = true;
					} // if
				} // for
			}

			template <class T>
			void EventSignaler::marshalSingle(T const& value, MessageBuffer &buffer, int length) {
				Serializer<T, FindSerializer<T, Vector>::Value> ::serialize( buffer, value, length);
			}

			template <class T>
			void EventSignaler::marshal(T const& value, int length) {
				if (currList_->size()==1) { marshalSingle(value, *singleBuffer_, length);	return;	}
				SubscriberIter subscriber = currList_->begin();
				for (; subscriber!=currList_->end(); ++subscriber)	{
					ProtocolType currPColType = (*subscriber)->protocolType();
					bufferProcessed_[currPColType] = false;
				}
				for (subscriber = currList_->begin(); subscriber!=currList_->end(); ++subscriber)	{
					ProtocolType currPColType = (*subscriber)->protocolType();
					if (!bufferProcessed_[currPColType]) {
						// Puffer von Subscribern, die das gleiche Protokoll verwernden, werden nur einmal gefuellt
						Serializer<T, FindSerializer<T, Vector>::Value> ::serialize( *sendBuffer_[currPColType], value, length);
						bufferProcessed_[currPColType] = true;
					} // if
				} // for
			}

			inline std::ostream &operator <<(std::ostream &os, EventSignaler const& s) {
				os << "Server<EventProxy>";
				EventSignaler::SubscriberCIter subscriber = s.allSubscribers_.begin();
				if (subscriber!=s.allSubscribers_.end()) {
					if (subscriber->isNull()) {
						os << " NULL ";
					} else {
						os << " : " << (*subscriber)->protocolInfo();
					}
				}
				return os;
			}

		} // namespace message
	} // namespace system
} // namespace precitec


#endif // EVENT_SIGNALER_H_
