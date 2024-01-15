#ifndef CAM_PORT_H
#define CAM_PORT_H

/**
 * ip::CamPort ist ein Wrapper für Camera-Handles
 * ip::pf::Port ist die konkrete Implementierung für Photon-Focus
 * Kameras und hält eine Handle, die automatisch gesachlossen wird
 */


#	include <iostream>
#	include <bitset>
#	include <string>

#	include "system/types.h"
#	include "system/typeTraits.h"
#	include "pf/photonFocus.h"

namespace precitec
{

class BaseCamera;

namespace ip
{

/**
 * Handle-Klasse fuer Ports
 * Um portabel zu bleiben wird definiert dass das
 * interne Handle auf einen int passen muss.
 * So sol erreicht werden, dass die Portschnittstelle
 * gleichzeitig mit Kameras untershiedlicher Hersteller
 * Benutzt werden kann
 */
class CamPort
{
 public:
	 CamPort() {

		 //std::cout << "CamPort(): " << std::endl;
	 } /// neg. -> ungueltig + Zahl groesser als erlaubt
	 virtual ~CamPort() {
		 //std::cout << "~CamPort: " << std::endl;
	 }
 public:
	 /// da PfPort nie wirft, muss Gueltigkeit pruefbar sein
	 bool isValid() const {
		 //std::cout << "CamPort::isValid: " << std::endl;
		 return isHandleValid(); }
	 /// ist Port verfuegbar
	 bool isFree() const { return isPortFree(); }
	 /// oeffnet exisitierenden Port
	 int open() {
		 //std::cout << "CamPort::open: " << std::endl;
		 return reopen(); }
	 /// oeffnet einen existierenden Port auf portNum
	 int open(uInt portNum) {
		 //std::cout << "CamPort::open( " << portNum << ")"<< std::endl;
		 return portNum<LimPorts__ ? openByNumber(portNum) : NoPort__; }
	 /// schliesst einen existierenden Port
	 void close() {
		 //std::cout << "CamPort::close: " << std::endl;
		 closePort(); }
	 /// die Kamera muss auf das Handle zugreifen koennen
	 /// die Portnummer     ??? Kruecke, muss weg
	 virtual int handle() const { return int(isValid() ? portNum() : NoPort__); }
 private:
	 /// die PortNummer, hier ungueltig
	 virtual uInt portNum() const { return LimPorts__; }
	 /// da PfPort nie wirft, muss Gueltigkeit pruefbar sein
	 virtual bool isHandleValid() const {
		 //std::cout << "CamPort::isHandleValid: " << std::endl;
		 return false; }
	 /// ist Port verfuegbar
	 virtual bool isPortFree() const { return false; }
	 /// oeffnet ungueltigen/geschlossenen Port ; liefert hier ungueltige Handl
	 virtual int reopen() {
		 //std::cout << "CamPort::reopen: " << std::endl;
		 return NoPort__; }
	 /// oeffnet einen Port auf portNum
	 virtual int openByNumber(uInt portNum) { return NoPort__; }
	 /// schliesst einen existierenden Port
	 virtual void closePort() { }
 public:
	 /// max. Anzahl der logischen Ports ! >= Anzahl der phys. Ports
	 static const uInt LimPorts__ = 32;
	 /// Handle fuer nicht existierenden Port, liefert ungueltige PortNum
	 static const uInt NoPort__   = ~LimPorts__;
	 friend std::ostream &operator << (std::ostream &os, CamPort const &p);
 private:
	 /// Ports sind einzig
	 //CamPort(CamPort&) {}
	 //CamPort operator = (CamPort&) { return *this; }
}; // CamPort

namespace pf
{

class Port : public ip::CamPort
{
 public:
	 /// default CTor erzeugt ungueltigen port (fuer Arrays)
	 Port() : handle_(NoPort__) {
		 //std::cout << "Port: " << std::endl;
	 }
	 /// oeffnet Port; Scheitern -> port.isValid==false
	 Port(uInt portNum);
	 /// port kopieren, wird sofort geoeffnet
	 Port(Port const& port) : handle_(openPort(port.portNum())) {
		  //std::cout << "Port::Port(Port): " << handle_ << std::endl;
		 //std::cout << "this Port: " << *this << std::endl;
	 }
	 virtual ~Port();
	 /// gibt Infos ueber einen Port
	 static bool info(uInt portNum, std::string &manuName,
													std::string &portName, int &version,
													std::string &portType, int /*verbosity*/);
	 static uInt numPorts();
friend std::ostream &operator << (std::ostream &os, Port const &p);
//		 friend BaseCamera;
 private:
	 /// da PfPort nie wirft, muss Gueltigkeit pruefbar sein
	 virtual bool isHandleValid() const {
		 return handle_ >= 0 && portNum()<LimPorts__;
	 }
	 /// ist Port verfuegbar
	 virtual bool isPortFree() const { return isValid() && !portUsed__.test(portNum()); }
	 /// oeffnet einen existierenden Port auf portNum
	 virtual int reopen() {
	 //std::cout << "Port::reopen: " << std::endl;
		 return handle_ = openPort(portNum()); }
	 /// oeffnet einen existierenden Port auf portNum
	 virtual int openByNumber(uInt portNum) {
		 return handle_ = openPort(portNum);
	 }
	 /// schliesst einen existierenden Port
	 virtual void closePort() { handle_ = closePort(portNum()); }
	 /// die Portnummer
	 virtual uInt portNum() const {
		 return uInt(handle_<0 ? ~handle_ : handle_); }
 private:
	 enum PortType { Serial, Custom, USB, RS232, NumTypes };
 private:
	 //Port(Port&) {}
	 //Port operator = (Port&) { return *this; }
 private:
	 static bool   validateSystem();
	 /// gibt (ggf. ungueltige) handle zurueck
	 static int    openPort(uInt handle);
	 /// schliesst port
	 static int    closePort(uInt handle);
	 /// portType to String
	 static std::string const& portTypeName(PortType portType);
 private:
	 /// soll garantierte, einmalige Initialisierung sicherstellen
	 static bool   portSystemValid__;
	 /// Anzahl der Physikalisch vorhandenen Ports
	 static uInt   numPorts__;
	 /// gibt an welche Ports belegt sind
	 static std::bitset<LimPorts__> portUsed__;
 private:
	 uInt handle_;
}; // Port
std::ostream &operator << (std::ostream &os, Port const &p);
} // pf
} // ip
} // precitec


#endif  // CAM_PORT_H
