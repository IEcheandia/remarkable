#ifndef T3_CONTROL_H_
#define T3_CONTROL_H_
#pragma once

#include <iostream>
#include "system/types.h"

/**
 * Interfaces::t3_control.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, device, worker
 *  - 4 Schnittstellen:
 *  		Control: M_InformControl, M_ControlWorker
 *  		Device:  E_InformControl, E_SupplyWorker
 *  		Worker:	 E_InformControl
 *  - 1 SharedMem: Data_ rw-Device r-Worker
 *
 * T3Control: generische StateMachine
 *  - ist Subscriber fuer E-InformControl
 *  - ist Client fuer M-ControlWorker
 *  - ist Client fuer M-ControlDevice
 */


namespace precitec
{
namespace test
{
	class Controller {
	public:
		enum State {
			UnInitialized = -1,
			Initialized,
			ReadyForTask,
			InGeopardy
		};
		enum Error {
			NoError = 0,
			Meditating,
			Crashed,
			Exception
		};
		class Code {
		public:
			Code() {}
			Code(int seed) { fill(seed); setCheckSum(); }
		public:
			void  tell(std::ostream &os, int codeNum) const;
		private:
			void	 fill(int seed=1);
			int  calcCheckSum() const;
			void setCheckSum() { checkSum_ = calcCheckSum();  }
			bool testCheckSum() const { return checkSum_ == calcCheckSum(); }
			friend  std::ostream &operator <<(std::ostream &os, Code const& c) {
				os << "C:{" << c.checkSum_ << "} " ;
				for (int i=0; i<Code::CodeSize; ++i)
					os << "[" << c.code_[i] << "]";
				return os;
			}
		private:
			enum { CodeSize = 10 };
			int code_[CodeSize];
			int checkSum_;
		};
	};

 	inline int Controller::Code::calcCheckSum() const {
 		int cs = 0;
		for (int i=0; i< CodeSize; ++i) {
			cs += code_[i] + i; // i wird hinzugefuegt, damit bei genullten Klassen die Laenge geprueft wird
		}
 		return cs;
 	}

 	inline void Controller::Code::fill(int seed) {
		for (int i=0, j=max(1, seed); i< CodeSize; ++i) {
//			code_[i] = (j*=324555761);
			code_[i] = (j++);
		}
 	}

 	inline void Controller::Code::tell(std::ostream &os, int codeNum) const {
 		const int FehlerCode = 8;
		static PvString codes[FehlerCode+1] = {
			"42", "Hallo Welt", "Aii gugg!", "ommmmmmm ...", "????", ":-/", "xoxox", "Aii daa!", "Fehler"
		};
		os << codeNum << ": ";
		os << codes[(testCheckSum()==true) ? checkSum_&7 : FehlerCode];
 		os << std::endl;
 	}
} // namespace test
} // namespace precitec

#endif // T3_CONTROL_H_
