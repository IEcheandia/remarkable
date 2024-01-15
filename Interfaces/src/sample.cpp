/**
 * @file   
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		HS
 * @date		2014
 * @brief		Sample file writing. Used in video recorder writer.
*/

#include "common/sample.h"
#include "module/moduleLogger.h"

#include "Poco/Path.h"


using namespace Poco;
using namespace precitec;
namespace fileio {

/*static*/ const std::array<char, 3>	Sample::m_oSampleExtensionAscii		= {{ 0X53, 0X4D, 0X50 }};
/*static*/ const short					Sample::m_oSampleMagicNumber		= 1971;
/*static*/ const short					Sample::m_oVersion					= 1;
/*static*/ const char*					Sample::m_oSampleExtension			= "smp";

Sample::Sample(const std::string& p_rFilePath) : m_oFilePath( p_rFilePath ) {
	if (Path{ m_oFilePath }.getExtension() != m_oSampleExtension) {
		wmLog(eDebug, "%s: Extension (%s) invalid.\n", __FUNCTION__, Path{ m_oFilePath }.getExtension().c_str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", m_oFilePath.c_str(), __FUNCTION__);
	} // if

	// test if file exists

	m_oFileStream.open(m_oFilePath); // open stream for reading
	const auto	oFileExisted	=	m_oFileStream.good(); 
	m_oFileStream.close();

	// file did not exist. create it and write file header.

	if (oFileExisted == false) {
		m_oFileStream.open(m_oFilePath, std::ios::out | std::ios::binary); // open stream for writing
		
		if (checkStreamState("open") == false) {
			return;
		} // if

		writeFileHeader();
	} // if

	// file did exist. Open it and read file header.

	else { // if file exists already, read header
		m_oFileStream.open(m_oFilePath,  std::ios::in | std::ios::out | std::ios::binary); // open stream for modification

		m_oFileStream.read(reinterpret_cast<char*>(&m_oFileHeader), sizeof(SampleHeader));

		if (checkStreamState("read") == false) {
			return;
		} // if

		if (isValid() == false) {
			wmLog(eDebug, "%s: Header not valid.\n", __FUNCTION__);
			wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", m_oFilePath.c_str(), __FUNCTION__);

			return;
		} // if			
	} // if

} // Sample



bool Sample::appendDataBlock(int p_oSensorId, const char* p_pData, std::size_t p_oSize) {
	m_oDataHeader.m_oSampleCount	=	p_oSize;
	m_oDataHeader.m_oSensorId		=	p_oSensorId;

	bool	oSuccess	= false;
	
	oSuccess	|=	incNbSignals();
	oSuccess	|=	writeDataHeader();
	oSuccess	|=	writeDataBlock(p_pData);
	
	return oSuccess;
} // save

bool Sample::readAllData( SampleDataHolder & _dataHolder )
{
	bool	oSuccess	= false;

	if( m_oFileStream )
	{
		//get length of file
		m_oFileStream.seekg( 0, m_oFileStream.end );
		int length = m_oFileStream.tellg();
		m_oFileStream.seekg( 0, m_oFileStream.beg );
		char * buffer = new char [length];
		m_oFileStream.read( buffer, length );
		char* ptr = buffer;
		/// 9 = sizeof('SMP') + sizeof(preciKey) + sizeof(version) + sizeof(countSignals)
		int minSize = /*sizeof('SMP')*/ 3 + sizeof(short)+sizeof(short)+sizeof(short);
		if (length >= minSize)
		{
			oSuccess = true;
			//first 3 bytes = 'smp'
			ptr += 3;
			short preciKey = *(short*)ptr;
			ptr += sizeof(short);
			short version = *(short*)ptr;
			version=version;
			ptr += sizeof(short);
			short countSignals = *(short*)ptr;
			ptr += sizeof(short);
			if (preciKey == m_oSampleMagicNumber)
			{
				for (int i = 0; i < countSignals; i++)
				{
					SampleDataHolder::OneSensorData oneSensorData;
					int sensorId = *(int*)ptr;
					ptr += sizeof(int);
					int sampleCount = *(int*)ptr;
					ptr += sizeof(int);
					oneSensorData.dataVector.reserve(sampleCount);
					oneSensorData.sensorID=sensorId;
					for (int i = 0; i < sampleCount; i++)
					{
						int sampleValue = *(int*)ptr;
						ptr += sizeof(int);
						oneSensorData.dataVector.push_back(sampleValue);
					}

					_dataHolder.allData.push_back(oneSensorData);
				}
			}
		}
		if (buffer != nullptr)
		{
			delete[] buffer;
		}
	}

	return oSuccess;
}


bool Sample::writeFileHeader() {	
	
	m_oFileStream.seekp(0, std::ios::beg);
	if (checkStreamState("seekp") == false) {
		return false;
	} // if

	m_oFileStream.write(reinterpret_cast<const char*>(&m_oFileHeader), sizeof(m_oFileHeader));
	
	return checkStreamState("write");
} // writeFileHeader



bool Sample::incNbSignals() {
	++m_oFileHeader.m_oNbSignals;

	return writeFileHeader();
} // incNbSignals



bool Sample::writeDataHeader() {	
	m_oFileStream.seekp(0, std::ios::end);
	if (checkStreamState("seekp") == false) {
		return false;
	} // if

	m_oFileStream.write(reinterpret_cast<const char*>(&m_oDataHeader), sizeof(DataHeader));
	
	return checkStreamState("write");
} // writeDataHeader



bool Sample::writeDataBlock(const char* p_pData) {
	m_oFileStream.seekp(0, std::ios::end); // write at beginning of the stream
	
	if (checkStreamState("seekp") == false) {
		return false;
	} // if

	m_oFileStream.write(p_pData, m_oDataHeader.m_oSampleCount * sizeof(int));

	return checkStreamState("write");
} // writeData


inline bool Sample::checkStreamState(const std::string& p_rLastStreamOp) const {
	if (m_oFileStream.good() == false) {
		wmLog(eWarning, "%s: Filestream not good after (%s).\n", __FUNCTION__, p_rLastStreamOp.c_str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", m_oFilePath.c_str(), __FUNCTION__);

		return false;
	} // if
	return true;
} // checkStreamState



Sample::SampleHeader::SampleHeader() 
:	m_oExtensionAscii		( Sample::m_oSampleExtensionAscii ), // GCC Bug 53361
	m_oMagicNumber			{ Sample::m_oSampleMagicNumber },
	m_oVersion				{ Sample::m_oVersion },
	m_oNbSignals			{ 0 }
{}



Sample::DataHeader::DataHeader(int p_oSensorId , unsigned int p_oSampleCount) 
:	m_oSensorId			{ p_oSensorId },
	m_oSampleCount		{ p_oSampleCount }
{}


} // namespace fileio
