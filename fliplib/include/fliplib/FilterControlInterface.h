///////////////////////////////////////////////////////////
//  FilterControlInterface.h
//  Implementation of the Class FilterControlInterface
//  Created on:      24-Jan-2008 10:56:27
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_7EDBB2BD_AE56_4256_AD37_FC6E348930B4__INCLUDED_)
#define EA_7EDBB2BD_AE56_4256_AD37_FC6E348930B4__INCLUDED_

#include "fliplib/OverlayCanvas.h"
#include "fliplib/ArmState.h"
#include "common/defines.h"
#include <cassert>

namespace fliplib
{
	class ExternalData { 
	public:
		//ExternalData() {}
		virtual ~ExternalData()=0;
	}; // class ExternalData

	/*virtual*/ inline ExternalData::~ExternalData() {}

	/**
	 * Basic Steuer Control des Filters
	 * 
	 * Stellt Methoden zum steuern der Filter und deren
	 * Resourcen zur Verfuegung. 
	 *   
	 */	
	class FLIPLIB_API FilterControlInterface
	{
	
	public:
	
		FilterControlInterface():
			m_pCanvas		( nullptr ),
			m_pExternalData	( nullptr )
		{};
		virtual ~FilterControlInterface() {};
	
		/**
		 * veranlasst den Filter sich neu zu initalisieren
		 */
		virtual void init()	{}
		
		/**
		 * veranlasst den Filter die Parameter neu zu lesen
		 */
		virtual void setParameter() {}
		
		/**
		 * veranlasst den Filter seine Resourcen freizugeben
		 */		
		virtual void dispose() 	{}
		
		/**
		 * Canvas fuer Overlay setzen. Der Canvas wird nicht vom Filter verwaltet
		 */
		virtual void setCanvas(OverlayCanvasInterface* canvas) { m_pCanvas = canvas; 	}
		/**
		 * Pointer auf externe daten setzen. Visitor existiert.
		 */
		virtual void setExternalData(const ExternalData* p_pExternalData) { m_pExternalData = p_pExternalData; 	}
		
		/**
		 * Wird aufgerufen, wenn der Filter in den Canvas zeichnen soll. Der Canvas ist jetzt gueltig und muss
		 * nich mit hasCanvas geprueft werden.
		 */
		 virtual void paint() { }
				
		/**
		 * veranlasst den Filter Resourcen zu initalisieren
		 */		
		virtual void arm (const ArmStateBase& state) {}
		
	protected:
		// Folgende Methoden koennen verwendet werden, wenn die Paint Methode nicht verwendet werden soll
		// Bsp. wenn direkt in der Proceed gezeichnet wird.
		
		// True=Ein gueltiger Canvas ist vorhanden 
		bool hasCanvas() { return m_pCanvas != nullptr; }

		// const getter to base pointer. Asserts non-nullptr.
		const ExternalData*		externalData() const	{ assert(m_pExternalData != nullptr && "Nullptr: m_pExternalData"); return m_pExternalData; }
		// Cast Helper Canvas
		//template<class T>
  //      T& canvas() { return static_cast<T&>(*m_pCanvas); } 
        
        template<class T>
		T& canvas(int p_oCounter) { return static_cast<T*>(m_pCanvas)[p_oCounter % g_oNbPar]; }

		// const getter to derived T pointer. Asserts non-nullptr and T being a derived type of 'ExternalData'.
		template<typename T>
		const T* externalData() const { 
			poco_assert_dbg(dynamic_cast<const T*>(externalData()) != nullptr && "T is not a type derived of 'ExternalData'."); 
			return static_cast<const T*>(externalData()); 
		}
		
		
	private:
		OverlayCanvasInterface*				m_pCanvas;	
		const ExternalData*					m_pExternalData;	///< Pointer to external data base class. Ownership remains extern. Valid only if set by user using the setter visitor.
	};
	
}
#endif // !defined(EA_7EDBB2BD_AE56_4256_AD37_FC6E348930B4__INCLUDED_)
