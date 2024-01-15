#ifndef INSPECTIONCONTROLDEFAULTS_H_
#define INSPECTIONCONTROLDEFAULTS_H_

#include <fstream>
#include <iostream>
#include <Poco/File.h>
#include "common/systemConfiguration.h"

namespace precitec
{

namespace interface
{

const std::string  InspContrDefFileName("InspectionControlDefaults.xml");

class InspectionControlDefaults : public SystemConfiguration
{
private:

    static std::string configFilePath()
    {
        std::string oHomeDir(getenv("WM_BASE_DIR"));
        std::string oInspContrDefFileName = oHomeDir + "/config/" + InspContrDefFileName;
        std::cout << "Filename: " << oInspContrDefFileName << std::endl;

        if (!Poco::File{oInspContrDefFileName}.exists())
        {
            std::ofstream newfile;
            newfile.open(oInspContrDefFileName.c_str());
            if (newfile)
            {
                newfile << "<InspectionControlDefaults>" << std::endl;
                newfile << "	<File_New_Created>1</File_New_Created>" << std::endl;
                newfile << "</InspectionControlDefaults>" << std::endl;
                newfile.close();
            }
        }

        return oInspContrDefFileName;
    }

    InspectionControlDefaults()
        : SystemConfiguration(configFilePath())
    {
    }

public:
    static InspectionControlDefaults &instance()
    {
        static InspectionControlDefaults s_instance;
        return s_instance;
    }
};

} // namespace interface

} // namespace precitec

#endif /*INSPECTIONCONTROLDEFAULTS_H_*/

