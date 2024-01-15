#ifndef WELDHEADDEFAULTS_H_
#define WELDHEADDEFAULTS_H_

#include <iostream>
#include <fstream>
#include <Poco/File.h>

#include "common/systemConfiguration.h"

namespace precitec
{

namespace interface
{

const std::string  WeldHeadDefFileName("WeldHeadDefaults.xml");

class WeldHeadDefaults : public SystemConfiguration
{
private:
    static std::string configFilePath()
    {
        std::string oHomeDir(getenv("WM_BASE_DIR"));
        std::string oWeldHeadDefFileName = oHomeDir + "/config/" + WeldHeadDefFileName;
        std::cout << "Filename: " << oWeldHeadDefFileName << std::endl;

        if (!Poco::File{oWeldHeadDefFileName}.exists())
        {
            std::ofstream newfile;
            newfile.open(oWeldHeadDefFileName.c_str());
            if (newfile)
            {
                newfile << "<WeldHeadDefaults>" << std::endl;
                newfile << "	<File_New_Created>1</File_New_Created>" << std::endl;
                newfile << "</WeldHeadDefaults>" << std::endl;
                newfile.close();
            }
        }

        return oWeldHeadDefFileName;
    }

    WeldHeadDefaults()
        : SystemConfiguration(configFilePath())
    {
    }

public:
    static WeldHeadDefaults &instance()
    {
        static WeldHeadDefaults s_instance;
        return s_instance;
    }

};

} // namespace interface

} // namespace precitec

#endif /*WELDHEADDEFAULTS_H_*/

