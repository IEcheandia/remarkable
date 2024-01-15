#ifndef TEST_SERVER_SERVER_H_
#define TEST_SERVER_SERVER_H_

#include <iostream>

#include "system/types.h"					// wg PvString
#include "message/testServer.interface.h"	// hier werden alle weiteren Header eingebunden

namespace precitec
{
namespace interface
{
	// alle Tempate-Instantiierungen muessen im gleichen Namespace erfolgen
	template <>
	class TTestServer<MsgServer> : public TTestServer<AbstractInterface>
	{
	public:
		TTestServer() {}
		virtual ~TTestServer() {}
	public:
		/// addiert zwei Zahlen
		int add(int a, int b) { /*std::cout << "server: " << a << " + " << b << " = " << a+ b << std::endl; */return a+b;	}

		/// erzeugt einen bool
		bool compare(int a, int b, float c, float d) {
			//std::cout << "server cmp:" << a << ", "	<< b << ", " << c	<< ", " << d << " = "
			//<< ((a==b) && (c==d)) << std::endl;
			return (a==b) && (c==d);
		}

		/// gibt lokal einen String aus
		void output(void) { std::cout << "server print: ich tu' was" << std::endl; }

		/// kopiert PvString unveraendert in den Ausgang
		PvString copy(PvString const& s) { /*std::cout << "server copy:" << s << std::endl; */return s; }

		/// konkateniert zwei PvStrings
		PvString cat(PvString const& s, PvString const& t) { /*std::cout << "server cat:" << s+t << std::endl; */return s+t; }

		/// Test vuer beliebig lange Felder,  hat Element-Test
		Field multiplyField(Field const& f, int factor) {
			return f.multiply(factor);
		}

		/// Test fuer beliebig lange Felder,  hat Element-Test
		Field getField(int size) {
			return Field(size);
		}

		/// Test vuer beliebig lange Felder,  hat Element-Test
		void sendField(Field const& f) {
			//std::cout << "testServer[Server]::sendField( " << ")" << std::endl;
			std::cout << f << std::endl;
			//std::cout << "testServer[Server]::sendField() ok" << std::endl;
		}
	};

} // interface
} // precitec


#endif /*TEST_SERVER_SERVER_H_*/
