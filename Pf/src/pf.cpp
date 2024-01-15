#include <cstdlib>
#include <iostream>
#include <ctype.h>  // toupper(char)
#include <cstdio>  // getchar()

#include <istream>
#include <unistd.h>
#include <termios.h>
#include <limits>

//#include "Poco/Timestamp.h"
//#include "system/types.h"
typedef unsigned char byte;
typedef unsigned int	uInt;



//#include "system/typeTraits.h"
#include "pf/photonFocus.h"
#include "camPort.h"
#include "pfCamera.h"
#include "camProperty.h"
//#include "geo/range.h"
#include "pf.h"

namespace precitec
{
	using std::cout;
	using std::endl;
	//using geo2d::Range;
	/**
	 * kurze Überisch ausgeben
	 */

	template <> const uInt		No<uInt>::Value		= std::numeric_limits<uInt>::max();
    template <> const int       No<int>::Value      = std::numeric_limits<int>::max();
	template <> const String	No<String>::Value	= String("No Value");

	void help()
	{
			cout << "pf <Siso0|Siso1> Set|Get|List|Exec <Property>" << endl;
	}


	/**
	 * configure Term configureirt das Terminal so um,
	 * dass Tastatureingaben einzeln ohne CR/LF eingelesen
	 * werden können
	 * der Parameter set ist true zum Setzen, false zum zuruecksetzen
	 */
	void configureTerm(bool set)
	{
	  struct termios termiosVar;

	  // set the terminal (stdin) to non canonical mode
	  if (tcgetattr(STDIN_FILENO, &termiosVar) == -1)
	  {
	      cout << "cannot access terminal attibutes" << endl;
	  }
	  if (set)
	  {
		  termiosVar.c_lflag &= ~(ICANON);
	  }
	  else
	  {
		  termiosVar.c_lflag |= ICANON;
	  }
	  if (tcsetattr(STDIN_FILENO, TCSADRAIN, &termiosVar) == -1)
	  {
	      cout << "cannot modify terminal attibutes" << endl;
	  }
	  tcflush(STDIN_FILENO, TCIFLUSH);
	}

	/**
	 * kurze aus EingabeString port ermitteln
	 * Format sisoX/comX X=PortNum <nix>
	 * \returns 0..6 No::Value für port nicht angegeben
	 * -1 Wert wird gebraucht, um festzustellen ob der Parameter verbraucht wurde
	 */
	uInt getPort(String s)
	{
		// zum Suchen auf Klenibuchstaben konvertieren
		for (uInt i=0; i!=s.length(); ++i) { s[i]=tolower(s[i]); }

		// ist es explizit ein Siso-Port (phys: 0..1) ?
		//uInt found = s.find(String("siso"));
		std::size_t found = s.find(String("siso"));
		if (found!=No<uInt>::Value)
		//if (found!= -1)
		{
			int sisoPort = s[found+4]-'0';
			//if (Range(0, 2).contains(sisoPort)) return sisoPort;
			if ((sisoPort>=0) && (sisoPort<2)) return sisoPort;
			else return No<uInt>::Value;
		}

		// ist es explizit ein Com-Port (phys: 2..5) ?
		found = s.find(String("com"));
		if (found!=No<uInt>::Value)
		{
			int comPort = s[found+3]-'0';
			//if (Range(0, 4).contains(comPort)) return 2+comPort;
			if ((comPort>=0) && (comPort<4)) return 2+comPort;
			else return No<uInt>::Value;
		}

		// wurde Port explizit (ohne siso oder com gesetzt)
		found = s.find_first_of("012345");
		if (found!=No<uInt>::Value) return s[found]-'0'; // port explizit gefunden

		return No<uInt>::Value; // nix gefunden
	}

	/**
	 * kurze aus EingabeString port ermitteln
	 * Format: list, get, set, <nix>
	 * \returns PfCommand (enum) Wert
	 */
	PfCommand getCommand(String s)
	{
		String 	commands[] = {"list", "get", "set", "exec"};
		for (uInt i=0; i!=s.length(); ++i) { s[i]=tolower(s[i]); }
	//	int			numCommands = sizeof(commands)/sizeof(String);

		PfCommand command = Nothing;
		for ( command=List; command<Nothing; command=PfCommand(command+1))
		{
			uInt found = s.find(commands[command]);
			if (found!=No<uInt>::Value) break;
		}
		return command;
	}


	/**
	 * einzelne Zeichen von konsole auslesen
	 */
	char getAnswer()
	{
		while (true)
		{
			int keyBuffer[2];

	 		int fileId = STDIN_FILENO;
			read(fileId, &keyBuffer[0], size_t(1));

			read(fileId, &keyBuffer[0], 1);
			keyBuffer[0] &= 0xFF; // 0xff ??? escape mask
			if (keyBuffer[0] == 0xff)
			{
				read(STDIN_FILENO, &keyBuffer[1], 1);
				return (keyBuffer[1] &= 0xFF);
			}
			else
			{
				return keyBuffer[0];
			}
		}
	}


	/**
	 * einen String mit (returrn als Abschluß)
	 * von Konsole auslesen
	 */
	void  getAnswerString(String &answer)
	{
		configureTerm(false);
		cout << "Enter: ";
		std::cin >> answer;
		cout << endl;
		configureTerm(true);
	}


	/**
	 * Stream-Variante von getAnswer
	 * ist noch zu testen
	 */
	char getPrimitivAnswer()
	{
		// Puffer leeren
		//while (std::cin.peek());
		char answer;
		std::cin.read(&answer, 1);
		cout << char(answer) << std::endl;
		return answer;
	}

	/**
	 * zieht Port, Commando + Parameter aus Kommandozeile
	 */
	bool analyzeCommandLine(int argc, char *argv[], int &portNum,
													PfCommand &command, String &parameter, String &value)
	{
		if (argc<2) { help(); return false; }

		// default-Werte setzen
		command 	= Nothing;
		parameter	= "";
		value			= "";

		const int DefaultPort = 2; // = Com0
		int  currArg = 1;
		portNum = getPort(argv[currArg++]);
		if (portNum==-1) { --currArg; portNum = DefaultPort; }
		cout << " Port set to " << portNum << endl;

		if (currArg>=argc) return true;
		command = getCommand(argv[currArg]);
		if (command!=Nothing) ++currArg;

		if (currArg>=argc) return true;
		parameter = argv[currArg++];

		if (currArg>=argc) return true;
		value 		= argv[currArg];
		return true;
	}

	/**
	 * die Gesamtliste der eigenschaften wird in drei
	 * Unterlisten mit primitiven Eigenschaften, Kommandos
	 * und Strukturen aufgeeilt
	 */
	void	generatePropertyLists(ip::Camera &camera, PropList & propList,
															PropArray &primList, 		uInt& primitives,
															PropArray &structList, 	uInt& structures,
															PropArray &commList, 		uInt& commands)
	{
		// nun auf drei Listen aufteilen: Kommandos, Strukturen und primitive Eigenschaften
		for (PropIter prop=propList.begin(); prop!=propList.end(); ++prop)
		{
			ip::Property property(camera, *prop);
			switch (property.type())
			{
			default:
			case 	PF_INVALID:
			case	PF_ROOT:
			case	PF_EVENT:
			case	PF_BUFFER:
			case	PF_ARRAY: // nix tun
			break;
			case	PF_MODE:
			case	PF_INT:
			case	PF_FLOAT:
			case	PF_BOOL:
			case	PF_REGISTER:
			case	PF_STRING:
				primList[primitives++] = *prop;
				break;
			case	PF_STRUCT:
				structList[structures++] = *prop;
				break;
			case	PF_COMMAND:
				commList[commands++]	 = *prop;
				break;
			}
		}
	}

	/**
	 * die Listen werden nacheinander am terminal ausgegeben
	 * die Strukturen immer Primitive und kommandos alternativ
	 */
	void  outputLists(ip::Camera &camera, ip::Property &sProperty, bool needPrimitives,
										PropArray const& primList, 		uInt primitives,
										PropArray const& structList, 	uInt structures,
										PropArray const& commList, 		uInt commands)
	{
		cout << "\t\t0" << "  " << "<raus>"  << endl;
		if ( (sProperty.isWritable())
			&& (sProperty.type()!=PF_ROOT)
			&& (sProperty.type()!=PF_STRUCT))
		{
			if (sProperty.type()==PF_COMMAND)
			{
				cout << "\t\t1" << "  " << "<ausfuehren>"  << endl;
			}
			else
			{
				cout << "\t\t1" << "  " << "<schreiben>"  << endl;
			}
			cout << endl;
		}

		char ch;
		if (needPrimitives)
		{
			ch = 'a';
			for (uInt i=0; i<primitives; ++i)
			{
				ip::Property	property(camera, primList[i]);
				if (property.isActive())
				{
					bool hasChild = camera.getChildProperty(primList[i]) != ip::pf::NoHandle;
					cout << "\t\t";
					cout << ch++ << (hasChild ? " +" : "  ") << property << endl;
				}
			}
		}
		else
		{
			ch = 'a';
			for (uInt i=0; i<commands; ++i)
			{
				ip::Property property(camera, commList[i]);
				if (property.isActive())
				{
					cout << "\t\t";
					cout << ch++ << "  " << property << endl;
				}
			}
		}

		ch = 'A';
		for (uInt i=0; i<structures; ++i)
		{
			ip::Property property(camera, structList[i]);
			if (property.isActive())
			{
				cout << "\t\t";
				cout << ch++ << " +" << property << endl;
			}
		}
	}

	/**
	 * je nach Befehl wird die entsprechende Eigenschaaft angewählt, gesetzt
	 * oder das Kommando ausgefürt
	 */
	ip::pf::Handle executeCommand(ip::Camera &camera, ip::Property &sProperty,
																bool needPrimitives, 					char ch,
																PropArray const& primList, 		uInt primitives,
																PropArray const& structList, 	uInt structures,
																PropArray const& commList, 		uInt commands)
	{
		// wir geben zurück welche Art von eingeschaft wir gewählt haben
		// Antwort auswerten
		if ( (sProperty.type()==PF_COMMAND) && (ch=='1'))
		{ // Kommando ausführen
			sProperty = String("1");
			return ip::pf::NoHandle; // eine Stufe zurück
		}
		else if (sProperty.isWritable() && (ch=='1'))
		{	// Wert einlesen und zuweisen
			String value;
			getAnswerString(value);
			//sProperty.write(&camera, value);
			std::cout << "writing: " << sProperty << " with " << value << std::endl;
			sProperty = String(value);
			return ip::pf::NoHandle; // eine Stufe zurück
		}
		else if ((ch>='A')&&(uInt(ch)<'A'+structures))
		{ // Struktur-Eintrag auswählen
			//cout << "structure selected: " << ch << endl;
			return structList[ch-'A'];
		}
		else
		{
			if (needPrimitives)
			{
				if ((ch>='a')&&(uInt(ch)<'a'+primitives))
				{ // primitiven eintrag auswählen
					//cout << "primitive selected: " << ch << endl;
					return primList[ch-'a'];
				}
			}
			else
			{
				if ((ch>='a')&&(uInt(ch)<'a'+commands))
				{ // Kommando auswählen
					//cout << "command selected: " << ch << endl;
					return commList[ch-'a'];
				}
			}
		}

		cout << "keine sinnvolle Selektion: " << ch << endl;
		return ip::pf::NoHandle;
	}

	/**
	 * lies einzelne Ebene des Eigenschafts-Baums einer Kamera aus
	 * es gibt 3 Filter um nu Einzelwerte, Subbäume und Kommandos auszulesen
	 * hie wird eine Eigenschaft per Konsolen-Einabe ausgewählt
	 */
	ip::pf::Handle  selectProperty(ip::Camera &camera, ip::pf::Handle startProperty,
																	 bool &needPrimitives, bool &needStructures)
	{

		ip::Property	sProperty(camera, startProperty);
		//cout << endl << sProperty << "(" << sProperty.type()  << ")" << "::" << PF_COMMAND << endl;
		cout << endl << sProperty << endl;

		// erst mal alle Eigenschaften holen
		PropList propList;
		if (startProperty==ip::pf::NoHandle)
		{ // starten mit root
			camera.listAllProperties(propList, false);
		}
		else
		{
			camera.listAllProperties(propList, false, startProperty);
		}

		PropArray structList(propList.size());
		PropArray primList(propList.size());
		PropArray commList(propList.size());

		uInt structures	= 0;
		uInt primitives 	= 0;
		uInt commands	 	= 0;
		generatePropertyLists(camera, propList,
													primList, primitives,
													structList, structures,
													commList, commands);


		outputLists(camera, sProperty, needPrimitives,
								primList, primitives,
								structList, structures,
								commList, commands);

		// Antwort einlesen
		char keyCommand = getAnswer();
	//	ch = getPrimitivAnswer();

		return	executeCommand(camera, sProperty, needPrimitives, keyCommand,
													 primList, primitives,
													 structList, structures,
													 commList, commands);
	}

	/**
	 * eine Zwischenschit über selectProperty. Diese rekursive Funktion erlaubt
	 * es rückwärts durch den Baum zu navigieren
	 */
	ip::pf::Handle  navigateProperties(ip::Camera &camera, ip::pf::Handle startProperty,
																	 bool &needPrimitives, bool &needStructures)
	{
		ip::pf::Handle property = startProperty;
		//cout << "no handle " << ip::pf::NoHandle << endl;
		do
		{
			bool prim = needPrimitives;
			bool struc = needStructures;
			property = selectProperty(camera, startProperty, prim, struc);
			if ( property != ip::pf::NoHandle) navigateProperties(camera, property, needPrimitives,
																												needStructures);
			//cout << " handle " << property << endl;
		} while ( property != ip::pf::NoHandle);
		return startProperty;
	}

	/**
	 * Funktion die die Filter so setzt, damit durch den
	 * ganzen 'normalen' Eigenschaftsbaum navigiert wreden kann
	 * ist die funktonali Implementierung des Befehls list
	 * List ist als rein interaktives Kommando gedacht
	 */
	bool executeList(ip::Camera &camera, String const& parameter)
	{
		bool needPrimitives = true;
		bool needStructures = true;
		// list ist immer interaktiv
		configureTerm(true);
		ip::pf::Handle property = parameter.empty() ? ip::pf::NoHandle : camera.findProperty(parameter);

		navigateProperties(camera, property, needPrimitives, needStructures);
		configureTerm(false);
		return true;
	}

	/**
	 * gleich wie list im Interaktiven Modus (kein Parameter)
	 * mit Parameter wird einfach diese Eigenschaft ausgelesen
	 */
	bool executeGet(ip::Camera &camera, String const& parameter)
	{
		if (!parameter.empty())
		{ // nicht interaktive variante
			//Poco::Timestamp timeStamp;
			 ip::pf::Handle prop = camera.findProperty(parameter);
			 if (prop!=ip::pf::NoHandle)
			 {
			 	ip::Property property(camera, prop);
			 	cout << property << endl;
			 }
			//cout << "getNI() " << timeStamp.elapsed() << " us"  << endl;
		}
		else
		{ // interaktive Variante
			bool needPrimitives = true;
			bool needStructures = true;
			configureTerm(true);
			//Poco::Timestamp timeStamp;
			ip::pf::Handle property = parameter.empty() ? ip::pf::NoHandle : camera.findProperty(parameter);
			//cout << "get() " << timeStamp.elapsed() << " us"  << endl;
			navigateProperties(camera, property, needPrimitives, needStructures);
			configureTerm(false);
		}
		return true;
	}

	/**
	 * gleich wie list im Interaktiven Modus (kein Parameter)
	 * mit Parameter und Wert wird einfach diese Eigenschaft gesetzt
	 */
	bool executeSet(ip::Camera &camera, String const& parameter, String const& value)
	{
		if (!parameter.empty() && !value.empty())
		{ // nicht interaktive variante
			//Poco::Timestamp timeStamp;
			 ip::pf::Handle prop = camera.findProperty(parameter);
			 if (prop!=ip::pf::NoHandle)
			 {
			 	ip::Property property(camera, prop);
			 	property = value;
			 	cout << property << endl;
			 }
			//cout << "setNI() " << timeStamp.elapsed() << " us"  << endl;
		}
		else
		{ // interaktive Variante
			bool needPrimitives = true;
			bool needStructures = true;
			configureTerm(true);
			//Poco::Timestamp timeStamp;
			ip::pf::Handle property = parameter.empty() ? ip::pf::NoHandle : camera.findProperty(parameter);
			//cout << "set() " << timeStamp.elapsed() << " us"  << endl;

			navigateProperties(camera, property, needPrimitives, needStructures);
			configureTerm(false);
		}
		return true;
	}

	/**
	 * wie list im Interaktiven Modus (kein Parameter), nur daß
	 * nur Strukturen und kommandos aufgelistet werden
	 * mit Parameter wird einfach dieses Kommando ausgeführt
	 */
	bool	executeCommand(ip::Camera &camera, String const& parameter)
	{
		if (!parameter.empty() )
		{ // nicht interaktive variante
			std::cout << "executing Command " << parameter << std::endl;
			ip::pf::Handle prop = camera.findProperty(parameter);
			if (prop!=ip::pf::NoHandle)
			{
				std::cout << "Command found" << parameter << std::endl;
				ip::Property property(camera, prop);
			 	property = "0"; // dummy - Parameter
			}
		}
		else
		{ // interaktive Variante
			bool needPrimitives = false;
			bool needStructures = true;
			configureTerm(true);
			ip::pf::Handle property = parameter.empty() ? ip::pf::NoHandle : camera.findProperty(parameter);

			navigateProperties(camera, property, needPrimitives, needStructures);
			configureTerm(false);
		}
		return true;
	}

	} // namespace precitec

	/**
	 * Hauptroutine
	 * analysiert Kommando Zeile
	 * wenn Kommandos sinnvoll extrahiert werden können
	 * werden diese ausgeführt
	 * sonst ein kleiner Hilfetext ausgegeben
	 */

	int main(int argc, char **argv)
	{
		std::cout << "Hallo Welt" << std::endl;
		using namespace precitec;

	  	int 			portNum;
		PfCommand 	command;
		String		parameter;
		String		value;
		if (!analyzeCommandLine(argc, argv, portNum, command, parameter, value)) return EXIT_FAILURE;
		String commString("");
		switch (command)
		{
			case List:
				commString = "List";
				break;
			case Get:
				commString = "Get";
				break;
			case Set:
				commString = "Set";
				break;
			case Execute:
				commString = "Exec";
				break;
			default:
			case Nothing:
				commString = "Nothing";
				break;
		}
		std::cout << "p0" << std::endl;

		// hardware ansprechen
		ip::pf::Port port(portNum);
		if (!port.isValid()) return EXIT_FAILURE;
		std::cout << "p1" << std::endl;
		ip::Camera camera(portNum);
		std::cout << "p2" << std::endl;
		if (!camera.isValid()) return EXIT_FAILURE; // Port wird hier durh Destruktor geschlossen

		bool ok = true;
		switch (command)
		{
			case List:
				ok = executeList(camera, parameter);
				break;
			case Get:
				ok = executeGet(camera, parameter);
				break;
			case Set:
				ok = executeSet(camera, parameter, value);
				break;
			default:
			case Execute:
				ok = executeCommand(camera, parameter);
				break;
			case Nothing:
				help();
				break;
		}
		return ok ? EXIT_SUCCESS : EXIT_FAILURE;

	}

