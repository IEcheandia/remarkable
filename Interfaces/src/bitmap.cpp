/**
 * @file   
 * @brief		Schnittstelle zum Bitmap lesen und Schreiben; wrid verwendet, falls keine Kamera angeschlossen ist
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		KIR, hs
 * @date		20.05.10
 */

#include "common/bitmap.h"

#include <sstream>
#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/uio.h>

#include <fcntl.h>
#include <numeric>

namespace fileio
{

Bitmap::Bitmap( const std::string& filename )
{
	std::memset( &header_, 0, sizeof(RawBitmapHeader) );

#if defined __linux__
	rawFile = open( filename.c_str(), O_RDONLY);
#else
	rawFile = open( filename.c_str(), O_RDONLY | O_BINARY);
#endif

	if (rawFile != -1) // Fehler beim Lesen
	{
		read( rawFile, (char*)&header_, sizeof( RawBitmapHeader ) );
	}

	//rawFile = open( filename.c_str(), O_RDONLY | O_BINARY );

	// bitmap invalid setzen
	if( rawFile == -1 ) // fehler beim Oeffnen ODER Lesen
		header_.magicNumber = 0;
}

Bitmap::Bitmap( const std::string& filename, int width, int height, bool topDown )
//: file_( filename.c_str(), ios::out | ios::binary )
{
	//rawFile = open( filename.c_str(), O_RDONLY | O_BINARY );

	int padWidth = ((width+3)/4)*4, 				// Bildbreite falls noetig auf
									 				// ein vielfache von 4 erhoehen
		dSize 	 = padWidth*height,	 				// Datengroesse
		hSize	 = sizeof(RawBitmapHeader), 		// Headergroesse
		fSize	 = dSize + hSize + sizeof(RGB_)*256;	// Dateigroesse

	// Die Werte des Headers setzen fuer ein unkomprimiertes 8Bit Bitmap
	header_.magicNumber = BitmapMagicNumber;
	header_.fileSize	= fSize;
	header_.reserved1	= 0;
	header_.reserved2	= 0;
	header_.dataOffset	= hSize + sizeof(RGB_)*256;
	header_.headerSize	= 0x28;				// laut spec 0x28
	header_.width		= width;
	header_.height		= topDown?-height:height; // "top down" DIB laut spec, das Bild
												  // kann auf dem Kopf geschrieben werden
	header_.planes		= 1;				// wir haben nur eine Ebene
	header_.bitsPerPixel= 8;				// nur 8 bit pro pixel
	header_.compression = 0;				// keine kompression
	header_.dataSize	= dSize;
	header_.hResolution = (72*10000)/254; 	// umrechnung Pixel/m entspicht 72 dpi
	header_.vResolution = (72*10000)/254;
	header_.colors		= 256;
	header_.importantColors = 256;

	// Datei oeffnen
#if defined __linux__
	rawFile = open( filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
#else
	rawFile = open( filename.c_str(), O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IWRITE );
#endif

	// bitmap invalid setzen
	if( rawFile == -1 ) {
		header_.magicNumber = 0;
		std::cerr << "\nWARNING:\t" << __FUNCTION__ << ": Could not open file '" << filename << "' (open returned -1).\n";
	}
}

Bitmap::~Bitmap()
{
	//file_.close();
	if (rawFile != -1)
	{
		close(rawFile);
	}
}

bool Bitmap::load( unsigned char *data ) const
{
	if (header_.bitsPerPixel != 8) {
		std::cerr << "\nWARNING:\t" << __FUNCTION__ << ": Can only load greyscale bitmaps (8 bit per pixel). Encountered " << header_.bitsPerPixel << " per pixel image header.\n";
		return false;
	}

	if (rawFile < 0)
		return false;

	int w = width(),
		h = height(),
		alignedWidth = ((w+3)/4)*4,
		padding = alignedWidth - w;

	// Setze den Dateizeiger auf den Anfang der Bilddaten
	//file_.seekg( dataOffset(), ios_base::beg );
	lseek( rawFile, dataOffset(), SEEK_SET );

	// Wenn das Bild schon auf dem Kopf steht, auf einmal einlesen
	if( topDown() & (padding == 0) )
	{
		read( rawFile, data, dataSize() );
	}
	else // sonst die Zeilen umdrehen
	{
		// Fuer jede Bildzeile
		for( int y=h-1; y>=0; --y )
		{
			//file_.read( &data[y*w], w );	  // Zeile lesen
			//file_.seekg( padding, ios::cur ); // ueberspinge die padding Bytes
			read( rawFile, &data[y*w], w );
			lseek( rawFile, padding, SEEK_CUR );
		}
	}

	return true; // TODO
}


bool Bitmap::load( unsigned char *data, std::vector<unsigned char>& p_rExtraData ) const
{
	if (rawFile < 0)
		return false;

	const auto oExtraLength	= header_.reserved1 == 1971 ? header_.reserved2 : 0;
	p_rExtraData.resize(oExtraLength);

	// Setze den Dateizeiger auf den Anfang der extra daten
	lseek( rawFile, dataOffset() - oExtraLength, SEEK_SET );

	// Lese extra daten
	read( rawFile, p_rExtraData.data(), oExtraLength );

	return load(data);
}


bool Bitmap::save(const unsigned char *data)
{
    if (rawFile < 0)
    {
        return false;
    }

    if (topDown())
    {
        return saveTopDown(data, {});
    }
    else
    {
        return saveWrite(data);
    }
    return false;
} // save

bool Bitmap::saveWrite(const unsigned char* data)
{
	if (writeHeader() == false) {
		return false;
	} // if

	if (writePalette() == false) {
		return false;
	} // if

	if (writeData(data) == false) {
		return false;
	} // if
	
	return true; // see http://www.qnx.com/developers/docs/6.3.0SP3/neutrino/lib_ref/w/write.html
}

bool Bitmap::save(const unsigned char *data, const std::vector<unsigned char>& p_rAdditionalData)
{
    if (rawFile < 0)
    {
        return false;
    }
	// adjust header to store extra data

	header_.fileSize	+=	int(p_rAdditionalData.size());
	header_.reserved1	=	p_rAdditionalData.size() != 0 ? 1971 : 0;
	header_.reserved2	=	(unsigned short)(p_rAdditionalData.size());
	header_.dataOffset	+=	int(p_rAdditionalData.size());

    if (topDown())
    {
        if (!saveTopDown(data, p_rAdditionalData))
        {
            return false;
        }
    }
    else
    {
        if (!saveWrite(data, p_rAdditionalData))
        {
            return false;
        }
    }
	
	// restore header. bmp could be saved again without extra data.

	header_.fileSize	-=	int(p_rAdditionalData.size());
	header_.reserved1	=	0;
	header_.reserved2	=	0;
	header_.dataOffset	-=	int(p_rAdditionalData.size());

	return true; // see http://www.qnx.com/developers/docs/6.3.0SP3/neutrino/lib_ref/w/write.html
}

bool Bitmap::saveWrite(const unsigned char* data, const std::vector<unsigned char>& p_rAdditionalData)
{
	if (writeHeader() == false) {
		return false;
	} // if

	if (writePalette() == false) {
		return false;
	} // if

	if (writeAddData(p_rAdditionalData) == false) {
		return false;
	} // if

	if (writeData(data) == false) {
		return false;
	} // if

	return true; // see http://www.qnx.com/developers/docs/6.3.0SP3/neutrino/lib_ref/w/write.html
}


bool Bitmap::writeHeader() {
	if (rawFile < 0)
		return false;

	bool writeOk = true;

	// Dateizeiger an den Anfang setzen
	//file_.seekg( 0, ios::beg );
	lseek( rawFile, 0, SEEK_SET );

	// Header schreiben
	//file_.write( (char*)&header_, sizeof(RawBitmapHeader) );
	writeOk &= write( rawFile, (char*)&header_, sizeof(RawBitmapHeader) ) == sizeof(RawBitmapHeader);

	return writeOk;
} // writeHeader


bool Bitmap::writePalette() {
	if (rawFile < 0)
		return false;

	bool writeOk = true;

	// Palette initialisieren
    initPalette();

	// Palette schreiben
	//file_.write( (char*)&(palette_[0]), sizeof(RGB_)*256 );
	writeOk &= write( rawFile, (char*)&(palette_[0]), sizeof(RGB_)*256 ) == sizeof(RGB_)*256;

	return writeOk;
} // writePalette


bool Bitmap::writeData(const unsigned char *data) {
	if (rawFile < 0)
		return false;

	bool writeOk = true;

	if ( (width() * height() ) == 0) // empty image
	{
		return true;
	}
	int w = width(),
		h = height(),
		alignedWidth = ((w+3)/4)*4,
		padding = alignedWidth - w;
	// Schreibzeiger auf Datenanfang setzen
	//file_.seekg( dataOffset(), ios::beg );
	lseek( rawFile, dataOffset(), SEEK_SET );

	// Wenn wir "auf dem Kopf" schreiben und kein padding haben,
	// koennen wir deutlich schneller, in einem Block schreiben.
	if( topDown() & (padding == 0) )
	{
		writeOk &= write( rawFile, data, dataSize() ) == dataSize();
	}
	else // sonst Zeilenweise rueckwaerts schreiben
	{	 // Da angenommen werden kann, dass Speicher immer 4 Byte Alignment hat, ist
		 // der erste Zugriff (eigentlich nicht mehr im zulaessigen Speicherbereich )
		 // trotzdem ok. Das ist nicht schoen aber selten.
		for( int y=h-1; y>=0; --y )
		{
			//file_.write( &data[y*w], alignedWidth );	  // Zeile schreiben
			writeOk &= write( rawFile, &data[y*w], alignedWidth ) == alignedWidth;
		}
	}

	if (writeOk == false) {
		std::cerr << "\nWARNING:\t" << __FUNCTION__ << ": At least one 'write' call could not write all bytes required.\n";
	}

	return writeOk;
} // writeData


bool Bitmap::writeAddData(const std::vector<unsigned char>& p_rAdditionalData) {
	if (rawFile < 0)
		return false;

	bool writeOk = true;

	// Schreibzeiger auf Datenanfang Zusatz-Daten setzen
	lseek( rawFile, dataOffset() - p_rAdditionalData.size(), SEEK_SET );

	// Zusatz-Daten schreiben
	writeOk &= write( rawFile, (char*)p_rAdditionalData.data(), p_rAdditionalData.size() ) == (int)p_rAdditionalData.size();
	
	return writeOk;
} // writeAddData

bool Bitmap::saveTopDown(const unsigned char* data, const std::vector<unsigned char>& p_rAdditionalData)
{
    std::vector<iovec> iovec;
    iovec.reserve(4);

    prepareWriteHeader(iovec);
    prepareWritePalette(iovec);
    prepareWriteAddData(iovec, p_rAdditionalData);
    prepareWriteData(iovec, data);

    seekToBeginOfFile();
    return writeVectors(iovec);
}

void Bitmap::seekToBeginOfFile()
{
    if (m_seekToBegin)
    {
        // save was called before, seek back to begin of file
        lseek(rawFile, 0, SEEK_SET);
    }
    m_seekToBegin = true;
}

bool Bitmap::writeVectors(const std::vector<iovec>& iov)
{
    const auto result{writev(rawFile, iov.data(), iov.size()) == ssize_t(std::accumulate(iov.begin(), iov.end(), std::size_t{0u}, [] (const auto& a, const auto& b) { return a + b.iov_len; }))};
    if (!result)
    {
        std::cerr << "\nWARNING:\t" << __FUNCTION__ << ": At least one 'write' call could not write all bytes required.\n";
    }
    return result;
}

void Bitmap::prepareWriteHeader(std::vector<iovec>& iov)
{
    iovec vec;
    vec.iov_base = (void*)&header_;
    vec.iov_len = sizeof(RawBitmapHeader);
    iov.emplace_back(std::move(vec));
}

void Bitmap::prepareWritePalette(std::vector<iovec>& iov)
{
    initPalette();
    iovec vec;
    vec.iov_base = (void*)&(palette_[0]);
    vec.iov_len = sizeof(RGB_)*256;
    iov.emplace_back(std::move(vec));
}

void Bitmap::initPalette()
{
	for( int i=0; i<256; i++)
		palette_[i] = RGB_(i);
}

void Bitmap::prepareWriteData(std::vector<iovec>& iov, const unsigned char* data)
{
    if ( (width() * height() ) == 0) // empty image
    {
        return;
    }
    if (!topDown())
    {
        return;
    }

    iovec vec;
    vec.iov_base = (void*)data;
    vec.iov_len = dataSize();
    iov.emplace_back(std::move(vec));
}

void Bitmap::prepareWriteAddData(std::vector<iovec>& iov, const std::vector<unsigned char>& additionalData)
{
    if (additionalData.empty())
    {
        return;
    }
    iovec vec;
    vec.iov_base = (void*)additionalData.data();
    vec.iov_len = additionalData.size();
    iov.emplace_back(std::move(vec));
}

std::ostream& operator<<( std::ostream& os, const Bitmap& bmpHdr )
{
	os  << " valid=" << bmpHdr.isValid()
		<< " topDown=" << bmpHdr.topDown()
		<< " fileSize=" << bmpHdr.fileSize()
		<< " dataOffset=" << bmpHdr.dataOffset()
		<< " headerSize=" << bmpHdr.headerSize()
		<< " width=" << bmpHdr.width()
		<< " height=" << bmpHdr.height()
		<< " planes=" << bmpHdr.planes()
		<< " bpp=" << bmpHdr.bitsPerPixel()
		<< " compression=" << bmpHdr.compression()
		<< " dataSize=" << bmpHdr.dataSize()
		<< " colors=" << bmpHdr.colors()
		<< " importantColors=" << bmpHdr.importantColors();
	return os;
}

} // namespace fileio
