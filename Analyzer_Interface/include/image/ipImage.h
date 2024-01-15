#ifndef IP_IMAGE_H_
#define IP_IMAGE_H_

#include <iostream>
#include <iomanip>

#include "geo/size.h"
#include "geo/rect.h"
#include "geo/range.h"
#include "geo/point.h"

#include "memory/smartPtr.h"
#include "image/ipSignal.h"

namespace precitec
{
namespace image
{
	typedef geo2d::Size 	Size2d;

	template <class ValueT>
	class TImage : public TSignal<ValueT>
	{
	public:
		/// Typ der EinzelDaten/Pixel (Typ in das aktuelle Scope ziehen)
		typedef ValueT 						Value;

		/// Typ der EinzelDaten/Pixel ???? ist dieser Typ noetig wg Value
		typedef ValueT						PixelType;

		///	Pointer auf Daten
		typedef Value 		  		 *Pointer;
		///	const-Pointer auf Daten
		typedef Value const  		 *ConstPointer;

		typedef typename system::TSmartPointer<ValueT>::ShArrayPtr ShArrayPtr;

		enum Type {
			BaseImage =-1, // (Pseudo)Abstrakt; Reale BildTypen fangen bei 0 an, wg FabrikTabelle[TType]
			BlockImage, // Std-Bild, monolithischer Speicherblock, Zugriff mit y*pitch+x
			LineImage,	 // Bild:Zeilen ggf koennen im Speicher: Zugriff ueber lines_[y]+x
			TNumTypes
		};	public:

protected:
		// TImage ist abstrakt
		// nun die Konstruktoren
		/// Bild der Groesse 0, ohne Speicher
		// hier landet letztendlich der Konstruktor
		// TSignal in AnalyzerInterface/include/image/ipSignal.h
	    TImage() : TSignal<ValueT>(), size_() {}

		/// copy-Construktor (SHALLOW)
		TImage(const TImage& rhs) : TSignal<ValueT>(rhs), size_(rhs.size_) {}

		/// neues Bild der Groesse size wird angelegt
		//TSignal<byte>(width*height)
	  TImage(Size2d newSize) : TSignal<ValueT>(newSize.area()),	size_(newSize)
	  {
	//      vigra_precondition((newSize.area() >= 0),
	//           "Image::Image(Size2d size): "
	//           "width and height must be >= 0.\n");
	  }

		/// CTor mit externem nacktem Speicher
		TImage(Pointer data, Size2d newSize): TSignal<ValueT>(data, newSize.area()), size_(newSize)
	  {
	//      vigra_precondition((newSize.area() >= 0),
	//           "Image::Image(Size2d size): "
	//           "width and height must be >= 0.\n");
	  }

		TImage(ShArrayPtr &dataPtr, Size2d size)
				: TSignal<ValueT>(dataPtr, size.area()), size_(size) {}

		TImage(system::ShMemPtr<ValueT> &dataPtr, Size2d size)
				: TSignal<ValueT>(dataPtr, size.area()), size_(size) {}

		/// subRoi CTor Original-Daten werden referenziert!! nicht kopiert
		TImage(TImage const& rhs, geo2d::Range const& r, Size2d s )
		: TSignal<ValueT>(*((TSignal<ValueT>*)(&rhs)), r), size_(s) {
		}

		/// Deserialisierungs-CTor
		TImage(system::message::MessageBuffer const& buffer)
		: TSignal<ValueT>(buffer), 	size_(system::Serializable::deMarshal<Size2d>(buffer)) {
			//std::cout << "TImage::deser size_: " << size_ << std::endl;
			//std::cout << "currentPos(size_): " << buffer.currentPos() << std::endl;
		}

	public:

		virtual ~TImage() 	{}

		/// Bild kopieren ggf. naxh resize

		TImage & operator=(const TImage & rhs) { TImage tmp(rhs); swap(tmp);	return *this;	}
		///	Bild neu dimensionieren; neue Elemente mit Wert fuellen
		void resizeFill(Size2d newSize, Value const& value)	{ 
			if (newSize != size_) { 
				TSignal<ValueT>::resizeFill(newSize.area(), value);
				size_ = newSize; 
			}
			else {
				TSignal<ValueT>::fill(value);
			}
		}
		/// Bild neu dimensionieren
		void resize(Size2d const& newSize) {
			if (newSize != size_) {
				TSignal<ValueT>::resize(newSize.area());
				size_ = newSize;
			}
		}
		/// Bild aus anderem Bild dimensinieren + kopieren
		void resizeCopy(const TImage & rhs)	{	resizeCopy(rhs.size(), rhs.begin()); }
		/// interne Daten tauschen
		void swap(TImage & rhs ) { using std::swap; TSignal<ValueT>::swap(rhs); swap(size_, rhs.size_); }
		/// Bildbreite
		int width() const {return size().width; }
		/// Bildhoehe
		int height() const { return size().height; }
		/// Bildgroesse x, y
		Size2d size() const	{ return size_; }
		/// isValid
		bool isValid() const
		{
			return ( ( TSignal<ValueT>::isValid() ) && (size_.width > 0) && (size_.height > 0) );
		}

	public:
		/* Der 'Serialisierungs' Block
		 * Die Enum in der Basisklasse wird von der type()-Funktion der abgeleiteten Klassen zurueckgeben
		 * Die Basisklasse muss nicht viel tun, es reicht wenn sie ihre Parameter marshalled (hier size_)
		 * Die abgleiteten Klassen muessen je eine statische create-Funktion bereistellen
		 * Eine Tabelle der create-Funktionen indiziert mit den Bildtypen wird statisch
		 * In der TImage-create-Funktion angelegt
		 * Weiterhin brauchen die abgeleiteten Klassen einen CTor mit message::MessageBuffer-Parameter in dem
		 * letztlich nur die serialize-Funktion aufgerufen wird
		 */

		virtual int type() const { return BaseImage; }
		void serialize( system::message::MessageBuffer &buffer ) const {
			TSignal<ValueT>::serialize(buffer);
			system::Serializable::marshal(buffer, size_);
			//std::cout << "currentPos(size_): " << buffer.currentPos() << std::endl;
		}
		void deserialize( system::message::MessageBuffer const&buffer ) {	TImage tmp(buffer); swap(tmp); }

		static TImage* create(int type, system::message::MessageBuffer const& buffer );

	public:
	


	  // bottom up <-> top down
	  void turnUpsideDown()
	  {
		  TImage oCpy(*this);
		  auto oWidth = width(); auto oHeight = height();
		  for (int i=0; i < oHeight; ++i)
		  {

			  for (int j=0; j < oWidth; ++j)
			  {
				  (*this)[i][j] = oCpy[oHeight-i-1][j];
			  }
		  }
	  }

	protected:
	private:
	  /// Bildgroesse: x, y
	  Size2d 	 size_;
	}; // TImage

	// -----------------------------------------------------------------------------------------------
	// Implementierungen


	// TEST
	// -----------------------------------------------------------------------------------------------

	//! Prints an image::TImage<T> type pixel-wise to std::out.
    /*!
      \param p_rImageIn Input image.
	  \param p_pMsg Optional label
	  \param p_IncRow filter Sampling rate in X direction. By default: '1'.
	  \param p_IncCol filter Sampling rate in Y direction. By default: '1'.
	  \return void
      \sa image::BImage
    */
	template <typename T>
	inline void printPixelTImage( const TImage<T> &p_rImageIn, const char *const p_pMsg = "PixelTImage", int p_IncRow = 1, int p_IncCol = 1 ) {
		std::cout << "\n< TImage: " << p_pMsg << ":\n";
		for (int row = 0; row < p_rImageIn.size().height; row += p_IncRow) {
			const T *const pRow = p_rImageIn[row]; // get line pointer
			std::cout << "row " << std::setw(3u) << row << ": ";
			for (int col = 0; col < p_rImageIn.size().width; col += p_IncCol) {
				std::cout << std::setw(3u) << static_cast<int>(pRow[col]) << ' ';
				//std::cout << static_cast<bool>(pRow[col]); // print binary without space
			} // for
			std::cout << "row end\n";
		} // for
		std::cout << ">\n" << std::endl;
	} // printPixelTImage


	//! Prints an image::TImage<T> type bit-wise to std::out.
    /*!
      \param p_rImageIn Input image.
      \param p_oNbBitsPerLine Number of bits printed per line.
	  \param p_pMsg Optional label
	  \param p_IncRow filter Sampling rate in X direction. By default: '1'.
	  \param p_IncCol filter Sampling rate in Y direction. By default: '1'.
	  \return void
      \sa image::BImage
    */
	template <typename T>
	inline void printBitTImage( const TImage<T> &p_rImageIn, const std::size_t p_oNbBitsPerLine, const char *const p_pMsg = "BitTImage", int p_IncRow = 1, int p_IncCol = 1 ) {
		std::cout << "\n< TImage: " << p_pMsg << ":\n";
		const std::size_t	oNbBitsPerPixel		( sizeof(T) * 8 );
		const unsigned int	oWidth				( p_rImageIn.size().width );
		const unsigned int	oHeight				( p_rImageIn.size().height );
		unsigned int		i					( 0 );
		for (int oRow = 0; oRow < oHeight; oRow += p_IncRow) {
			const T *const pRow = p_rImageIn[oRow]; // get line pointer
			std::cout << "Row "<< std::setw(3u) << oRow << ": ";
			for (unsigned int oCol = 0; oCol < oWidth; ++oCol) {
				std::cout << ' ';
				for (int oBitPos = oNbBitsPerPixel - 1; oBitPos >=  0; oBitPos -= p_IncCol, i += p_IncCol) {
					if (i >= p_oNbBitsPerLine) {					
						i = 0;
						std::cout << " Row end\n";
						break;
					} // if
					std::cout << static_cast<unsigned int>((pRow[oCol] >> oBitPos) & 1);
				} // for
			} // for
			//std::cout << "Row end\n";
		} // for
		std::cout << ">\n" << std::endl;
	} // printBitTImage



} // namespace image
} // namespace precitec


#endif /*IP_IMAGE_H_*/
