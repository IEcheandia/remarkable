#include "util/CalibrationCorrectionContainer.h"
#include <module/moduleLogger.h>

#include <fstream>
#include <sstream>
#include <locale>

namespace precitec {
namespace coordinates{


   

   
   
/*static*/ CalibrationCameraCorrectionContainer CalibrationCameraCorrectionContainer::load(std::string filename)    
{
	std::ifstream oIn(filename.c_str());
	if (!oIn.good())
	{
		wmLog(eWarning, "File" + filename + "cannot be read\n");
        return {};
	}
	const char separator = ';';
	enum Field
	{
        eScannerX = 0,
        eScannerY,
        eTCPOffsetX,
        eTCPOffsetY,
        NUMFIELDS
    };
    
	
	typedef std::pair<geo2d::DPoint, CalibrationCameraCorrection> offset_t;
	std::vector<offset_t> offsets ;
	
    std::string line;
    while ( std::getline(oIn, line) )
    {
        std::stringstream   lineStream(line);
        std::string   tok;

        
        std::array<double, NUMFIELDS> values;
        int counter = 0;
        while ( std::getline(lineStream, tok, separator) )
        {
            if (counter == NUMFIELDS)
            {
                //discard extra char
                break;
            }
            std::istringstream tokStream(tok);
            tokStream.imbue(std::locale::classic());
            if (!(tokStream >> values[counter]))
            {
                break;
            }
            
            counter++;            
        }
        if (counter == NUMFIELDS) 
        {
            //discard lines that do no have exactly 4 numbers   (e.g header)
            offsets.push_back(offset_t{{values[eScannerX],values[eScannerY]}, {values[eTCPOffsetX],values[eTCPOffsetY]}});
        }
        
    }
    
    return {offsets};
}
/*static*/ CalibrationIDMCorrectionContainer CalibrationIDMCorrectionContainer::load(std::string filename)    
{
	std::ifstream oIn(filename.c_str());
	if (!oIn.good())
	{
		wmLog(eWarning, "File" + filename + "cannot be read\n");
        return {};
	}
	const char separator = ';';
	enum Field
	{
        eScannerX = 0,
        eScannerY,
        eZMedian,
        eOffset,
        eCalibratedValue,
        NUMFIELDS
    };
    
	typedef std::pair<geo2d::DPoint, CalibrationIDMCorrection> offset_t;
	std::vector<offset_t> offsets ;
	
    std::string line;
    while ( std::getline(oIn, line) )
    {
        std::stringstream lineStream(line);
        std::string   tok;
        
        std::array<double,NUMFIELDS> values;
        int counter = 0;
        while ( std::getline(lineStream, tok, separator) )
        {
            if (counter == NUMFIELDS)
            {
                //discard extra char
                break;
            }
            char* end;
            values[counter] = std::strtod(tok.c_str(), &end);
            if (end == tok.c_str())
            {
                break;
            }
            
            counter++;            
        }
        if (counter == NUMFIELDS ) 
        {
            //discard lines that do not have the min number of numeric values (e.g header)
            offsets.push_back(offset_t{
                                        {values[eScannerX],values[eScannerY]}, 
                                        { int(std::round(values[eOffset]))}}); 
            
        }
    
        
    }
    
    return {offsets};
}
/*static*/ bool CalibrationCameraCorrectionContainer::write(const CalibrationCameraCorrectionContainer & corrections, std::string filename)
{
    
	std::ofstream oOut(filename.c_str());
	oOut.imbue(std::locale::classic()); 
	if (!oOut.good())
	{
		wmLog(eWarning, "File" + filename + "cannot be written\n");
        return false;
	}
	const char separator = ';' ;
    
    oOut << "Scanner X " << separator
            << "Scanner Y " << separator 
            << "TCP x" << separator 
            << "TCP y" << separator 
            << "\n";
    const auto & rArray = corrections.m_data.m_correctionArray;
    for (int index = 0, n = rArray.size(); index < n; index ++)
    {
        const auto & rCorrection = rArray[index];
        auto pos = corrections.m_data.computeInputPosition(index);
        oOut << pos[0] << separator
            << pos[1] << separator 
            << rCorrection.m_oTCPXOffset << separator 
            << rCorrection.m_oTCPYOffset << separator 
            << "\n";
    }
    return true;
    
}





CalibrationCameraCorrectionState::CalibrationCameraCorrectionState (CalibrationCameraCorrectionContainer init)
    : m_container(std::move(init))
{
    for (unsigned int i = 0; i < g_oNbParMax; i++)
    {
        m_currentPosition[i] = {0.0,0.0};
        m_currentCorrection[i] = m_container.get(m_currentPosition[i]);
    }
}
    
bool CalibrationCameraCorrectionState::updateCorrectionPosition(double x,  double y, int index)
{
    bool changed = false;
    auto & rCurrentPosition = m_currentPosition[index];
    
    if (rCurrentPosition[CalibrationCorrectionContainer<CalibrationCameraCorrection>::eX] !=  x 
        || rCurrentPosition[CalibrationCorrectionContainer<CalibrationCameraCorrection>::eY] !=  y)
    {
        rCurrentPosition = CalibrationCorrectionContainer<CalibrationCameraCorrection>::InputPosition{x, y};
        m_currentCorrection[index] = m_container.get(rCurrentPosition);
        changed = true;
    }
    return changed;
}

geo2d::DPoint CalibrationCameraCorrectionState::getCurrentPosition(int index) const
{
    return {m_currentPosition[index][CalibrationCorrectionContainer<CalibrationCameraCorrection>::eX], m_currentPosition[index][CalibrationCorrectionContainer<CalibrationCameraCorrection>::eY] };
}

const CalibrationCameraCorrection & CalibrationCameraCorrectionState::getCurrentCorrection(int index) const
{
    return m_currentCorrection[index];
}

CalibrationCameraCorrection CalibrationCameraCorrectionState::computeCorrection(double x,  double y) const
{
    return m_container.get(CalibrationCorrectionContainer<CalibrationCameraCorrection>::InputPosition{x,y});
}

const CalibrationCameraCorrectionContainer & CalibrationCameraCorrectionState::getCorrectionContainer() const
{
    return m_container;
}




CalibrationCameraCorrection CalibrationCameraCorrectionContainer::get ( double x, double y ) const
{
    return m_data.get ( x,y );
}




CalibrationCameraCorrection CalibrationCameraCorrectionContainer::get ( CalibrationCorrectionContainer< CalibrationCameraCorrection >::InputPosition pos ) const
{
    return m_data.get ( pos );
}

int CalibrationIDMCorrectionContainer::getDelta( double x, double y ) const
{
    return m_data.get( x,y).m_oDelta;
}

double CalibrationIDMCorrectionContainer::get ( double x, double y , double rawZ) const
{
    return m_model.fit(x, y, rawZ) + getDelta( x,y );
}


void CalibrationCameraCorrectionContainer::serialize ( system::message::MessageBuffer& buffer ) const
{
    marshal ( buffer, m_data );
}


void CalibrationCameraCorrectionContainer::deserialize ( const system::message::MessageBuffer& buffer )
{
    deMarshal ( buffer, m_data );
}


CalibrationIDMCorrectionContainer::CalibrationIDMCorrectionContainer()
{}

CalibrationIDMCorrectionContainer::CalibrationIDMCorrectionContainer(const std::vector<std::pair<geo2d::DPoint, CalibrationIDMCorrection>> & offsets)
    : m_data(offsets)
    {}


void CalibrationIDMCorrectionContainer::serialize ( system::message::MessageBuffer& buffer ) const
{
    marshal ( buffer, m_data );
    marshal ( buffer, m_model );
}


void CalibrationIDMCorrectionContainer::deserialize ( const system::message::MessageBuffer& buffer )
{
    deMarshal ( buffer, m_data );
    deMarshal ( buffer, m_model );
}

}
}
