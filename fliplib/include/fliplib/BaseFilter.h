///////////////////////////////////////////////////////////
//  BaseFilter.h
//  Implementation of the Class BaseFilter
//  Created on:      30-Okt-2007 15:11:55
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_586073B0_669B_471c_8A98_A0BFF4D6478E__INCLUDED_)
#define EA_586073B0_669B_471c_8A98_A0BFF4D6478E__INCLUDED_

#include <map>
#include <string>

#include "Poco/Condition.h"
#include "Poco/UUID.h"
#include "Poco/SharedPtr.h"
#include "Poco/ThreadLocal.h"
#include "fliplib/Fliplib.h"
#include "fliplib/BasePipe.h"
#include "fliplib/BaseFilterInterface.h"
#include "fliplib/PipeGroupEvent.h"
#include "fliplib/Parameter.h"
#include "fliplib/ParameterContainer.h"
#include "fliplib/AbstractFilterVisitor.h"
#include "fliplib/PipeConnector.h"
#include <atomic>

#include "system/timer.h"


namespace fliplib
{

	class FLIPLIB_API PipeInfo
	{
		typedef std::map<std::string, std::string> AttributeMap;

	public:
		PipeInfo(const std::string& name, const std::string& contentType, BasePipe* pipe) :
			name_(name), contentType_(contentType), pipe_(pipe) {}

		const std::string& name() 			const { return name_;			}
		const std::string& contentType() 	const { return contentType_;	}
		BasePipe* pipe() 					const { return pipe_; 			}

		void setAttribute(const std::string&  key, const std::string& value);
		AttributeMap & getAttribute();

		bool isInputPipe () const { return pipe_ == NULL;	 }
		bool isOutputPipe() const { return !isInputPipe(); }

		std::string toXml() const;

	private:
			// hide constructors
		PipeInfo();
		PipeInfo(const PipeInfo &);
			// hide operator =
		PipeInfo& operator = (const PipeInfo& other);

	private:
		std::string name_;
		std::string contentType_;
		BasePipe* pipe_;
		AttributeMap attributes_;
	};


	/**
	 * Uniform Basisklasse fuer konkrete Filter.
	 *
	 * Jeder Filter muss innerhalb einer Library einen eindeutigen Namen besitzen. Der Name wird ueber den Konstruktor dieser Klasse
	 * initialisiert und kann nicht mehr geaendert werden.
	 *
	 * Die Aufgaben der Basisklasse sind wie folgt:
	 * - Veroeffentlichung eigener Ausgaenge. Jeder konkreter Filter stellt seine Ergebnisse aus den Berechungnen in _Pipes_ zur Verfuegung. Saemtliche Pipes werden in
	 *   in dieser Basisklasse registriert, damit sie vom GraphBuilder oder anderen Filtern jederzeit, ueber einen Iterator, abgefragt werden
	 *   koennen. (siehe registerOutput, unregisterOutput)
	 *
	 * - Abonnierung der Daten anderer Filter. Die Ergebnisse anderer Filter kann empfangen werden, indem der Filter auf ein Event einer
	 *   Pipe wartet. Sobald Daten in die Pipe geschrieben werden, wird eine Ereignis ausgeloest und eine definierte Methode (Handler) im
	 *   Empfaengerfilter wird aufgerufen. Der Empfaengerfilter kann die Daten nun aus der Pipe auslesen. Der Defaulthandler fuer heisst
	 *   Proceed und muss, wenn kein anderer definiert wurde, von der konkreten Klasse ueberladen werden.
	 *
	 * - Gruppierung mehrerer Pipes. Standardmaessig unterstuetzt die BaseFilterklasse die Gruppierung mehrerer Pipes. Dass bedeutet, der EventHandler
	 *   _proceed_ wird erst aufgerufen, wenn alle Pipes in einer Gruppe neue Daten signalisiert haben. Standardmaessig wird eine Gruppe unterstuetzt.
	 *   Die Unterstuetzun mehrerer Gruppen muss in der abgeleiteten Klasse implementiert werden.
	 *
	 * - Referenzierung auf ClassLoader. Die Filter werden in der Regel in ausgelagerten Bibliotheken implementiert. Solange eine Instanz
	 *   eines BaseFilters existiert, wird eien Referenz auf den ClassLoader gehalten und damit verhindert, dass der ClassLoader geloescht wird.
	 *
	 * Die Abonnierung der Pipes anderer Filter wird vom GraphBuilder initiert und besteht aus zwei Schritten
	 * 1. connectPipe 		(Die zu verbindende Pipe wird dem Filter uebergeben. Aufrufer GraphBuilder)
	 * 2. subscripe			(Der BaseFilter versucht sich intern mit der Pipe zu verbinden.)
	 *
	 */
	class FLIPLIB_API BaseFilter : public BaseFilterInterface, public FilterControlInterface
	{

	public:
		typedef std::map<std::string, PipeInfo*> PipeMap;
		typedef std::map<std::string, std::string> AttributeMap;

		/**
		 * Hilfsklasse zum auslesen der Pipes von einem Filter.
		 * \code
		 * for (BaseFilter::Iterator it = source.begin(); it != source.end(); ++it)
		 * {
		 * 	string name 	= it->first;		// Name der Pipe
		 * 	PipeInfo* pipe 	= it->second;		// Zeiger auf PipeInfo
		 * }
		 * \endcode
		 */
		class Iterator
		{
		public:
			typedef std::pair<std::string, PipeInfo*> Pair;

			Iterator(const PipeMap::const_iterator& it)
			{
				_it = it;
			}
			Iterator(const Iterator& it)
			{
				_it = it._it;
			}
			~Iterator()
			{
			}
			Iterator& operator = (const Iterator& it)
			{
				_it = it._it;
				return *this;
			}
			inline bool operator == (const Iterator& it) const
			{
				return _it == it._it;
			}
			inline bool operator != (const Iterator& it) const
			{
				return _it != it._it;
			}
			Iterator& operator ++ () // prefix
			{
				++_it;
				return *this;
			}
			Iterator operator ++ (int) // postfix
			{
				Iterator result(_it);
				++_it;
				return result;
			}
			inline const Pair* operator * () const
			{
				_pair.first  = _it->first;
				_pair.second = _it->second;
				return &_pair;
			}
			inline const Pair* operator -> () const
			{
				_pair.first  = _it->first;
				_pair.second = _it->second;
				return &_pair;
			}

			inline PipeInfo* getPipeInfo()
			{
				return _it->second;
			}

			inline BasePipe* getPipe()
			{
				return getPipeInfo()->pipe();
			}


		private:
			PipeMap::const_iterator _it;
			mutable Pair _pair;
		};
		/**
		 *  @brief	Verbosity level. Controls the amount of debug output information.
		 */
		enum VerbosityType {
			eNone  				= 0,			///< no verbosity
			eLow,								///< low verbosity
			eMedium,							///< medium verbosity
			eHigh,								///< high verbosity
			eMax,								///< max verbosity
			eVerbosityTypeMin	= eNone,		///< delimiter
			eVerbosityTypeMax	= eMax			///< delimiter
		}; // VerbosityType

		/**
		 * Initialisiert eine neue Instanz der BaseFilter Klasse. FilterID ist zufaellig!
		 * \param [in] name Eindeutiger Name des Filters
		 */
		BaseFilter(const std::string& name);

		BaseFilter(const BaseFilter &)=delete;

		/**
		 * Initialisiert eine neue Instanz der BaseFilter Klasse
		 * \param [in] name Eindeutiger Name des Filters
		 * \param [in] filterID Eindeutiger ID des Filters
		 */
		BaseFilter(const std::string& name, Poco::UUID const& filterID);

		virtual ~BaseFilter();		/// Destruktor

		/**
		 * Registriert eine Pipe in der Pipe-Verwaltung (threadsafe). Wird der Filter zerstoert werden saemtlich registrierte Pipes
		 * ebenfalls zerstoert.
		 *
		 * \param [in] pipe Zeiger auf zu Pipe.
		 * \param [in] Typenbeschreibung. Wird vom GraphEditorverwendet um Pipes zu verbinden. (Muss identisch sein)
		 * \param [in] channel. Reihenfolge der Pipes wie sie im GraphEditor angezeigt werden. Bei -1 wird die Reihenfolge automatisch gesetzt
		 */
		void registerPipe(BasePipe* pipe, const std::string& contentType, int channel=-1);
		void registerPipe(const std::string& pipeName, const std::string& contentType, int channel=-1);
	private:
		void registerPipe(BasePipe* pipe, const std::string& pipeName, const std::string& contentType, int channel);
	public:

		/**
		 * Loescht die Pipe aus der Pipe-Verwaltung (threadsafe); Der Aufrufer ist selber verantwortlich, dass die Pipe korrekt
		 * geloescht wird.
		 *
		 *  \param [in] pipe Zeiger auf die zu loeschende Pipe.
		 */
		void unregisterPipe(const BasePipe *pipe);
		void unregisterPipe(const std::string& pipeName);
        virtual bool isValidConnected() const;

	protected:
		/**
		 * Abonniert die Daten einer bestimmten Pipe. Um eigene Eventhandler zu implementieren oder das abonnieren der Daten zu verweigern
		 * muss diese Methode ueberladen werden. Standardmaessig werden alle Pipes abonniert und der Defaulthandler _proceed_ registriert.
		 * Die Events mehrere Pipes koennen in einer Gruppe zusammengefasst werden. Dadurch wird der Handler erst aufgerufen, wenn in allen
		 * Pipes neue Daten signalisiert wurden. Sollen mehrere Gruppen implementiert werden muss dies in der abgeleiteten Klasse gemacht werden.
		 *
		 * Beispiel fuer die Implementierung eines eigenen Handlers
		 * \code
		 * bool MyFilter::subscribe(BasePipe& pipe, int group)
		 * {
		 *	pipe.subscribe(Delegate<MyFilter, fliplib::PipeEventArgs>(this, &MyFilter::myProceedMethod));
		 * 	return true;
		 * }
		 * \endcode
		 *
		 * \param [in] pipe Ausgabepipe eines anderen Filters
		 * \param [in] group Gruppennummer. Die BaseFilterklasse unterstuetzt 0 + 1. 0=keine Gruppe, 1=Gruppe 1
		 * \return true = Registruerung erwuenscht. false = Registrierung verweigern
		 */
		virtual bool subscribe(BasePipe& pipe, int group);

		/**
		 * Loescht das Abo einer bestimmten Pipe.
		 *
		 * \param [in] pipe Pipe Ausgabepipe eines anderen Filters
		 * \return true = Pipe gefunden und geloescht
		 */
		virtual bool unsubscribe(BasePipe& pipe);

		/**
		 * Standardprozedure fuer die Eventverarbeitung; subscribe und unsubscribe nicht von der Subklasse
		 * ueberladen sind, muss mindestens proceed ueberladen werden. Diese Methode wirft eine NotImplementedException
		 */
		virtual void proceed(const void* sender, PipeEventArgs& e);

		/**
		 * Standardprozedure fuer die Eventverarbeitung; subscribe und unsubscribe nicht von der Subklasse
		 * ueberladen sind, muss mindestens proceed ueberladen werden. Diese Methode wirft eine NotImplementedException
		 */
		virtual void proceedGroup(const void* sender, PipeGroupEventArgs& e);

		/**
		 * Wraps the proceed call to measure its runtime.
		 */
		void timedProceed(const void* p_pSender, PipeEventArgs& p_rEventArgs);

		/**
		 * Wraps the proceedGroup call to measure its runtime.
		 */
		void timedProceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rGroupEventArgs);

		PipeInfo* getPipeInfo(const std::string &pipeName) const;

		/**
		 * Liefert die Anzahl Pipes.
		 *
		 * \param [in] input	True=nur Eingangspipe werden gezaehlt. False=nur Ausgangspipes werden gezaehlt
		 */
		int pipeCount(bool input) const;

        /**
         * Allows the concrete implementation to specify the ingoing PipeConnectors.
         * Should be called from the constructor
         **/
        void setInPipeConnectors(const std::initializer_list<PipeConnector> &list)
        {
            m_inPipeConnectors = list;
        }

        /**
         * Allows the concrete implementation to specify the outpoing PipeConnectors.
         * Should be called from the constructor.
         **/
        void setOutPipeConnectors(const std::initializer_list<PipeConnector> &list)
        {
            m_outPipeConnectors = list;
        }

        /**
         * Allows to get the attributs of the filter from the attributes.json file.
         **/
        void setVariantID(const Poco::UUID &variantID)
        {
            m_variantID.push_back(variantID);
        }

        void setVariantID(const std::initializer_list<Poco::UUID> &IDList)
        {
            m_variantID = IDList;
        }

	public:
		/**
		 * Liefert einen Zieger auf die Pipe in Abhaengigkeit des Namen (threadsafe)
		 *
		 * \param [in] name Name der Pipe die gefunden werden soll
		 * \return Zeiger auf die Pipe. Wenn keine passende Pipe gefunden wird, liefert diese Funktion NULL zurueck
		 */
		BasePipe* findPipe(const std::string &name) const;

		Iterator begin() const; //< Getter Begin Iterator fuer die foreach Abfrage (threadsafe)
		Iterator end() const;	//< Getter End Iterator fuer die foreach Abfrage (threadsafe)

		/**
		 * Der Filter soll mit einer Pipe eines anderen Filters verbunden werden. Die Pipe wird standardmaessig in einer Gruppe
		 * zusammengefasst
		 *
		 * \param [in] pipe Die zu verbindende Pipe
		 * \return Die Pipe wurde korrekt verbunden
		 */
		bool connectPipe(BasePipe* pipe);

		/**
		 * Der Filter soll mit einer Pipe eines anderen Filters verbunden werden. Wenn group>0 gesetzt wird, wird die Pipe i
		 * in einer Gruppe(Nr) zusammengefasst. Die Klasse BaseFilter unterstuetzt genau eine Gruppe. Sollen mehrere
		 * Gruppen unterstuetzt werden, muss dieses Verhalten in der Subklasse implementiert werden.
		 *
		 * \param [in] pipe Die zu verbindende Pipe
		 * \param [in] group Gruppennummer
		 * \return Die Pipe wurde korrekt verbunden
		 */
		bool connectPipe(BasePipe* pipe, int group);

		/**
		 * Der Filter soll die Verbindung zur Pipe wieder trennen.
		 *
		 * \param [in] pipe Pipe zu der die Verbindung getrennt werden soll
		 * \return Die Pipe wurde korrekt verbunden
		 */
		bool disconnectPipe(BasePipe* pipe);

		std::string name() const;	//< Getter Name des Filters
		Poco::UUID id() const; 		//< Getter Id des Filters
		void  setId(const Poco::UUID& id); 	//< Setter Id des Filters

		Poco::UUID filterID() const; //< Getter Filter ID
		std::string nameInGraph() const; ///<Get filter name with graph index, if available

		/**
		 * Liefert Liste aller Paramter von diesem Filter. Die Parameter werde vom GraphBuilder geladen
		 */
		ParameterContainer& getParameters();

		/**
		* Liefert ein XML ELement mit alle Filterstandardwerten
		*
		* \param[in]fqName fully qualified name. Wenn "" wird der interne Name des Filters verwendet
		*/
		std::string toXml() const { return toXml(""); }
		std::string toXml(const std::string& fqName) const;


		void setAttribute(const std::string&  key, const std::string& value);
		void setAttribute(const std::string& pipeName, const std::string&  key, const std::string& value);

		AttributeMap & getAttribute();
		AttributeMap & getAttribute(const std::string& pipeName);

        int readCounter() const {return m_oCounter;}
        int readProcessingIndex() const {return m_oProcessingIndex;}
		/**
		 * Setzt den synchronisierungs counter.
		 */
		void setCounter(int p_oCount);
		/**
		 * Setzt den timer counter zurueck.
		 */
		void resetTimerCnt();

		/**
		 * Setzt den signal-Zaehler fuer alle Eingangspipes zurueck. Sinnvoll bei Arm Nahtstart falls noch auf Signals gewartet wird.
		 */
		void resetSignalCntGroupEvent();

		/**
		 * Resets the signal counter for the given @p imageNumber.
		 *
		 * Useful in case not all samples were signaled in case a sensor doesn't provide data.
		 */
		void resetSignalCountGroupEvent(int imageNumber);

		/**
		  * only, when the filter counter is equal to the image number of the incoming data, proceed gets invoked
          * otherwise, the filter is still blocked by working on older data
          * NB: the CAS deliberately does not increment the counter, this is done in logTiming()
		  */
        void synchronizeOnImgNb(int p_oImgNb);

        /**
         * Logs the min, max and mean processing time on verbosity max.
         **/
        void logProcessingTime();

        /**
         * Ensures that the Filter's image number is at least @p imageNumber.
         **/
        void ensureImageNumber(int imageNumber);

        /**
         * Skips the processing of @p imageNumber. The synchronization is peroformed
         * and directly afterwards the preSignalAction gets invoked without any calculation
         * and without notifying any signals.
         **/
        void skipImageProcessing(int imageNumber);

         /* *
          * Sets an incremental index for the filter instance
          * */
        void setGraphIndex(int index);

        /**
         * Sets the option to log timing also when the verbosity is not maximal
         * */
        void alwaysEnableTiming(bool p_oValue);

        void resetProcessingIndex () {m_oProcessingIndex = -1;}
        static void resetProcessingCounter() { sProcessingCounter = 0;}

        /**
         * @returns the ingoing PipeConnectors.
         **/
        const std::vector<PipeConnector> &inPipeConnectors() const
        {
            return m_inPipeConnectors;
        }

        /**
         * @returns the outgoing PipeConnectors.
         **/
        const std::vector<PipeConnector> &outPipeConnectors() const
        {
            return m_outPipeConnectors;
        }

        /**
         * @returns the variantID.
         **/
        const Poco::UUID &variantID(unsigned int position) const
        {
            return m_variantID.at(position);
        }

        /**
         * @returns the size of the variantID vector
         **/
        unsigned int variantIDSize() const
        {
            return m_variantID.size();
        }

	private:
		/**
		 * stops timer and outputs mean time elapsed if max verbosity. needs to be called in proceed before signal(). start is called automatically.
		 */
        /*inline*/ void logTiming();
        void logPaintTime(const std::chrono::nanoseconds &elapsed);

		Poco::UUID filterID_;				// Feste ID des Filters. Identifiziert einen Filter eindeutig.
		Poco::UUID instanceID_;				// Eindeutige ID des Filters (InstanceID). Wird vom GraphBuilder vergeben
		std::string name_;					// Name des Filters
		PipeMap map_;						// Liste aller OutputPipes
		PipeGroupEvent groupevent_;			// Gruppe der InputPipes
		AttributeMap attributes_;			// Liste aller frei definierbaren Attribute fuer Reflection
		precitec::system::ElapsedTimer			m_oTimer;			///< proceed / proceedGroup timer, summed up over cycles. Reset on setParameter.
        std::chrono::nanoseconds m_overallProcessingTime;
        std::pair<int, std::chrono::nanoseconds> m_minProcessingTime;
        std::pair<int, std::chrono::nanoseconds> m_maxProcessingTime;
        std::chrono::nanoseconds m_paintTime;
        std::pair<int, std::chrono::nanoseconds> m_minPaintTime;
        std::pair<int, std::chrono::nanoseconds> m_maxPaintTime;
        int m_paintTimeCounter;
        Poco::FastMutex m_synchronizationMutex;
        Poco::Condition m_synchronization;
        Poco::ThreadLocal<bool> m_preSignalActionCalled;
        bool m_oAlwaysEnableTiming; // log timing also when verbosity != max
        int m_oGraphIndex; //(optional) unique index inside graph
        int m_skippedCounter;  //additional info for processing time
        int m_oProcessingIndex = -1; //(optional) unique index assigned according to the pipe notification
        static std::atomic<int> sProcessingCounter ;

        std::vector<PipeConnector> m_inPipeConnectors;
        std::vector<PipeConnector> m_outPipeConnectors;
        std::vector<Poco::UUID> m_variantID;
	protected:


		/*virtual*/ void setParameter();	// set base filter parameter that all filters have in common

        /**
		 * Triggers base-filter actions like logTiming() and increments m_oCounter, which unlocks the proceed routines.
         *
         * Has to be called by all filters BEFORE signaling next filter(s). Even, if there is no signal() call, as in seam-end-result.
         * Since this routine also handles synchronisation, following to its call, no member variables of a filter shall be accessed.
         * Otherwise, data corruption or thread hangup could occcur.
		 */
		void preSignalAction();

        int	                m_oTimerCnt;		///< nb cycles for performance measure. Reset on setParameter.
        int	                m_oCounter;			///< nb cycles. Reset on setParameter. Used for synchronizing with image number of data packets
		ParameterContainer 	parameters_;		// Container fuer FilterParameter
		VerbosityType		m_oVerbosity;		// every filter has a verbosity parameter

	};
} // namespace fliplib
#endif // !defined(EA_586073B0_669B_471c_8A98_A0BFF4D6478E__INCLUDED_)
