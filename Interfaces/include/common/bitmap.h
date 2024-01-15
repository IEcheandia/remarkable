/**
 * @file
 * @brief		Schnittstelle zum Bitmap lesen und Schreiben, verwendet falls keine Kamera angeschlossen ist
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		KIR, hs
 * @date		20.05.10
 */


#ifndef BITMAPHEADER_H_
#define BITMAPHEADER_H_

#include "InterfacesManifest.h"

#include <string>
#include <vector>
#include <cstdlib> // abs

//#include <stdio.h>

struct iovec;

namespace fileio
{

const short BitmapMagicNumber = 19778;

/** Klasse zum Lesen und Schreiben von unkomprimierten 8-Bit Bitmaps
 *  Die Klassen funktioniert mom. nur auf little endian Architekturen korrekt.
 */
class INTERFACES_API Bitmap
{
public:
	/** Bitmap von Datei
	 *  Oeffnet eine Datei mit dem Namen filename und liesst den Header aus.
	 *  Die Daten werden dabei noch nicht geladen.
	 * \param filename Dateiname des zu oeffnenden Bitmaps. z.B. images/test.bmp
	 */
	Bitmap( const std::string& filename );

	/** Graustufen Bitmap
	 * Erzeugt einen Bitmapheader fuer ein 8-Bit Graustufen Bitmap ohne Kompression.
	 * Das Bitmap wird dabei noch nicht geschrieben.
	 * \param filename Dateiname des zu speichernden Bitmaps
	 * \param width Bildbreite in Pixel (muss ein vielfaches von 4 sein!)
	 * \param height Bildhoehe in Pixel
	 * \param top down Flag. true wenn die Bilddaten auf dem Kopfstehen false sonst.
	 * default: true
	 */
	 Bitmap( const std::string& filename, int width, int height, bool topDown = true );

	/** Destruktor
	 * schliesst die geoeffneten Resourcen
	 */
	~Bitmap();

	/** Prueft die Magic Number 'BM'
	 * \return true wenn die Magicnumber ueberinstimmt mit 'BM', false sonst
	 */
	bool isValid() const { return header_.magicNumber == BitmapMagicNumber; }

	/** Dateigroesse
	 * \return Liefert die Dateigroesse des BItmaps in Bytes
	 */
	int	fileSize() const { return header_.fileSize; }

	/** Offset der Bilddaten
	 * \return Offset vom Dateianfang bis zu den Bilddaten
	 */
	int dataOffset() const { return header_.dataOffset; }


	/** Gueltige Groesse?
	 *
	 */
	bool validSize() const { return ( (width() > 0) && (height() > 0) ); }

	/** Groesse der Bitmapheaders
	 * \return Groesse des Bitmapheaders in Byte
	 */
	int headerSize() const { return header_.headerSize; }

	/** Bildbreite
	 * \return Bildbreite in Pixel
	 */
	int width() const { return header_.width; }

	/** Bildhoehe
	 * \return Bildhoehe in Pixel
	 */
	int height() const { return std::abs( header_.height ); }

	/** Bildebenen
	 * \return Anzahl der Bildebenen
	 */
	short planes() const { return header_.planes; }

	/** Bits Pro Pixel
	 * \return Bits pro Pixel
	 */
	short bitsPerPixel() const { return header_.bitsPerPixel; }

	/** Kompression
	 * \return Kompressionsart
	 */
	int compression() const { return header_.compression; }

	/** Groesse der Bitmapdaten
	 * \return Groesse der Bitmapdaten in Bytes, muss ein Vielfaches von 4 sein
	 */
	int dataSize() const { return header_.dataSize; }

	/** Farben
	 * \return Anzahl der Farben
	 */
	int colors() const { return header_.colors; }

	/** Wichtige Farben
	 * \return Anzahl der wichtigen Farben
	 */
	int importantColors() const { return header_.importantColors; }

	/** Top Down Flag
	 * \return Liefert true wenn die Bilddaten horizontal gespiegelt sind,
	 * false sonst.
	 */
	bool topDown() const { return header_.height < 0; }

	/** Laden der Bitmapdaten
	 * Liest die Daten von der Festplatte in den Speicher hinter data.
	 * Es wird von der Klasse keine Speicher angelegt, das muss vom Aufrufer
	 * erledigt werden!
	 * \param data Zeiger auf Speicher der mindestens dataSize() gross ist
	 */
	bool load( unsigned char *data ) const;

	/** Laden der Bitmapdaten
	 * Liest die Daten von der Festplatte in den Speicher hinter data.
	 * Es wird von der Klasse keine Speicher angelegt, das muss vom Aufrufer
	 * erledigt werden!
	 * \param	data				Zeiger auf Speicher der mindestens dataSize() gross ist
	 * @param	p_rAdditionalData	Laedt zusaetzliche Daten, die vor das Pixelarray geschrieben wurden, falls vorhanden. Passt die Groesse des Vektors an.
	 *								Siehe save(const unsigned char *data, const std::vector<unsigned char>&) const;
	 */
	bool load( unsigned char *data, std::vector<unsigned char>& p_rExtraData ) const;

	/** Schrieben der Bitmapdaten
	 * Schreibt die Daten hinter data auf die Festplatte.
	 * @param data Zeiger auf die zu Speichernden Bilddaten. Der Speicher muss mindestens dataSize() gross sein.
	 */
	bool save( const unsigned char *data );

	/** Schrieben der Bitmapdaten
	 * Schreibt die Daten hinter data auf die Festplatte.
	 * @param	data				Zeiger auf die zu Speichernden Bilddaten. Der Speicher muss mindestens dataSize() gross sein.
	 * @param	p_rAdditionalData	Zusatz-Daten, die vor das Pixelarray geschrieben werden. fileSize und dataOffset werden entsprechend angepasst.
	 *								reserved1 speichert einen unique key (1971), reserved2 die groesse in bytes der Daten.
	 *								Format der Version 0:
	 *
	 *								Byte 00 - 01: Version (unsigned short)
	 *								Byte 02 - 03: HW-ROI X (unsigned short)
	 *								Byte 04 - 05: HW-ROI Y (unsigned short)
	 *								Byte 06 - 07: Bildnummer (unsigned short)
	 */
	bool save(const unsigned char *data, const std::vector<unsigned char>& p_rAdditionalData);

private:
	bool writeHeader();
	bool writePalette();
	bool writeData(const unsigned char* data);
	bool writeAddData(const std::vector<unsigned char>& p_rAdditionalData);
    bool saveWrite(const unsigned char* data);
    bool saveWrite(const unsigned char* data, const std::vector<unsigned char>& p_rAdditionalData);
    bool saveTopDown(const unsigned char* data, const std::vector<unsigned char>& p_rAdditionalData);
    void prepareWriteHeader(std::vector<iovec>& iov);
    void prepareWritePalette(std::vector<iovec>& iov);
    void initPalette();
    void prepareWriteData(std::vector<iovec>& iov, const unsigned char* data);
    void prepareWriteAddData(std::vector<iovec>& iov, const std::vector<unsigned char>& additionalData);
    void seekToBeginOfFile();
    bool writeVectors(const std::vector<iovec>& iov);

	// sicherstellen dass der Compiler fuer diesen Abschnitt eine
	// 1-Byte-Ausrichtung verwendet. Das ist Compilerabhaengig.
	// Ohne diese Ausrichtung kann der Header nicht am Stueck von der Platte
	// gelesen werden.
#pragma pack(1)

	// Struktur um den Bitmapheader auszulesen
	struct RawBitmapHeader
	{
		short 	magicNumber;				// Identifiziert das Bitmap, sollte 'BM' sein
		int		fileSize;					// Dateigroesse in Bytes
		unsigned short		reserved1;		// 0 oder unique key (1971)
		unsigned short		reserved2;		// 0 oder die groesse der Zusatz-Daten in bytes
		int		dataOffset;					// Offset vom Dateianfang zum Anfang der Bitmapdaten
		int		headerSize;					// Laenge des Bitmapheaders zur Beschreibung der Farben, Compression, etc.
		int		width;						// Bildbreite in Pixel
		int		height;						// Bildhoehe in Pixel
		short	planes;						// Anzahl der Bildebenen
		short	bitsPerPixel;				// Bits pro Pixel (1,4,8,16,24,32)
		int		compression;				// Kompressionsart 0-keine, 1-RLE 8bit, 2-RLE 4bit, 3-Bitfields
		int		dataSize;					// Groesse der Bitmapdaten, muss auf die naechsten 4Byte Aufgerundet werden
		int		hResolution;				// Horizontale Aufloesung in Pixel pro Meter
		int		vResolution;				// Vertikale Aufloesung in Pixel pro Meter
		int		colors;						// Anzahl der Farben, bei 8-bit per Pixel waere es z.B. 256
		int		importantColors;			// Anzahl der wichtigen Farben. Ist gleich colors wenn ale Farben benutzt werden
	};

	/// Struktur fuer Paletteneintraege
	struct INTERFACES_API RGB_
	{
		// blau, gruen, rot, filler
		char b, g, r, f;

		RGB_() : b(0), g(0), r(0), f(0) {}
		RGB_(char i ) : b(i), g(i), r(i), f(0) {}
	};

#pragma pack()  // Ende des 1-byte ausgerichteten Blocks

	RawBitmapHeader header_;		// RawHeader
	//std::fstream 	file_;			// Dateistream zum Lesen und Schreiben
	RGB_			palette_[256];	// Farbpalette fuer 8 bit
	int				rawFile;		// Filedeskriptor
	bool m_seekToBegin{false};
};

/// Konsolenausgabe
INTERFACES_API std::ostream& operator<<( std::ostream& os, const Bitmap& bmpHdr );


} // namespace fileio

#endif /*BITMAPHEADER_H_*/
