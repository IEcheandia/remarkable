 #ifndef IP_LINEIMAGE_H_
#define IP_LINEIMAGE_H_

#include "geo/size.h"
#include "geo/rect.h"
#include "geo/range.h"
#include "geo/point.h"

#include "memory/smartPtr.h"
#include "image/ipSignal.h"
#include "system/sharedMem.h"

#ifndef NDEBUG
  #define ASSERT_IMAGE_BOUNDS
#endif

#ifdef ASSERT_IMAGE_BOUNDS
    #define assert_bounds(x,y)   ( assert( x < this->size().width) , assert( y < this->size().height ) )
#else
    #define assert_bounds(x,y) ( static_cast<void>(x), static_cast<void>(y) )
#endif
namespace precitec
{
namespace image
{
	template <class ValueT>
	class TLineImage : public TImage<ValueT> {
	public:
		/// Typ der EinzelDaten/Pixel (Typ in das aktuelle Scope ziehen)
		typedef ValueT 						Value;

		/// Typ der EinzelDaten/Pixel ???? ist dieser Typ noetig wg Value
		typedef ValueT						PixelType;

		/// Referenz auf Daten: Rueckgabewert von image[diff], image(x, y)
		typedef Value &       		Reference;
		/// Referenz auf const-Daten: Rueckgabewert von image[diff], image(x, y) bei const-Image
		typedef Value const &     ConstReference;

		///	Pointer auf Daten
		typedef Value 		  		 *Pointer;
		///	const-Pointer auf Daten
		typedef Value const  		 *ConstPointer;
		/// abgeleiteter Allokator fuer Zeilenzeiger-Vektor
		//typedef typename Alloc::template rebind<Pointer>::other LineAllocator;

		/// 1D-Random Access-Iterator des Bildes, STL-kompatibel
		typedef Value * iterator1D; // ???? Achtung Schreibweise klein, noch zu aendern
		/// 1D-Random Access-Iterator des Bildes, STL-kompatibel
		typedef Value const * constIterator1D; // ???? Achtung Schreibweise klein, noch zu aendern

		typedef Size2d SizeType;
		typedef typename system::TSmartPointer<ValueT>::ShArrayPtr ShArrayPtr;


    public:
        // nun die Konstruktoren
        /// Bild der Groesse 0, ohne Speicher : wird verwendet um Bilder ausserhalb DMA anzulegen
        // TImage in AnalyzerInterface/include/image/ipImage.h
        TLineImage() : TImage<ValueT>(), lines_(), imageId_(0)
        {
            //std::cout<<"ipLineImage.h - TLineImage Konstruktor"<<std::endl;
        }

        /// copy-Construktor (SHALLOW)
        TLineImage(const TLineImage& rhs) : TImage<ValueT>(rhs), lines_(rhs.lines_), imageId_(rhs.imageId())
        {
                assert(this->isValid() == rhs.isValid()) ;
                assert(!this->isValid()
                            || (this->begin() == rhs.begin()  && this->end() == rhs.end() ));
        }

        /// CTor aus existierendem Smartptr (egal welcher Art) und Datengroesse
        TLineImage(ShArrayPtr &dataPtr, Size2d size)
            : TImage<ValueT>(dataPtr, size), lines_(TImage<ValueT>::height()), imageId_(0)
        {
            fillLines(TImage<ValueT>::width());
            assert(this->size() == size);
            assert(!this->isValid() || (this->begin() == dataPtr.get()));
        }

        /// CTor aus existierendem Smartptr (egal welcher Art) und Datengroesse
        /// wird verwendet um die Bilder auf den DMA Speicher anzulegen ...
        TLineImage( system::ShMemPtr<ValueT> &dataPtr, Size2d size)
                : TImage<ValueT>(dataPtr, size), lines_(TImage<ValueT>::height()), imageId_(0)
        {
            fillLines(TImage<ValueT>::width());
            assert(this->size() == size);
            assert(!this->isValid() || (this->begin() == dataPtr.get()));
        }

        /// neues Bild der Groesse size wird angelegt
        // TImage mit new Size konstruieren, Bildhohe TSignals instanzieren ...
        TLineImage(Size2d newSize) : TImage<ValueT>(newSize), lines_(newSize.height), imageId_(0)
        {
            fillLines(TImage<ValueT>::width());
            assert(this->size() == newSize);
        //      vigra_precondition((newSize.area() >= 0),
        //           "Image::Image(Size2d size): "
        //           "width and height must be >= 0.\n");
        }

        /// CTor mit externem nacktem Speicher
        TLineImage(Pointer data, Size2d newSize) : TImage<ValueT>(data, newSize), lines_(newSize.height), imageId_(0)
        {
            fillLines(TImage<ValueT>::width());
            assert(this->size() == newSize);
            assert(!this->isValid() || (this->begin() == data));

        //      vigra_precondition((newSize.area() >= 0),
        //           "Image::Image(Size2d size): "
        //           "width and height must be >= 0.\n");
        }

        /// subRoi CTor
        TLineImage(TLineImage const& rhs, geo2d::Rect roi, bool useSizeWithBorder = false)
        : TImage<ValueT>(rhs, rhs.roiToRange(roi, useSizeWithBorder), Size2d{useSizeWithBorder? roi.sizeWithBorder() : roi.size()}),
          lines_(TImage<ValueT>::height()),
          imageId_(0)
        {
            int xOffset(roi.x().start());
            int yOffset(roi.y().start());
            int height(lines_.numElements());

            for (int y=0; y<height; ++y)
            {
                lines_(y) = rhs.lines_(y+yOffset)+xOffset;
            }
#ifndef NDEBUG
            if (height == 0 || this->width() == 0)
            {
                assert(!this->isValid());
                return;
            }
            assert(this->begin() == rhs[roi.y().start()] + roi.x().start());
            if (roi.isEmpty())
            {
                return;
            }
            assert( this->end() -1 == (useSizeWithBorder? (rhs[roi.y().end()] + roi.x().end()) : (rhs[roi.y().end()-1] + roi.x().end()-1)));
#endif
        }

        /// Deserialisierungs-CTor
        TLineImage(system::message::MessageBuffer const& buffer);

        virtual ~TLineImage() 	{}
    private:
        /// CTor aus normalem Smartptr, privat weil falscher Pointertyp
        TLineImage(Poco::SharedPtr<ValueT>, Size2d ) {}
        /// 'linearisiert' Roi auf Speicherblock: erstes + letztes Element von Roi
        geo2d::Range roiToRange(geo2d::Rect const& roi, bool useSizeWithBorder) const
        {
            assert(this->isValid());
            auto firstX = roi.x().start();
            auto firstY = roi.y().start();
            assert(firstX >= 0 && firstY >= 0);
            assert(firstY < lines_.numElements());
            if (roi.isEmpty())
            {
                auto index = lines_[firstY] - TSignal<ValueT>::data()+ firstX;
                return geo2d::Range (index, index + 1);
            }

            auto lastX = useSizeWithBorder? roi.x().end() : roi.x().end() -1;
            auto lastY = useSizeWithBorder? roi.y().end() : roi.y().end() -1;
            assert(lastY < lines_.numElements());
            assert(lastX >= 0 && lastY >= 0);
            geo2d::Range r(lines_[firstY]- TSignal<ValueT>::data()+firstX,
                                        lines_[lastY]  - TSignal<ValueT>::data() + lastX);
            return r;
        }

	public:
		/// Bild kopieren ggf. nach resize
		TLineImage & operator=(TLineImage const& rhs) { TLineImage tmp(rhs); swap(tmp);	return *this;	}

		/// Bild kopieren aus SharedMemPtr
		TLineImage & operator=(system::TSmartSharedPtr<ValueT> const& dataPtr) { TLineImage tmp(dataPtr); swap(tmp);	return *this;	}

		/// Bild kopieren aus SmartPtr
		TLineImage & operator=(system::TSmartArrayPtr<ValueT> const& dataPtr) { TLineImage tmp(dataPtr); swap(tmp);	return *this;	}

        //check if it's possible to iterate on all the pixels by using begin() and end(). It assumes that the image is valid
		bool isContiguos() const
		{
			assert(TImage<ValueT>::isValid());
			auto oSize = this->size();
			return (lines_[0] == nullptr)
				|| (((lines_[oSize.height - 1] + oSize.width) - lines_[0]) == oSize.area()); //end-begin == num pixels
		}

        //returns stride in pixels
        ptrdiff_t stride() const
        {
            if (lines_.numElements() <= 1)
            {
                return this->size().width;
            }
            return lines_[1] - lines_[0];
        }

        //check if the image memory is overlapping
        bool overlapsWith(const TLineImage & rImage) const
        {
            if (!this->isValid() ||  !rImage.isValid())
            {
                return false;
            }
            bool beginOutside = this->begin() < rImage.begin() || this->begin() >= rImage.end();
            bool endOutside = this->end() < rImage.end() || this->end() > rImage.end();
            return !beginOutside || !endOutside;

        }


		/// Bild neu dimensionieren; neue Elemente mit mit Wert fuellen
		void resizeFill(Size2d const& newSize, ValueT newValue = ValueT()) {
			if (newSize != TImage<ValueT>::size()) {
				TImage<ValueT>::resizeFill(newSize, newValue);
				lines_.resize(newSize.height);
				fillLines(newSize.width);
			}
			else {
				fill(newValue);
			}
            assert(this->size() == newSize);
            if (!this->isValid())
            {
                return;
            }
            assert(this->getValue(0, 0)==newValue);
            assert(this->getValue(newSize.width-1, newSize.height-1)==newValue);
		}

		/// Bild neu dimensionieren
		void resize(Size2d const& newSize) {
			if (newSize != TImage<ValueT>::size()) {
				TImage<ValueT>::resize(newSize);
				lines_.resize(newSize.height);
				fillLines(newSize.width); // adjust line pointers
			}
            assert(this->size() == newSize);
		}

		void swap(TLineImage & rhs );
		/// Bild auf konstanten Wert setzen
        TLineImage & fill(ValueT const& value)
        {
            const auto oSize = this->size();

            Pointer const * pDestLines = lines_.data();
            Pointer const * const pDestLinesEnd = pDestLines + oSize.height;

            #ifndef NDEBUG
            int y = 0;
            #endif

            for ( ; pDestLines != pDestLinesEnd; ++pDestLines)
            {
                Pointer pDestPixel = (*pDestLines);
                assert(pDestPixel == rowBegin(y));

                std::fill_n(pDestPixel, oSize.width, value);

                #ifndef NDEBUG
                ++y;
                #endif
            }

            assert(y == oSize.height);
            return *this;

        }

		ValueT getValue( const int & dx, const int & dy ) const
		{
		assert_bounds(dx,dy);
			return lines_[dy][dx];
		}

		void setValue( const int & dx, const int & dy, const ValueT & v )
		{
			assert_bounds(dx,dy);
			lines_[dy][dx] = v;
		}

		void clear()
		{
			//also forces reallocation
			resize( Size2d( 0, 0 ) );
			assert( !TImage<ValueT>::isValid() );
		}

		/// 1D random access Zeilen-Iterator des ersten (linken) Pixels der y. Zeile
		ConstPointer rowBegin(int y) const
		{
			assert_bounds(0,y);
			return lines_[y];
		}
		/// 1D random access Zeilen-Iterator des letzten (rechten) Pixels der y. Zeile
		ConstPointer rowEnd(int y) const
		{
			assert_bounds(0,y);
			return lines_[y] + TImage<ValueT>::width();
		}
		/// 1D random access Zeilen-Iterator des ersten (linken) Pixels der y. Zeile
		Pointer rowBegin(int y)
		{
			assert_bounds(0,y);
			return lines_[y];
		}
		// 1D random access Zeilen-Iterator des letzten (rechten) Pixels der y. Zeile
		Pointer rowEnd(int y)
		{
			assert_bounds(0,y);
			return lines_[y] + TImage<ValueT>::width();
		}

		/// 2D random access Iterator des linken oberen Pixels
		Pointer upperLeft()
		{
			return begin();
		}
		/// 2D random access Iterator des linken oberen Pixels
		ConstPointer upperLeft() const
		{
			return begin();
		}
		/// 2D random access Iterator des linken unteren Pixels
		Pointer lowerLeft()
		{
			return rowBegin(TImage<ValueT>::height() - 1);
		}
		/// 2D random access Iterator des linken unteren Pixels
		ConstPointer lowerLeft() const
		{
			return rowBegin(TImage<ValueT>::height()- 1);
		}

		/// 1D random Iterator des ersten Pixels
		iterator1D begin()
		{
            if (!this->isValid())
            {
                return nullptr;
            }
			return rowBegin(0);
		}

		/// 1D random Iterator des letzten Pixels
		iterator1D end()
		{
			return rowBegin(TImage<ValueT>::height() - 1) + TImage<ValueT>::width();
		}
		/// 1D random const-Iterator des ersten Pixels
		constIterator1D begin() const
		{
            if (!this->isValid())
            {
                return nullptr;
            }
			return rowBegin(0);
		}
		/// 1D random const-Iterator des letzten Pixels
		constIterator1D end() const
		{
		return  rowBegin(TImage<ValueT>::height() - 1) + TImage<ValueT>::width();
		}

		///const-Bildzugriff value = image(1, 3);
		virtual Reference operator()(int dx, int dy)
		{
			assert_bounds(dx,dy);
			return lines_[dy][dx];
		}
		///const-Bildzugriff value = image(1, 3);
		virtual ConstReference operator()(int dx, int dy) const {
			 assert_bounds(dx,dy);
			 return lines_[dy][dx];
		}
		/// Bildzugriff mit Doppelklammern value = image[y][x];  // fehlertraechtig wg x-y y-x
		virtual Pointer operator[](int dy) {
			assert_bounds(0,dy);
			return lines_[dy];	}
		/// const-Bildzugriff mit Doppelklammern value = image[y][x];  // fehlertraechtig wg x-y y-x
		virtual ConstPointer operator[](int dy) const {
			assert_bounds(0,dy);
			return lines_[dy]; }

        /**
         * Id of the image. The id is consecutively numbered by the framegrabber.
         * The id is allowed to wrap around, thus is only temporarily unique.
         **/
        uint32_t imageId() const
        {
            return imageId_;
        }

        void setImageId(uint32_t imageId)
        {
            imageId_ = imageId;
        }

        static void copyPixels(const TLineImage<ValueT>  & rSourceImage, TLineImage<ValueT> & rDestinationImage)
        {
            static_assert(std::is_trivial<ValueT>::value, "copyPixels is implemented only for trivially copiable types");

            const auto oSize = rSourceImage.size();
            ConstPointer const * pSrcLines = rSourceImage.lines_.data();
            ConstPointer const * const pSrcLinesEnd = rSourceImage.lines_.data() + oSize.height;
            Pointer const * pDestLines = rDestinationImage.lines_.data();

            #ifndef NDEBUG
            int y = 0;
            #endif

            const std::size_t bytesPerRow = oSize.width * sizeof(ValueT);
            while (pSrcLines != pSrcLinesEnd)
            {
                assert((*pSrcLines) == rSourceImage.rowBegin(y) && (*pDestLines) == rDestinationImage.rowBegin(y));

                std::memcpy(*pDestLines, *pSrcLines, bytesPerRow);

                ++pSrcLines;
                ++pDestLines;

                #ifndef NDEBUG
                ++y;
                #endif

            }

        }


        template<class UnaryOperation>
        static void transform(const TLineImage<ValueT> & rIn, TLineImage<ValueT> & rOut, UnaryOperation unary_op)
        {
            const auto oSize = rIn.size();
            ConstPointer const * pSrcLines = rIn.lines_.data();
            ConstPointer const * const pSrcLinesEnd = rIn.lines_.data() + oSize.height;
            Pointer const * pDestLines = rOut.lines_.data();
#ifndef NDEBUG
            int y = 0;
#endif
            while (pSrcLines != pSrcLinesEnd)
            {
                ConstPointer pSrcPixel = (*pSrcLines);
                ConstPointer pSrcPixelEnd = pSrcPixel + oSize.width;
                Pointer pDestPixel = (*pDestLines);
                assert(pSrcPixel == rIn.rowBegin(y) && pDestPixel == rOut.rowBegin(y));

                while(pSrcPixel != pSrcPixelEnd)
                {
                    *pDestPixel++ = unary_op(*pSrcPixel++);
                }

                ++pSrcLines;
                ++pDestLines;
#ifndef NDEBUG
                ++y;
#endif
            }
        }


        template<class TValueOut, class UnaryOperation>
        static void transform(const TLineImage<ValueT> & rIn, TLineImage<TValueOut> & rOut, UnaryOperation unary_op)
        {
            typedef typename TLineImage<ValueT>::ConstPointer TInConstPointer;
            typedef typename TLineImage<TValueOut>::Pointer TOutPointer;


            const auto oSize = rIn.size();
            TInConstPointer const * pSrcLines = rIn.lines_.data();
            TInConstPointer const * const pSrcLinesEnd = rIn.lines_.data() + oSize.height;

           // TLineImage<TValueOut>.lines_ is private if TValueOut != ValueT, we need to access it with rowBegin(y)
            int y = 0;

            while (pSrcLines != pSrcLinesEnd)
            {
                TInConstPointer pSrcPixel = (*pSrcLines);
                TInConstPointer pSrcPixelEnd = pSrcPixel + oSize.width;
                TOutPointer pDestPixel = rOut.rowBegin(y);
                assert(pSrcPixel == rIn.rowBegin(y) && pDestPixel == rOut.rowBegin(y));

                while(pSrcPixel != pSrcPixelEnd)
                {
                    *pDestPixel++ = unary_op(*pSrcPixel++);
                }

                ++pSrcLines;
                ++y;
            }
        }


        template<class OutputIt, class UnaryOperation>
        static OutputIt transform(const TLineImage<ValueT> & rIn, OutputIt d_first, UnaryOperation unary_op)
        {
            const auto oSize = rIn.size();
            ConstPointer const * pSrcLines = rIn.lines_.data();
            ConstPointer const * const pSrcLinesEnd = rIn.lines_.data() + oSize.height;

            #ifndef NDEBUG
            int y = 0;
            #endif

            while (pSrcLines != pSrcLinesEnd)
            {
                ConstPointer pSrcPixel = (*pSrcLines);
                ConstPointer pSrcPixelEnd = pSrcPixel + oSize.width;

                assert(pSrcPixel == rIn.rowBegin(y));

                while(pSrcPixel != pSrcPixelEnd)
                {
                    *d_first++ = unary_op(*pSrcPixel++);
                }

                ++pSrcLines;

                #ifndef NDEBUG
                ++y;
                #endif
            }
            return d_first;
        }

        template<class InputIt, class UnaryOperation>
        static void transform(InputIt first1, InputIt last1, TLineImage<ValueT> & rOut, UnaryOperation unary_op)
        {
            const auto oSize = rOut.size();
            assert(last1-first1 == oSize.area());

            Pointer const * pDestLines = rOut.lines_.data();
            Pointer const * const pDestLinesEnd = pDestLines + oSize.height;

            #ifndef NDEBUG
            int y = 0;
            #endif

            while (first1 < last1 &&   pDestLines != pDestLinesEnd)
            {
                Pointer pDestPixel = (*pDestLines);
                Pointer pDestPixelEnd = pDestPixel + oSize.width;

                assert(pDestPixel == rOut.rowBegin(y));

                while(first1 != last1 && (pDestPixel != pDestPixelEnd))
                {
                    *pDestPixel++ = unary_op(*first1++);
                }

                ++pDestLines;

                #ifndef NDEBUG
                ++y;
                #endif
            }
        }


        void copyPixelsTo(TLineImage<ValueT> & rDestination) const
        {
            if (! this->isValid())
            {
                return;
            }

            rDestination.resize(this->size());

            if (this->isContiguos() && rDestination.isContiguos())
            {
                std::copy(this->begin(), this->end(), rDestination.begin());
            }
            else
            {
                copyPixels(*this,rDestination);
            }

        }

        template<class TValueOut, class UnaryOperation>
        void transformTo(TLineImage<TValueOut> & rDestination, UnaryOperation unary_op) const
        {
            if (! this->isValid())
            {
                return;
            }

            assert(this->size() == rDestination.size());

            if (this->isContiguos())
            {
                if (rDestination.isContiguos())
                {
                    std::transform(this->begin(), this->end(), rDestination.begin(), unary_op);
                    return;
                }
                else
                {
                    return TLineImage<TValueOut>::transform(this->begin(), this->end(), rDestination, unary_op);
                }
            }

            return TLineImage<ValueT>::transform(*this, rDestination, unary_op);
        }



        template<class UnaryFunction>
        UnaryFunction for_each(UnaryFunction f) const
        {
            if (!this->isValid())
            {
                return f;
            }
            if (this->isContiguos())
            {
                return std::for_each(this->begin(), this->end(), f);
            }
            const auto oSize = this->size();
            return this->for_each( f, 0,0,oSize.width, oSize.height);
        }


        template<class UnaryFunction>
        UnaryFunction for_each(UnaryFunction f,  int x0, int y0, int width, int height)  const
        {
#ifndef NDEBUG
            auto y = y0;
#endif
            if (width == 0)
            {
                return f;
            }
            ConstPointer const * pSrcLines = this->lines_.data() + y0;
            ConstPointer const * const pSrcLinesEnd = pSrcLines + height;

            while (pSrcLines != pSrcLinesEnd)
            {
                ConstPointer pSrcPixel = (*pSrcLines) +x0;
                ConstPointer pSrcPixelEnd = pSrcPixel + width;
                assert(pSrcPixel == this->rowBegin(y)+x0);

                while(pSrcPixel != pSrcPixelEnd)
                {
                    f(*pSrcPixel++);
                }

                ++pSrcLines;
#ifndef NDEBUG
                ++y;
#endif
            }
            return f;

        }

	// der Serialisierungs-Block
		/// BildTyp fuer polymorphe Serialiserung
		int type() const { return TImage<ValueT>::LineImage; }
		/// Fabrik fuer Polymorphe Deserialisierung (aus TImage augerufen)
		static TImage<ValueT>* create(system::message::MessageBuffer const& buffer );

		/// rekursiv in Buffer serialisieren
		void serialize( system::message::MessageBuffer &buffer ) const;
		/// aus Buffer rekursiv deserialisieren
		void deserialize( system::message::MessageBuffer const&buffer );

		/// Fabrik fuer Generator
		static TLineImage* generate(Size2d size) { return new TLineImage(size); }
		/// Fabrik fuer Generator,  Erzeugung mit vorhandenem Speicher ????
		static TLineImage* generate(Size2d size, void *p) { return new TLineImage(size, p); }

	private:
		/// fill fuer diverse CToren
		void fillLines(std::size_t pitch);


	private:
		/// 1D Array mit Zeilenzeigern
	  TSignal<Pointer> lines_;
      uint32_t imageId_;
	}; // TLineImage


#if defined __QNX__ || defined __linux__

	template <class ValueT>
	TImage<ValueT> * TLineImage<ValueT>::create(system::message::MessageBuffer const& buffer )
	{
		return new TLineImage( buffer );
	}
#else

	template <class ValueT>
	typename TImage<ValueT> * TLineImage<ValueT>::create(system::message::MessageBuffer const& buffer )
	{
		return new TLineImage( buffer );
	}
#endif


	template <class ValueT>
	void TLineImage<ValueT>::fillLines(std::size_t pitch) {
		const int height(TImage<ValueT>::size().height);
		assert(int(lines_.getSize()) == height);
		if (!TImage<ValueT>::isValid())
		{
			for (int y=0; y<height; ++y)
			{
				lines_(y) = Pointer(0);
			}
		}
		else
		{
			auto pLineStart = TSignal<ValueT>::data();
			auto pLineStorage = lines_.data();
			auto pLineStorageEnd = pLineStorage + height;
			while(pLineStorage != pLineStorageEnd)
			{
				(*pLineStorage) = pLineStart;
				++pLineStorage;
				pLineStart += pitch;
			}

#ifndef NDEBUG
			std::size_t offset(0);
			for (int y=0; y<height; ++y, offset+=pitch)
			{
				assert( lines_(y) == TSignal<ValueT>::data() + offset);
			}
#endif
		} // if
	}

	template <class ValueT>
	void TLineImage<ValueT>::serialize( system::message::MessageBuffer &buffer ) const {
		TImage<ValueT>::serialize(buffer);
		// im Moment wird davon ausgegangen,
		// - dass alle Zeilen gleich lang sind
		// - dass sie kontinuierlich in einen Block gemapt werden
		// - damit kann der Linien-Container automatisch erzeugt werden
		if (lines_.numElements()>1) {
			auto pitch = lines_(1)-lines_(0); // ???? bug: geht so nicht, Zeilen muessen einzenl de/serialisiert werden!!!!!!!
			system::message::Serializable::marshal(buffer, pitch); // pitch
			//system::message::Serializable::marshal(buffer, lines_(1)-lines_(0)); // pitch
		}	else {
			system::message::Serializable::marshal(buffer, std::size_t(TLineImage::width())); // pitch, bei <= 1 Zeile willkuerlich
		}
		system::message::Serializable::marshal(buffer, imageId_);
	}

	template <class ValueT>
	TLineImage<ValueT>::TLineImage(system::message::MessageBuffer const& buffer)
	: TImage<ValueT>(buffer), lines_(TLineImage<ValueT>::size().height)
	{
		std::size_t pitch(system::message::Serializable::deMarshal<std::size_t>(buffer));
		//std::cout << "TIndexImage: deser offset width height: "
		//					<< " : " << pitch << " : " << lines_.numElements() << std::endl;
		fillLines(pitch);

		imageId_ = system::message::Serializable::deMarshal<uint32_t>(buffer);
	}


	template <class ValueT>
	void TLineImage<ValueT>::deserialize( system::message::MessageBuffer const&buffer ) {
		TLineImage tmp(buffer); swap(tmp);
	}


	template <class ValueT>
	inline void TLineImage<ValueT>::swap(TLineImage & rhs ) {
		TImage<ValueT>::swap(rhs);
		lines_.swap(rhs.lines_);
		std::swap(imageId_, rhs.imageId_);
	}
/*
	/// Fabrik fuer Generator
	template <class ValueT>
	TLineImage* generate(Size2d size) {
	}

	/// Fabrik fuer Generator,  Erzeugung mit vorhandenem Speicher
	template <class ValueT>
	TLineImage* generate(Size2d size, void *p);

	template <class ValueT>
	inline TLineImage<ValueT> genModuloPattern(Size2d size, int seed) {
		TLineImage<ValueT> image(size);
		for (int y=0, i=seed; y<size.height; ++y)
			for (int x=0; x<size.width; ++x, ++i)
				image[y][x] = byte(i& 0x7);
		return image;
	}

	template <class ValueT>
	inline bool testModuloPattern(TLineImage<ValueT> image, int seed) {
		bool ok = true;
		for (int y=0, i=seed; y<image.size().height; ++y)
			for (int x=0; x<size.width; ++x, ++i)
				ok &= image[y][x] == byte(i& 0x7);
		return ok;
	}
*/

} // namespace precitec
} // namespace image

#undef assert_bounds


#endif /*IP_LINEIMAGE_H_*/
