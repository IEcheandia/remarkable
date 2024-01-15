#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sstream>

#include "viWeldHead/Scanlab/contourMetaLanguage.h"

class SmartMoveInterpreterTest;

namespace precitec
{
namespace system
{
class MemoryFile;
}

namespace hardware
{

/**
* Smart move interpreter generates from an array filled with the contour meta language the specific smart move commands to create the corresponding contour.
**/
class SmartMoveInterpreter
{
public:
    SmartMoveInterpreter();
    ~SmartMoveInterpreter();

    const std::vector<std::string>& currentContour() const
    {
        return m_currentContourHPGL2;
    }
    bool isCurrentContourEmpty() const
    {
        return m_currentContourHPGL2.empty();
    }
    std::string filename() const
    {
        return m_HPGL2Filename;
    }
    void setFilename(const std::string& filename);
    int fileDescriptor() const;
    bool debug() const
    {
        return m_debug;
    }
    void setDebug(bool saveGeneratedFileToLogFolder);

    void newContour();
    void createContourFile(const std::vector<std::shared_ptr<contour::Command>>& contourMetaLanguage);

private:
    std::string translateInitialize();
    std::string translateMark(double x, double y);
    std::string translateJump(double x, double y);
    std::string translateLaserPower(double power);
    std::string translateMarkSpeed(double speed);

    void translateContour(const std::vector<std::shared_ptr<contour::Command>>& contourMetaLanguage);

    void writeAnonymousFile();

    void saveHPGL2Contour();

    std::unique_ptr<system::MemoryFile> m_inMemoryFile;
    std::vector<std::string> m_currentContourHPGL2;
    std::size_t m_contourFileSize{0};
    std::string m_HPGL2Filename;
    std::ostringstream m_stringstream;
    bool m_debug{false};

    friend SmartMoveInterpreterTest;
};

}
}
