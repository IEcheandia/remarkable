/*
 * Tools.h
 *
 *  Created on: 23.10.2009
 *      Author: f.agrawal
 */

#ifndef TOOLS_H_
#define TOOLS_H_


#include <sstream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include "GlobalDefs.h"
#include "DefsHW.h"

/**
 * Hilfsfunktionen
 **/
class Tools {
public:
	Tools();
	virtual ~Tools();

	/**
	 * ErrorLog
	 * @param sError Fehlerbeschreibung
	 * @param desc char* Beschreibung (z.B. vom AT-EM)
	 * @param nErr FehlerID
	 **/
	static void ErrorLog(std::string sError,char* desc= NULL, unsigned long nErr = 0);

	/**
	 * InfoLog
	 * @param sInfo Information
	 **/
	static void	InfoLog(std::string sInfo);

	/**
	 * Concat String und int
	 * @param s String
	 * @param n int
	 * @return Ergebnis
	 **/
	static std::string ConcatStringInt(std::string s, int n);

	/**
	 * Concat String + ProductCode (int) + int
	 * @param s String
	 * @param productCode int
	 * @param id int
	 * @return Ergebnis
	 **/
	static std::string ConcatStringProductCodeInt(std::string s,int productCode, int id);

	/**
	 * Wandelt String in int um
	 * @param s String
	 * @return \e s umgewandelt in int
	 **/
	static int StringToInt(std::string s);

	/**
	 * Erzeuge Verbindungsstring fuer VMI-Framework Messaging (QNX MessagePassing)
	 * @param connectionType ConnectionType
	 * @param slaveType SlaveType
	 * @param vendorID VendorID
	 * @param productCode ProductCode
	 * @param id ID (Instanz)
	 * @return Verbindungsstring
	 **/
	static std::string CreateMsgString(std::string connectionType, std::string slaveType, int vendorID, int productCode, int id);
};
#endif /* TOOLS_H_ */
