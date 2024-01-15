/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR
 *  @date			2009
 *  @brief			Defines class TriggerInterval, which represents an interval of equidistant triggersignals.
 */


#ifndef TRIGGERINTERVAL_H_
#define TRIGGERINTERVAL_H_

#include "common/product.h"
#include "workflow/stateMachine/abstractState.h"

namespace precitec
{
namespace interface
{
		

	// Diese Klasse beschreibt einen Bereich in dem Triggersignale bei einer bestimmte Geschwindigkeit ueber eine bestimmte Laenge 
	// erzeugt werden sollen. Typischerweise entspricht ein Triggerinterval einem Nahtbereich
	class TriggerInterval
	{
		public:
			enum { uSec=1, mSec=1000*uSec, Sec=1000*mSec };
		
		public: 
			TriggerInterval() :
				m_oTriggerDistance	(0), 
				m_oNbTriggers		(0), 
				m_oTriggerDelta		(0), 
				m_oState			(workflow::eOperate) 
				{} 
			TriggerInterval(unsigned int p_oTriggerDistance, unsigned int p_oNbTriggers) :
				m_oTriggerDistance(p_oTriggerDistance),
				m_oNbTriggers(p_oNbTriggers),
				m_oTriggerDelta(0),
				m_oState(workflow::eOperate)
				{}
			TriggerInterval(unsigned int p_oTriggerDistance, unsigned int p_oNbTriggers, unsigned int p_oTriggerDelta, workflow::State p_oState) :
				m_oTriggerDistance(p_oTriggerDistance),
				m_oNbTriggers(p_oNbTriggers),
				m_oTriggerDelta(p_oTriggerDelta),
				m_oState(p_oState)
				{}
		 
			inline unsigned int triggerDistance()	const { return m_oTriggerDistance; }
			inline unsigned int nbTriggers()		const { return m_oNbTriggers; }
			inline unsigned int triggerDelta()		const { return m_oTriggerDelta; }
			inline workflow::State state()			const { return m_oState; }
									
		private:			
			unsigned int m_oTriggerDistance;	///<- Triggerabstand in Nanosekunden
			unsigned int m_oNbTriggers;         ///<- Anzahl Trigger pro Interval
			unsigned int m_oTriggerDelta;       ///<- Triggerabstand in Micrometern
			workflow::State m_oState;
	};
	
	typedef std::vector<TriggerInterval> TriggerIntervalList;
}
}
	
#endif /*TRIGGERINTERVAL_H_*/
