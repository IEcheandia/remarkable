#include "util/camGridData.h"
#include "math/2D/avgAndRegression.h"
#include "math/mathCommon.h" //constrain angle
#include "Poco/Checksum.h"

#include <algorithm>				///< for_each
#include <iomanip>
#include <tuple>		//get<>
#include <fstream>
#include <cctype>
#include <sstream> 

using precitec::geo2d::coord2D;

#ifndef NDEBUG
const bool ASSERT_INVALID_CAMGRIDDATA = false;
#endif


namespace {
    //Camera Flash functions
    //fields
    enum CamGridDataBytes_v3
    {
        e_magicnumber = 0, //'W' 'M' 'C'
        e_version = 3, 
        e_endAddress = 4, //2 bytes (0:2048)
        e_checksum = 6, // 4 bytes
        e_numCorners = 10, //2 bytes to represent the number of corners (for example 300)
        e_numRows = 12,
        e_numColumnsMin = 13,
        e_numColumnsMax = 14,
        e_gridDelta = 15, //mm*100 :the expected values are 60, 135, 220
        e_sensorWidth = 16, //2 bytes: 1024
        e_sensorHeight = 18, //2 bytes: 1024
        e_triangulationAngle = 20, // 4 bytes
        e_dataStart = 24, //each point (x = 0-1023, y = 0-1023) occupies 3 bytes
        e_MAX = 2048
    };

    /*
    * use 2 bytes to represent a 16 bit integer
    * e.g nint=0x0F   byte1=F  byte2=0
    * */

    void convertUInt16ToBytes(std::uint16_t nint, std::uint8_t & byte1, std::uint8_t & byte2)
    {
        byte1 =  nint & 0x00ff;
        byte2 = (nint & 0xff00) >> 8;
    }

    void convertBytesToUInt16(std::uint8_t byte1, std::uint8_t byte2, std::uint16_t & nint)
    {
        nint = static_cast<std::uint16_t> (byte1);
        std::uint16_t int2 = static_cast<std::uint16_t> (byte2) << 8;
        nint |= int2;
    }

    void convertUInt32ToBytes(std::uint32_t nint, std::uint8_t & byte1, std::uint8_t & byte2, std::uint8_t & byte3, std::uint8_t & byte4)
    {
        byte1 =  nint & 0x000000ff;
        byte2 = (nint & 0x0000ff00) >> 8;
        byte3 = (nint & 0x00ff0000) >> 16;
        byte4 = (nint & 0xff000000) >> 24;
    }

    void convertBytesToUInt32(std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3, std::uint8_t byte4, std::uint32_t & nint)
    {
        nint = static_cast<std::uint32_t> (byte1);
        std::uint32_t int2 = static_cast<std::uint16_t> (byte2) << 8;
        std::uint32_t int3 = static_cast<std::uint16_t> (byte3) << 16;
        std::uint32_t int4 = static_cast<std::uint16_t> (byte4) << 24;
        nint |= int2;
        nint |= int3;
        nint |= int4;
        
    }
    
    void convertConvexAngleToBytes(const double & angleRad, std::uint8_t & byte1, std::uint8_t & byte2, std::uint8_t & byte3, std::uint8_t & byte4)
    {
        using namespace precitec::math;
        //ensure that the angle is in the range [0 , pi)
        double angle = constrainAngle(angleRad, angleUnit::eRadians, false);
        std::uint32_t intRepresentation = angle * 1e9;
        convertUInt32ToBytes(intRepresentation, byte1, byte2, byte3, byte4);
        
    }

    void convertBytesToConvexAngle(std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3, std::uint8_t byte4, double & angleRad)
    {
        std::uint32_t intRepresentation;
        convertBytesToUInt32(byte1, byte2, byte3, byte4, intRepresentation);
        angleRad = double(intRepresentation) *1e-9;     
    }

    
    /*
    * use 3 Bytes to represent a pair of coordinates (each between 0-1280, must be less than 4096),
    * e.g x=   0[0x000], y = 1023[0x 3FF] -> byte1 =  0[0x00] byte2=  3[0x03] byte3=255[0xFF] 
    * e.g x=1023[0x3FF], y =    0[0x 000] -> byte1 = 63[0x3F] byte2=240[0xF0] byte3=  0[0x00] 
    * e.g x=1023[0x3FF], y = 1023[0x 3FF] -> byte1 = 63[0x3F] byte2=243[0xF3] byte3=255[0xFF] 
    * e.g x=  17[0x011], y =  123[0x  7B] -> byte1 =  2[0x01] byte2= 16[0x10] byte3=123[0x7B]
    * to check: x = byte1*16+byte2/16; y = byte2%16*256+byte3
    * byte1 = x/16
    * byte3 = y%256
    */
    void convertBytesToCoords(std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3, std::uint16_t & x, std::uint16_t & y)
    {
        x = static_cast<std::uint16_t> (byte1)        << 4 |  static_cast<std::uint16_t> (byte2) >> 4;
        y = static_cast<std::uint16_t> (byte2 & 0x0f) << 8 |  static_cast<std::uint16_t> (byte3);
        assert(x == ( byte1*16+ byte2/16));
        assert(y == ( (byte2%16)*256+ byte3));
    }

    void convertCoordsToBytes(std::uint16_t x,std::uint16_t y, std::uint8_t & byte1, std::uint8_t & byte2, std::uint8_t & byte3 )
    {
        assert((x & 0xf000)== 0); //x < 4096
        assert((y & 0xf000)== 0); //y < 4096

        std::uint16_t byte2_x, byte2_y;

        byte1 =  (x & 0x0ff0) >> 4;
        byte2_x =(x & 0x000f) << 4;
        byte2_y =(y & 0x0f00) >> 8;
        byte3 =  (y & 0x00ff);
        byte2 = byte2_x | byte2_y;
    }    
    
    //read floating point value forcing classic locale (not particularly perfomant)
    double atof_classiclocale(const char* s)
    {
        std::istringstream oStream(s);
        oStream.imbue(std::locale::classic());
        double value;
        oStream >> value;
        return value;
    }
    
   
}


namespace precitec
{
namespace system
{
const char CamGridData::mFillChar(' ');
const char CamGridData::mSeparator(';');
const char CamGridData::mWhitespace[7]{' ', '\t', '\n', '\v', '\f', '\r', mFillChar };//make sure fillchar is included
const unsigned int CamGridData::mBytesHeaderSize(CamGridDataBytes_v3::e_dataStart);
const unsigned int CamGridData::mBytesMaxAddress(CamGridDataBytes_v3::e_MAX);

CamGridData::CamGridData()
{
	setDefaults();
}



//*****************************************************
//*  Calibration from Camera Flash functions (v03)    *
//*****************************************************


void CamGridData::saveToBytes(std::vector<std::uint8_t> & rBytes) const
{
    unsigned int nCorners(0), nRows(0), nColsMin(0), nColsMax(0);
    parseGridSize(nCorners, nRows, nColsMin, nColsMax);

    rBytes.resize(e_dataStart + nCorners*3);
    
    rBytes[e_magicnumber] = 'W';
    rBytes[e_magicnumber+1] = 'M';
    rBytes[e_magicnumber+2] = 'C';
    rBytes[e_version] = 3;
    
    //checksum and endAddress will be computed at the end
    convertUInt16ToBytes(nCorners, rBytes[e_numCorners], rBytes[e_numCorners+1] );

    rBytes[e_numRows] = static_cast<std::uint8_t>(nRows);
    rBytes[e_numColumnsMin] = static_cast<std::uint8_t>(nColsMin);
    rBytes[e_numColumnsMax] = static_cast<std::uint8_t>(nColsMax);
    rBytes[e_gridDelta] = static_cast<std::uint8_t>(m_oGridDelta*100);
    convertUInt16ToBytes(m_oSensorWidth, rBytes[e_sensorWidth], rBytes[e_sensorWidth+1]);
    convertUInt16ToBytes(m_oSensorHeight, rBytes[e_sensorHeight], rBytes[e_sensorHeight+1]);
    convertConvexAngleToBytes(m_oAngleRad, rBytes[e_triangulationAngle], rBytes[e_triangulationAngle+1],rBytes[e_triangulationAngle+2],rBytes[e_triangulationAngle+3]);

    std::uint16_t offset = e_dataStart;
    unsigned int pointCounter = 0;
    for ( auto & rLine : m_oGrid )
    {
        for (const geo2d::coord2D & point : rLine.second)
        {

            assert(offset == e_dataStart + pointCounter*3);
            assert(offset+3 < (CamGridDataBytes_v3::e_MAX));
            convertCoordsToBytes(point.ScreenX, point.ScreenY, rBytes[offset],rBytes[offset+1], rBytes[offset+2]);
            pointCounter++;
            offset += 3;
        }
    }
    assert(offset == rBytes.size());
    convertUInt16ToBytes(offset, rBytes[e_endAddress], rBytes[e_endAddress+1]);
    assert(pointCounter == nCorners);
    
    std::uint32_t oChecksumValue = computeChecksum(rBytes);
    convertUInt32ToBytes(oChecksumValue, rBytes[e_checksum], rBytes[e_checksum+1],rBytes[e_checksum+2],rBytes[e_checksum+3]);
}


bool CamGridData::checkBytesHeader(const std::vector<std::uint8_t>& rBytes, std::uint16_t & rEndAddress, std::uint32_t& rChecksum)
{
    if (rBytes.size() < mBytesHeaderSize)
    {
        return false;
    }
    if (rBytes[e_magicnumber] != 'W' 
        || rBytes[e_magicnumber+1] != 'M'
        || rBytes[e_magicnumber+2] != 'C'
        || rBytes[e_version] != 3    )
    {
        return false;
    }

    
    convertBytesToUInt16(rBytes[e_endAddress], rBytes[e_endAddress+1], rEndAddress);
    assert(rEndAddress < e_MAX);
    
    convertBytesToUInt32(rBytes[e_checksum], rBytes[e_checksum + 1], rBytes[e_checksum+2], rBytes[e_checksum+3], rChecksum);
    
    return rEndAddress > 0;
    
}

//assumes rBytes is valid
std::uint32_t CamGridData::computeChecksum(const std::vector<std::uint8_t> & rBytes)
{
    
    std::uint16_t oEndAddress;
    
    convertBytesToUInt16(rBytes[e_endAddress], rBytes[e_endAddress+1], oEndAddress);
        
    unsigned int start = e_checksum + 4 ;
    Poco::Checksum oChecksum;
    oChecksum.update(reinterpret_cast<const char*>(rBytes.data()) + start,  oEndAddress - start );    
    return oChecksum.checksum();
}

bool CamGridData::loadFromBytes(const std::vector<std::uint8_t> & rBytes, std::uint32_t& rChecksum, bool verifyChecksum)
{
    setDefaults();
    std::uint16_t endAddress;
    
    bool valid = checkBytesHeader(rBytes, endAddress, rChecksum);
    if (!valid )
    {
        std::cout << "Invalid header " << std::endl;
        return false;
    }
    if (rBytes.size() < endAddress)
    {
        return false;
    }
    if (verifyChecksum && (computeChecksum(rBytes)!= rChecksum))
    {
        std::cout << "Wrong checksum " << std::endl;
        return false;
    }

    std::uint16_t expectedNumCorners;
    convertBytesToUInt16(rBytes[e_numCorners], rBytes[e_numCorners+1], expectedNumCorners );

    m_oGridDelta = rBytes[e_gridDelta]/100.0;
    std::uint16_t oSensorWidth, oSensorHeight;
    convertBytesToUInt16(rBytes[e_sensorWidth], rBytes[e_sensorWidth+1], oSensorWidth);
    convertBytesToUInt16(rBytes[e_sensorHeight], rBytes[e_sensorHeight+1], oSensorHeight);
    
    //cast
    setSensorSize(oSensorWidth, oSensorHeight);

    double oAngleRad;
    convertBytesToConvexAngle(rBytes[e_triangulationAngle], rBytes[e_triangulationAngle+1],rBytes[e_triangulationAngle+2],rBytes[e_triangulationAngle+3], oAngleRad);
    if (! (oAngleRad >= 0) && oAngleRad < precitec::math::STRAIGHT_ANGLE_RAD)
    {
        return false;
    }
    setTriangulationAngle_rad(oAngleRad);
    
    //see CornerExtractor::generateGridMap
    tGridMap oGridMap;
    std::vector<precitec::geo2d::coord2D> oCurrentLine;
    int oYSum = 0;

    std::uint16_t lastX(0);
    bool ok = true;
    std::uint16_t offset = e_dataStart;
    for (std::uint16_t cornerCounter=0;
         cornerCounter < expectedNumCorners && ok;
         offset+=3, ++cornerCounter)
    {
        assert(oCurrentLine.size() == 0 || lastX == oCurrentLine.back().ScreenX);

        std::uint16_t x,y;
        convertBytesToCoords(rBytes[offset], rBytes[offset+1], rBytes[offset+2],x,y);

        //check if this corner still belogns to the same line
        if (x < lastX )  //assuming no line has only 1 point
        {
            assert(oCurrentLine.size() > 0);
            //the previous line has ended
            int oYAvg = std::round(oYSum / double(oCurrentLine.size()));
            auto oInsertionResult = oGridMap.insert(std::make_pair(oYAvg, oCurrentLine));
            assert(oInsertionResult.second || !ASSERT_INVALID_CAMGRIDDATA );
            (void) oInsertionResult;
            oCurrentLine.clear();
            oYSum = 0;
            lastX = 0;
        }
        oYSum += y;
        lastX = x;
        oCurrentLine.push_back(coord2D(x,y));
    }
    //insert the last line
    assert(oCurrentLine.size() > 0);
    int oYAvg = std::round(oYSum / double(oCurrentLine.size()));
    auto oInsertionResult = oGridMap.insert(std::make_pair(oYAvg, oCurrentLine));
    assert(oInsertionResult.second || !ASSERT_INVALID_CAMGRIDDATA);
    (void) oInsertionResult;
    ok &= (offset == endAddress);
    
    if (!ok)
    {
        std::cout << "Error when loading CamGridData from bytes" << std::endl;
        return false;
    }

    //see CamGridDataSingleton::setCorners
    setGridMap(oGridMap);
    if (!isCompatibleWithHeader(rBytes))
    {
        m_oGrid.clear();
        return false;
    }
    assert(hasData() || !ASSERT_INVALID_CAMGRIDDATA);
    return true;
}

bool CamGridData::saveToBytes(const std::string filename) const
{
    std::ofstream output(filename, std::ios::binary);
    if (!output.good())
    {
        std::cout << "Can't write to " << filename << std::endl;
        return false;
    }
    std::vector<std::uint8_t> oBytes;
    saveToBytes(oBytes);
    std::copy(oBytes.begin(), oBytes.end(), std::ostreambuf_iterator<char>(output));
    return true;
}

bool CamGridData::loadFromBytes(const std::string filename, std::uint32_t& rChecksum, bool verifyChecksum)
{
    std::ifstream input(filename , std::ios::binary);
    if (!input.good())
    {
        std::cout << "Can't read " << filename << std::endl;
        return false;
    }
    std::vector<std::uint8_t> oBytes ((std::istreambuf_iterator<char>(input)),(std::istreambuf_iterator<char>()));
    return loadFromBytes(oBytes, rChecksum, verifyChecksum);            
}

bool CamGridData::isCompatibleWithHeader(const std::vector<std::uint8_t> & rBytes) const
{
    std::uint16_t expectedNumCorners;
    convertBytesToUInt16(rBytes[e_numCorners], rBytes[e_numCorners+1], expectedNumCorners );

    //sanity check
    std::uint8_t expectedNumRows = rBytes[e_numRows];
    std::uint8_t expectedNumColumnsMin = rBytes[e_numColumnsMin];
    std::uint8_t expectedNumColumnsMax = rBytes[e_numColumnsMax];
    
    unsigned int nCorners(0), nRows(0), nColsMin(0), nColsMax(0);
    parseGridSize(nCorners, nRows, nColsMin, nColsMax);

    return nCorners == expectedNumCorners
            && nRows == expectedNumRows
            && nColsMin == expectedNumColumnsMin
            && nColsMax == expectedNumColumnsMax;
    
}



// ************* calibration camera flash functions end *************

void CamGridData::setGridMap(const tGridMap &p_rGrid)
{
    m_oGrid = p_rGrid;
}

void CamGridData::setSerial(const std::string p_oSerial)
{
    m_oSerial = p_oSerial;
}

void CamGridData::setSensorSize(const unsigned int p_oWidth, const unsigned int p_oHeight)
{
    m_oSensorWidth = p_oWidth;
    m_oSensorHeight = p_oHeight;
}

void CamGridData::setSensorWidth(const unsigned int p_oWidth)
{
    m_oSensorWidth = p_oWidth;
}

void CamGridData::setSensorHeight(const unsigned int p_oHeight)
{
    m_oSensorHeight = p_oHeight;
}

void CamGridData::setTriangulationAngle_rad(double p_oAngle)
{
    m_oAngleRad = p_oAngle;
    m_oHasTriangAngle = true;
}

bool CamGridData::hasTriangulation()
{
	return m_oHasTriangAngle;
}

void CamGridData::setGridDelta(const double p_oGridDelta)
{
    m_oGridDelta = p_oGridDelta;
}


void CamGridData::setDefaults()
{
    m_oGridDelta = 1.35; 
    m_oAngleRad = 0.0; 
    m_oSensorWidth = 1024; 
    m_oSensorHeight = 1024;
    m_oSerial = std::string(""); 
    m_oGrid.clear(); 
    m_oHasTriangAngle = false;
    m_oCorrectionFactor = 1;
}


// ---------------------------------------


void CamGridData::show(std::ostream & out) const
{
	out << std::fixed << std::setprecision(4)
		<< "gridDelta " << m_oGridDelta << "\n"
		<< "triangulation " << m_oHasTriangAngle << " " << triangulationAngle_deg() << "deg " << m_oAngleRad << " rad"<<"\n"
		<< "Sensor size " << m_oSensorWidth << " " << m_oSensorHeight << "\n"
		<< "Origin " << originX() << " " << originY() <<  "\n"
		<< "Serial " << m_oSerial << "\n"
		<< "Correction Factor " << m_oCorrectionFactor << "\n"
		<< "Grid Lines \n";
	gridToCsv(m_oGrid, out);
	out << "\n";
}

void CamGridData::parseGridSize(unsigned int & nPoints, unsigned int & nRows, unsigned int & nColsMin, unsigned int & nColsMax) const
{
	nPoints = 0;
	nRows = m_oGrid.size();
	nColsMax = 0;
	if (nRows == 0)
	{
		nColsMin = 0;
		return;
	}
	nColsMin = sensorHeight();
	for ( auto && rLine : m_oGrid )
	{
		unsigned int nCols = rLine.second.size();
		nPoints += nCols;
		nColsMin = nCols < nColsMin ? nCols : nColsMin;
		nColsMax = nCols > nColsMax ? nCols : nColsMax;
	}
	assert(nPoints >= nColsMin * nRows);
	assert(nPoints <= nColsMax * nRows);
}

bool CamGridData::hasData() const
{
    return m_oGrid.size() > 0;
}

std::string CamGridData::saveToCSV(const std::string & pFilename) const
{
	std::ostringstream oErrorMsg;	
	try
	{
		std::ofstream oFileStream(pFilename.c_str());
		oFileStream.imbue(std::locale::classic());
		
		oFileStream << "GridDelta" << mSeparator << m_oGridDelta << '\n';
		oFileStream << "SensorWidth" << mSeparator << m_oSensorWidth << '\n';
		oFileStream << "SensorHeight" << mSeparator << m_oSensorHeight << '\n';
		oFileStream << "CorrectionFactor" << mSeparator << m_oCorrectionFactor << '\n';
		oFileStream << "TriangAngleRad" << mSeparator << triangulationAngle_rad()  << '\n';
		oFileStream << "GridCoordinates"<<'\n';
		gridToCsv(m_oGrid, oFileStream);
	}
	catch ( std::exception &p_rException )
	{
		oErrorMsg << p_rException.what() << std::endl;
	}
	return oErrorMsg.str();
}


std::string CamGridData::loadFromCSV(const std::string & pFilename)
{
	setDefaults(); //reset internal data
    
	std::ostringstream oErrorMsg;
	std::ifstream oIn(pFilename.c_str());
	if (!oIn.good())
	{
		return "File" + pFilename + "cannot be read";
	}

	std::string line;
	const std::string DataStart = "GridCoordinates";
	while ( std::getline(oIn, line))
	{
        std::stringstream lineStream(line);
        std::string tok;

        std::string key;
		std::string val;
		std::getline(lineStream, key, mSeparator);
		std::getline(lineStream, val, mSeparator);
		
		if ( std::getline(lineStream, tok, mSeparator) )
		{
			std::cout << "Not critical error: Unknown data in line " << line << std::endl;
		}
		
		if ( key == "GridDelta" )
		{
			m_oGridDelta = atof_classiclocale(val.c_str());
		}
		if ( key == "SensorWidth" )
		{
			m_oSensorWidth = std::atoi(val.c_str());
		}
		if ( key == "SensorHeight" )
		{
			m_oSensorHeight = std::atoi(val.c_str());
		}
		if ( key == "CorrectionFactor" )
		{
			m_oCorrectionFactor = atof_classiclocale(val.c_str());
		}
		if ( key == "OrigX" )
		{
			//m_oOrigX = std::atoi(val.c_str());
		}
		if ( key == "OrigY" )
		{
			//m_oOrigY = std::atoi(val.c_str());
		}
		if ( key == "MaxDist" )
		{
			//m_oMaxDist = std::atoi(val.c_str());
		}
		if ( key == "TriangAngleRad" )
		{
			double oAngleRad = atof_classiclocale(val.c_str());
			setTriangulationAngle_rad(oAngleRad);
		}
		if ( line.find(DataStart) == 0 )  
		{
			break;
		}
	}

	if ( line.find(DataStart)!=0 )
	{

		oErrorMsg << "Data not found";
	}
	else
	{
		//lets' continue parsing 
		bool ok = gridFromCSV(m_oGrid, oIn);
		if ( !ok )
		{
			oErrorMsg << "Error during loading of the grid coordinates";
			setDefaults();
		}
	}
	assert(hasData() == oErrorMsg.str().empty() || !ASSERT_INVALID_CAMGRIDDATA);
	return oErrorMsg.str();

}


void CamGridData::gridToCsv(const tGridMap &p_rMap, std::ostream & out) const
{

	unsigned int nCorners, nRows, nColsMin, nColsMax;
	parseGridSize(nCorners, nRows, nColsMin, nColsMax);
	const std::string key("YAvg");
	out << key << mSeparator;
	for ( unsigned int i = 0; i < nColsMax; i++ )
	{
		out << std::setfill(mFillChar) << std::setw(2) << "x";
		out << std::setfill('0') << std::setw(2) << i << mSeparator;
		out << std::setfill(mFillChar) << std::setw(2) << "y";
		out << std::setfill('0') << std::setw(2) << i << mSeparator;
	}
	out << '\n';


	for (auto && rLine : p_rMap)
	{
		out << std::setfill(mFillChar) << std::setw(key.size()) << rLine.first << mSeparator;
		const auto & rCoords = rLine.second;
		for (size_t i=0; i < rLine.second.size(); ++i)
		{
			out << std::setfill(mFillChar) << std::setw(4) << rCoords[i].ScreenX << mSeparator
				<< std::setfill(mFillChar) << std::setw(4) << rCoords[i].ScreenY << mSeparator;
		}
		out << '\n';
	}
}

bool CamGridData::gridFromCSV(tGridMap &p_rMap, std::istream & p_In) const
{
	const std::string key("YAvg");

	unsigned int nColsMax = 0;
	std::string  header;
	std::getline(p_In, header);
	//check header
	{
		bool oHeaderOk(true);

		//std::cout << "header is " << header;
		std::stringstream   lineStream(header);
		std::string   tok;
		int i = 0;
		while ( std::getline(lineStream, tok, mSeparator) && oHeaderOk )
		{
			if ( i == 0 )
			{
				if ( tok != key )
				{
					std::cout << "Wrong header " << tok << std::endl;
					oHeaderOk = false;
				}
			}
			else
			{
				std::size_t f = tok.find_first_not_of(mWhitespace);
				if ( f == std::string::npos )
				{
					//this token is only whitespace, don't count it (could happen when the line ends with CRLF)
					i -= 1; 
				}
				else
				{
					char firstChar = tok[f];
					char expected = (i % 2 == 1 ? 'x' : 'y');
				
					if ( firstChar != expected )
					{
						oHeaderOk = false;
						std::cout << "\nUnexpected first char  '" << tok << "'" << " [" << f << " ]\n";
					}
				}
			}
			i++;
		}

		if ( i <= 0 ||  i % 2 != 1 )
		{
			std::cout << "\nNo pairs in " << header << "  (" << i << " tokens) " << std::endl;
			oHeaderOk = false;
		}
		else
		{
			nColsMax = (i - 1) / 2;
		}
		if ( !oHeaderOk )
		{
			std::cout << "Wrong format " << header << std::endl;
			return false;
		}
	} //end check header


	std::string line;
	try
	{
	
		while ( std::getline(p_In, line) )
		{
			//std::cout << "Read line " << line << std::endl;
			std::stringstream   lineStream(line);
			std::string   tok;
			int i = 0;
			std::int16_t oAvgY;
			std::vector<coord2D> oCorners;
			int oX, oY;
			while ( std::getline(lineStream, tok, mSeparator) )
			{
				//std::cout << tok;
				if ( i == 0 )
				{
					oAvgY = std::atoi(tok.c_str());
				}
				else
				{
					if ( i % 2 == 1 )
					{
						oX = atoi(tok.c_str());
					}
					else
					{
						oY = atoi(tok.c_str());
						oCorners.push_back(coord2D(oX, oY));
					}
				}
				i++;
				
			}
			//std::cout << std::endl;
			if ( oCorners.size() > nColsMax )
			{
				return false;
			}
			if ( oCorners.size() == 0 )
			{
				std::cout << "Data ended before EOF, created corner map with " << p_rMap.size() << " lines \n";
				return p_rMap.size() > 0;
			}
			p_rMap.insert(std::make_pair(oAvgY, oCorners));
		}
		return true;
	}
	catch ( std::exception &p_rException )
	{
		std::cout <<p_rException.what() << std::endl;
	}
		return false;

}

} // namespace system
} // namespace precitec
