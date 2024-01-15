///////////////////////////////////////////////////////////
//  BasePipe.h
//  Implementation of the Class BasePipe
//  Created on:      30-Okt-2007 14:19:36
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_39DCF860_3604_4adc_9AB6_6A1D742D41AA__INCLUDED_)
#define EA_39DCF860_3604_4adc_9AB6_6A1D742D41AA__INCLUDED_

#include <string>
#include <typeinfo>

#include "Poco/Mutex.h"
#include "Poco/SharedPtr.h"
#include "Poco/BasicEvent.h"
#include "Poco/AbstractDelegate.h"

#include "fliplib/BaseFilterInterface.h"


namespace fliplib
{

	class PipeEventArgs;

	/**
	 * Abstrakte Basisklasse der Pipes (Interface)
	 *
	 * Die BasePipe ist die Grundlage aller Pipes. Sie wird zur Verwaltung der Pipes innerhalb der Filter verwendet.
	 */
	class FLIPLIB_API BasePipe
	{

	public:
		/**
		 *
		 */
		typedef Poco::AbstractDelegate<PipeEventArgs> NotificationHandler;

		/**
		 * Konstruktor mit Name der Pipe und Type. Type wird in der Pipe verwaltet und geloescht
		 *
		 * \param [in] parent	Zeiger auf Parent Filter
		 * \param [in] name		Eindeutiger Name der Pipe innerhalb eines Filters
		 */
		BasePipe(BaseFilterInterface* parent, const std::string& name);

		/**
		 * Konstruktor mit Name der Pipe und Type. Type wird in der Pipe verwaltet und geloescht
		 *
		 * \param [in] parent		Zeiger auf Parent Filter
		 * \param [in] name			Eindeutiger Name der Pipe innerhalb eines Filters
		 * \param [in] contentType	Type der Pipe. Bsp: ImageFrame
		 * \param [in] channel		Fuer GraphEditor. Reihenfolge der Pipe im Filter
		 */
		BasePipe(BaseFilterInterface* parent, const std::string& name, const std::string& contentType, int channel);

		virtual ~BasePipe();	// Destruktor

		/**
		 * Installiert den NotificationHander fuer die Pipe. Der Handler wird aufgerufen wenn der Absender neue Daten signalisiert.
		 *
		 * Beispiel, eigener Handler installieren:
		 * \code
		 * void MyFilter::MyHandlerMethod(const void* sender, fliplib::PipeEventArgs&  e)
		 * {
		 * }
		 *
		 * void MyFilter::InstallPipe(BasePipe& pipe)
		 * {
		 * 	pipe.install(Delegate<MyFilter, fliplib::PipeEventArgs>(this, &MyFilter::MyHandlerMethod));
		 * }
		 *
		 * pipe.signal();
		 * \endcode
		 *
		 * Die Methode MyHandlerMethod wird aufgerufen, wenn die Pipe signalisiert wird.
		 *
		 * \param [in] notificationHandler Referenz auf eine Funktion mit folgender Signatur void(const void* sender, PipeEventArgs& e).
		 */
		virtual void install(const NotificationHandler& notificationHandler) = 0;

		/**
		 * Deinstalliert den NotificationHandler fuer diese Pipe
		 *
		 * \param [in] notificationHandler Referenz auf eine Funktion mit folgender Signatur void(const void* sender, PipeEventArgs& e).
		 */
		virtual void uninstall(const NotificationHandler& notificationHandler) = 0;


		std::string name() const; 	//< Getter Liefert den Name der Pipe

		const std::type_info& type() const;	//< Getter Liefert den Type der Pipe fuer typeid()
		bool isPipeType(const std::type_info& type); // Vergleicht type mit der Type der Pipe
		unsigned int getHash(const std::type_info& type);
		unsigned int getHash();

		std::string tag() const; //< Getter Liefert Pipespezifische Parameter

		/**
		 * Setter fuer optionale Parameter
		 * \param[in]	beliebiger Text
		 */
		void setTag(const std::string& tag);

		/**
		 * Liefert true, wenn zwei Filter mit dieser Pipe verbunden sind
		 */
		 virtual bool linked() const = 0;

		/**
		 * Signalisiert dem Abonnenten, dass ein neues Datum in die Pipe geschrieben wurde. Diese Methode wird vom Pipe Besitzer
		 * aufgerufen.
		 */
		virtual void signal(int p_oImgNb) = 0;


		/**
		 * Sinalisiert dem Besitzer, dass neue Daten angeforder werden. Diese Methode wird vom Abonnenten aufgerufen
		 */
		virtual void requestData() = 0;
		
		
		/**
		* Deinstalliert alle NotificationHandler fuer diese Pipe
		*/
		virtual void uninstallAll() = 0;

		/***
		*
		* Gibt die Bildnummer zu den in der Pipe enthaltenen Daten aus.
		*/
		virtual int getImageNumber(int) const = 0;

	protected:
		virtual const std::type_info& getType() const = 0;	// Fragt den internen Type dieser Pipe ab

		BaseFilterInterface* parent_;		// Parent Filter

		std::string name_;			// Name der Pipe
		std::string tag_; 			// unabhaengige Pipeparameter
		unsigned int hash_;			// Hashcode vom Typename

	private:
		// hide constructor
		BasePipe();
		BasePipe(const BasePipe& pipe);
		// hide operator =
		BasePipe& operator = (const BasePipe& other);

		unsigned int getHash(const char* str, unsigned int len);

	};

}
#endif // !defined(EA_39DCF860_3604_4adc_9AB6_6A1D742D41AA__INCLUDED_)
