#include <string>
#include <iostream>
#include "pf/photonFocus.h"
#include "camPort.h"



namespace precitec
{
namespace ip
{
	
	std::ostream &operator << (std::ostream &os, CamPort const &p)
	{
		if (p.isValid())
		{
			os << p.portNum() << "(" << (p.isFree() ? "free" : "used") << ")";
		} 
		else
		{
			os << "no port @" << p.portNum();
		}
		return os;
	}
	
	
	namespace pf
	{
		
		
		using std::bitset;
		using std::cout;
		using std::endl;
		
		bool  Port::portSystemValid__ = false;
		uInt  Port::numPorts__        = 0; 
		bitset<Port::LimPorts__> Port::portUsed__;  // default-CTor -> alle Bits NULL
		
		/**
		 * idempotente Funktion (tut nur beim ersten Mal was)
		 * initialisiert Ports-System, muss sicher vor jedem
		 * anderen Zugriff auf Port ausgefuehrt werden
		 * Initialisiert weiterhin die usedPorts-Variable
		 * gibt zurueck, ob System ok 
		 */
		bool Port::validateSystem()
		{
			//cout << "static Port::validateSystem" << std::endl;
			if (!portSystemValid__)
			{
				// erster Zugriff auf PortSystem
				int nPorts; // noetig wg int uInt Mismatch
				portSystemValid__ = (pfPortInit(&nPorts) == 0);
				//cout << "portSystemValid__: " << portSystemValid__ << std::endl;
				numPorts__ = nPorts;
				//cout << "numPorts__: " << numPorts__ << std::endl;
				// portUsed__.reset(); // loescht nochmals zur Sicherheit alles ???
																 }
			else 
			{
				//cout << "found to be in valid state" << std::endl;
			}
			return portSystemValid__;
		} // validateSystem
		
		/**
		 * idempotente Funktion (tut nur beim ersten Mal was)
		 * initialisiert Ports-System, muss sicher vor jedem
		 * anderen Zugriff auf Port ausgefuehrt werden
		 * Initialisiert weiterhin die usedPorts-Variable
		 * gibt zurueck, ob System ok 
		 */
		uInt Port::numPorts()
		{
			//cout << "static Port::validateSystem" << std::endl;
			validateSystem();
			return numPorts__;
		} // validateSystem
		
		/**
		 * gewissermassen die Fabrik fuer Ports
		 */
		int Port::openPort(uInt portNum)
		{
			//cout << "openPort: " << portNum << std::endl;
			if (   validateSystem()
					&& (portNum < numPorts__) )
			{
				if ( !portUsed__.test(portNum) )
				{
					
					//cout << "trying to open port : " << portNum<< std::endl;
					int PortNum = portNum;
					int error = pfDeviceOpen(PortNum); 
					if (error>=0)
					{
						//cout << "no/good error: " << error << std::endl;
						portUsed__.set(portNum);
						//cout << "used[" << portNum << "]: " << portUsed__.test(portNum) << std::endl;
						return portNum;
					} else
					{ // der ganze else-Zweig ist nur wg debug
						cout << "Pf openPort bad error: " << -error << " : " << pfGetErrorString(error)<< std::endl;
						return NoPort__;
					}
				}
				else
				{
					// port schon offen
					return portNum;
				}
			}
			else
			{
					//cout << "bad error: " << error << std::endl;
			}
			cout << "openPort failed: " << portNum << std::endl;
			return NoPort__;
		}
		/**
		 * close wird in DTor benutzt, scheitert daher nie
		 * im Zweifel tut es nichts, normalerweise
		 * wird portUsed__ fuer den Port zurueckgesetzt
		 * gibt _immer_ eine ungueltige handle zurueck
		 */
		int Port::closePort(uInt portNum)
		{
			//cout << "Port::closePort: " << std::endl;
			if (   validateSystem()
					&& (portNum<numPorts__)
					&& portUsed__.test(portNum) )
			{
				//		cout << "closing portNum: " << portNum << std::endl;
				//		
				int PortNum = portNum;
				/*int error =*/ pfDeviceClose(PortNum);
				//		cout << "error: " << error << std::endl;
				portUsed__.reset(portNum);
			}
			return NoPort__; // invalid handle
		}
		
		/**
		 * erzeugt immer einen Port
		 * wenn Oeffnen scheitert ist Port ungueltig
		 * -> handle_.isValid() ergibt dann false
		 */
		Port::Port(uInt portNum) 
		 : handle_(openPort(portNum))
		 {
			//std::cout << "Port(portNum) " << std::endl;
		 }
		
		/**
		 * schliesst port, scheitert nie
		 */
		Port::~Port() 
		{
			//std::cout << "~Port: " << std::endl;
			close();
		}
		
		/** 
		 * Funktion, um mehr ueber einen Port zu erfahren
		 */
		bool Port::info(uInt portNum,
										std::string &manuName, 
										std::string &portName, 
										int      &version,	
										std::string &portType,
										int /*verbosity*/)
		{
			int portNumber = portNum;
			int numPorts;
			bool valid; // wir koennen (static) nicht auf valid_ zugreifen
			valid = pfPortInit(&numPorts) >= 0;
			valid &= numPorts > portNumber;
			char manuBuffer[pf::MaxStringLen];
			char portBuffer[pf::MaxStringLen];
			int  manuBuffLen=0;
			int  portBuffLen=0;
			int  versionNum=0;
			int  type=0;
			if (valid) 
			{
				int error = pfPortInfo(portNumber, manuBuffer, &manuBuffLen,
															 portBuffer, &portBuffLen, &versionNum, &type);
				valid    =  (error >= 0);
				manuName = manuBuffer;		
				portName = portBuffer;
				portType = portTypeName(PortType(type));
				version  = versionNum;
			}
			return valid;
		}
		
		std::string const& Port::portTypeName(PortType portType)
		{
			static std::string const names[NumTypes+1]	=	
			{ "Serial", "Custom", "USB", "RS232", "<unknown>" };
			if (portType>NumTypes) portType=NumTypes;
			return names[portType];
			
		}

		std::ostream &operator << (std::ostream &os, Port const &p)
	{
		if (p.isValid())
		{
			os << p.portNum() << "(" << (p.isFree() ? "free" : "used") << ")";
		} 
		else
		{
			os << "no port @" << p.portNum();
		}
		return os;
	}

	} // pf
} // ip::pf
} // precitec
