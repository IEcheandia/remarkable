

#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#define MAX_LINE_NUMBER 600


namespace RTCLogging
{
    
class RTCLog
{
public:
    RTCLog();
    ~RTCLog(){};
 
    template<typename... Ts>
    void msgLog(const std::string& mssg, int count, Ts... rest)
    {
        if(m_oWriteIndex >= MAX_LINE_NUMBER)
        {
            m_oWriteIndex = 0;
            m_FileCount++;
            createNewLogFile();
        }    
    
        Poco::DateTime now;
        std::string timestamp = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::ISO8601_FORMAT);    
    
        m_oFile << "SIMULATED RTC BOARD| " << timestamp << " | " << "Function call to -> " << mssg << "(";        
    
        if(count != 0)
        {  
             ((m_oFile << rest << ", "), ...);  
            
            m_oFile.seekp(-2, std::ios::end);
            m_oFile.put('\0');
            m_oFile << ");";
            
        }
        else
        {        
            m_oFile << ");";        
        }    
    
        m_oFile << std::endl;
    
        m_oWriteIndex++;
        return;
    }
 
    
    
    
private:
    void init();
    void createNewLogFile();
    void increaseFileIndex();
    
    
    
    std::string       m_oSt_LogFilePath; 
    std::ofstream     m_oFile;
    uint32_t          m_oWriteIndex;
    uint16_t          m_FileCount;
  
    
};
    
}
