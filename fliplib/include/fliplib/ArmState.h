///////////////////////////////////////////////////////////
//  ArmState.h
//  Implementation of the Class ArmState
//  Created on:      24-Jan-2008 11:32:29
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_E1A4F2A8_241C_498f_A749_66EE4FECB79A__INCLUDED_)
#define EA_E1A4F2A8_241C_498f_A749_66EE4FECB79A__INCLUDED_

#include "fliplib/Fliplib.h"

namespace fliplib
{

	/**
	 * Diese Klasse dient als Basisklasse fuer das Armen der Filter. siehe AbstractFilterVisitor und
	 * FilterControlInterface
	 */
	class FLIPLIB_API ArmStateBase
	{
	
	public:
		/**
		 * cto Initalisiert die Instanz der Klasse ArmState.
		 */
		ArmStateBase() : id_(0) {}
		
		/**
		 * cto Initalisiert die Instanz der Klasse ArmState
		 */
		ArmStateBase(int id) : id_(id) {}
		
		/**
		 * dcto
		 */
		//virtual ~ArmStateBase() {};
		
		/*virtual*/ int getStateID() const { return id_; }  //< liefert die StateID
		
		//const std::type_info& type() const 		 
		//{
		//	return getType();
		//}	

	//protected:		
		//	/*virtual*/ const std::type_info& getType() const
		//	{
		//		return typeid(*this);
		//	}
		
	private :
		int id_;
					
	};
	
	
	/**
	 * Vorlage fuer ArmState 
	 * HS: bei gebrauch basisklassse wieder virtuell machen
	 */
//	template < int ID>
//	class FLIPLIB_API ArmState : public ArmStateBase
//	{
//	public:
//		ArmState() : ArmStateBase(ID) {}
//		
//	protected:	
//		virtual const std::type_info& getType() const
//		{
//			return typeid(*this);
//		}
//	};
//	
//	// Einfacher Status nur mit Leve
//	template <>
//	class FLIPLIB_API ArmState< -1 > {};
//	
//	typedef ArmState< -1 > EmptyState;


}

#endif // !defined(EA_E1A4F2A8_241C_498f_A749_66EE4FECB79A__INCLUDED_)
