/**
 * @file   
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		HS
 * @date		2014, 2017
 * @brief		Sample file writing. Used in video recorder writer.
 * @detail		GG: Added Sample file reading. Used in Grabber when used as file grabber.
*/


#ifndef SAMPLE_H_INCLUDED_20140710
#define SAMPLE_H_INCLUDED_20140710

#include "InterfacesManifest.h"
#include "sampleDataHolder.h"

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <ostream>

namespace fileio {

/**
  * @brief	Sample file writing. Used in video recorder writer.
  * @detail	File structure consists of a main file header followed by data blocks, each with a small block header:
  *			[char] extension [char] extension [char] extension [int16] magic number [int16] version
  *			[int16] count sensor signals 
  *			{[DataHeader] block header [char[sample count]] data block}  
  */
class INTERFACES_API Sample {
public:
	static const std::array<char, 3>	m_oSampleExtensionAscii;	/// file extension in ASCII (SMP)
	static const short					m_oSampleMagicNumber;		/// magic number (1971)
	static const short					m_oVersion;					/// version number starting from 1
	static const char*					m_oSampleExtension;			/// file extension (smp). See 'videorecorder/literals.h'

	/**
	  * @brief	Modifies an existing smp file or creates a new smp file
	  * @param	p_rFilePath			Full path to file to be opended or created if not existing
	  */
	Sample(const std::string& p_rFilePath);

	/**
	  * @brief	Appends a data block including data header.
	  * @param	p_oSensorId		Sensor id of sample array.		
	  * @param	p_pData			Input data block as byte array.
	  * @param	p_oSize			Size of sample array. NB: number of int, not size as number of bytes.
	  * @return	If all file out operations suceeded.			
	  */
	bool appendDataBlock(int p_oSensorId, const char* p_pData, std::size_t p_oSize);

	/**
	  * @brief	Reads all data from the file.
	  * @param	p_pData		all	Data in file.
	  * @return	If all file in operations suceeded.
	  */
	bool readAllData(SampleDataHolder & _dataHolder);

private:

	/**
	  * @brief	Checks if file magic number and version are correct.
	  * @return	If file magic number and version are correct.
	  */
	bool isValid() const { return m_oFileHeader.m_oMagicNumber == m_oSampleMagicNumber && m_oFileHeader.m_oVersion <= m_oVersion; }

	/**
	  * @brief	Writes the file header.
	  * @return	If all file out operations suceeded.
	  */
	bool writeFileHeader();

	/**
	  * @brief	 Increments number of signals in file header and writes the header.
	  * @return	If all file out operations suceeded.
	  */
	bool incNbSignals();

	/**
	  * @brief	Writes the data header.
	  * @return	If all file out operations suceeded.
	  */
	bool writeDataHeader();

	/**
	  * @brief	Writes the data block. Only called by appendDataBlock().
	  * @param	p_pData			Data block (int sample(s)).
	  * @return	If all file out operations suceeded.
	  */
	bool writeDataBlock(const char* p_pData);

	/**
	  * @brief	Checks the stream state and logs a warning if stream not good.
	  * @param	p_rLastStreamOp			Last operation executed on stream. For log detail.
	  * @return	If stream is in a good state.
	  */
	bool checkStreamState(const std::string& p_rLastStreamOp) const;


#pragma pack(1) // 1-byte alligned block

	/**
	  * @brief	Sample file header
	  * @detail	[char] extension [char] extension [char] extension [int16] magic number [int16] version [int16] count sensor signals 
	  */
	class SampleHeader {
	public:
		SampleHeader();

		const std::array<char, 3>	m_oExtensionAscii;			// extension of file format
		const short 				m_oMagicNumber;				// magic number of file format
		const short 				m_oVersion;					// version of file format
		short 						m_oNbSignals;				// number of signals in data block
	}; // class SampleHeader

	/**
	  * @brief	Data header
	  * @detail	[int32] sensor id 	[int32] sample count
	  */
	class DataHeader {
	public:
		DataHeader(int p_oSensorId = 0, unsigned int p_oSampleCount = 0);

		int 		m_oSensorId;		// sensor id
		unsigned int			m_oSampleCount;		// number of samples
	}; // class DataHeader

#pragma pack() // 1-byte alligned block

	const std::string	m_oFilePath;	// Full  path to sample file
	std::fstream 		m_oFileStream;	// In-out file stream
	SampleHeader		m_oFileHeader;	// Sample file header
	DataHeader			m_oDataHeader;	// Data block header
}; // class Sample




} // namespace fileio

#endif /*SAMPLE_H_INCLUDED_20140710*/
