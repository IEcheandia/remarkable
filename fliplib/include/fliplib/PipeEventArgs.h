///////////////////////////////////////////////////////////
//  PipeEventArgs.h
//  Implementation of the Class PipeEventArgs
//  Created on:      30-Okt-2007 14:29:47
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_CB421F9D_81A0_4d66_ADB3_A12E7EBA67DC__INCLUDED_)
#define EA_CB421F9D_81A0_4d66_ADB3_A12E7EBA67DC__INCLUDED_

#include <string>
#include "Poco/Foundation.h"
#include "Poco/EventArgs.h"
#include "fliplib/Fliplib.h"
#include "fliplib/BasePipe.h"


namespace fliplib
{
	/// Vorwaerstdeklaration der Pipe um recursives Include  (Value()) zu vermeiden
	template <class T> class SynchronePipe;

	/**
	 * Event Argument fuer die Message innerhalb einer Pipe. Dieses EventArgument besitzt eine Referenz
	 * auf die Absenderpipe. Dadurch kann der Empfaenger auf die Senderpipe direkt zugreifen. Event Args
	 * ist NICHT Besitzer von Pipe.
	 */
	class FLIPLIB_API PipeEventArgs : public Poco::EventArgs
	{

	public:
		PipeEventArgs(BasePipe* pipe, int p_oImgNb);
		// Konstruktor mit Referenz auf Absenderpipe

		virtual ~PipeEventArgs();
		// Destruktor

		BasePipe* pipe() const;

		/// Liefert eine Referenz auf die Absenderpipe.
		//template <class ArgType>
		//operator ArgType() { return static_cast<SynchronePipe<ArgType>* >(pipe())->read(m_oCounter); }

	private:
		//template <> fliplib::SynchronePipe<int> a;


		//template <class ArgType>
		//ArgType value() {
		//	return (static_cast<  SynchronePipe<ArgType> * >(pipe()))->read(m_oCounter);
		//}

		PipeEventArgs();
		// Hide Standardkonstruktor

		BasePipe *pipe_;

    public:
        int m_oImgNb;   // image number of date packet. used for synchronization
	};


	class PipeGroupEvent;

	/**
	 * Fasst mehrere PipeEvents zusammen
	 */
	class FLIPLIB_API PipeGroupEventArgs : public Poco::EventArgs
	{

	public:
		PipeGroupEventArgs(int count, PipeGroupEvent* parent, int p_oImgNb);
		// Konstruktor mit Referenz auf Absenderpipe

		virtual ~PipeGroupEventArgs();
		// Destruktor

		bool isValid() const;
		// true, wenn in der Pipe ein "gueltiges" Datum steht. Wenn das Signal von einer Gruppe versendet
		// wurde, dann wird immer true zurueckgegeben.

		BasePipe* pipe(const std::string& name);
		// Liefert eine Referenz auf die Absenderpipe.

		BasePipe* pipe(const std::type_info& type);

		PipeGroupEvent* group() const;

		int count() const;

	private:
		PipeGroupEventArgs();
		// Hide Standardkonstruktor

		int count_;
		PipeGroupEvent* parent_;

    public:
        int m_oImgNb;   // image number of date packet. used for synchronization
	};

}

#endif // !defined(EA_CB421F9D_81A0_4d66_ADB2_A12E7EBA67DC__INCLUDED_)
