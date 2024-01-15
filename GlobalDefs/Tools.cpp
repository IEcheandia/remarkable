/*
 * Tools.cpp
 *
 *  Created on: 23.10.2009
 *      Author: f.agrawal
 */
#include <cstdio>

#include "Tools.h"

Tools::Tools() {
	// TODO Auto-generated constructor stub

}

Tools::~Tools() {
	// TODO Auto-generated destructor stub
}

void Tools::ErrorLog(std::string sError, char* desc, unsigned long nErr) {

	//TODO: add debug levels...
	if (desc != NULL && nErr != 0) {
		std::printf("Error: %s (Result = %s 0x%lx)\n", sError.c_str(), desc, nErr);
	} else {
		std::printf("Error: %s\n", sError.c_str());
	}
}

void Tools::InfoLog(std::string sInfo) {

	//TODO: add debug levels...
	std::printf("Info: %s\n", sInfo.c_str());
}

std::string Tools::ConcatStringInt(std::string s, int n) {
	std::stringstream sstm;
	sstm << s << n;
	return sstm.str();
}

std::string Tools::ConcatStringProductCodeInt(std::string s, int productCode, int id) {
	std::stringstream sstm;
	sstm << s << "_" << productCode << "_" << id;
	return sstm.str();
}

std::string Tools::CreateMsgString(std::string connectionType, std::string slaveType, int vendorID, int productCode, int id) {
	std::stringstream sstm;

	sstm << connectionType << slaveType << vendorID << "_" << productCode << "_" << id;
	return sstm.str();
}

int Tools::StringToInt(std::string s) {
	// String -> Int
	std::stringstream sstr(s);
	int ret;
	sstr >> ret;
	return ret;
}

