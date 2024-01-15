/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter tries to detect a poor penetration.
*/

#ifndef POORPENETRATION_H_
#define POORPENETRATION_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

using precitec::image::BImage;
using precitec::image::OverlayPoint;
using precitec::image::OverlayLine;
using precitec::image::OverlayRectangle;

//using precitec::filter::BoxSummer;

namespace precitec {
	using namespace interface;
	using namespace image;

	namespace filter {

		struct PoorPenetrationPoint
		{
			PoorPenetrationPoint();
			PoorPenetrationPoint(int x, int y, Color color);
			int x;
			int y;
			Color color;
		};

		struct PoorPenetrationLine
		{
			PoorPenetrationLine();
			PoorPenetrationLine(int x1, int y1, int x2, int y2, Color color);
			int x1;
			int y1;
			int x2;
			int y2;
			Color color;
		};

		struct PoorPenetrationRectangle
		{
			PoorPenetrationRectangle();
			PoorPenetrationRectangle(int x, int y, int width, int height, Color color);
			int x;
			int y;
			int width;
			int height;
			Color color;
		};

		class PoorPenetrationOverlay
		{
		public:
			PoorPenetrationOverlay();
//			PoorPenetrationOverlay(Trafo trafo);

			//void setTrafo(Trafo trafo);

			void reset();

			const std::vector<PoorPenetrationPoint> & getPointContainer() const;
			const std::vector<PoorPenetrationLine> & getLineContainer() const;
			const std::vector<PoorPenetrationRectangle> &  getRectangleContainer() const;

			void addPoint(int x, int y, Color color);
			void addLine(int x1, int y1, int x2, int y2, Color color);
			void addRectangle(int x, int y, int width, int height, Color color);

			//int getPointCount();
			//int getLineCount();
			//int getRectangleCount();

		private:
			std::vector<PoorPenetrationPoint> _pointContainer;
			std::vector<PoorPenetrationLine> _lineContainer;
			std::vector<PoorPenetrationRectangle> _rectangleContainer;

//			Trafo _trafo;
		};

	


		struct SChain // Speichert alles zu einer langen Kette
		{
			int		start;          ///< Anfang der Kette
			int		laenge;         ///< Laenge der Kette
			double	m;           ///< Steigung der Regressionsgerade
			double	b;           ///< Y-Achsenabschnitt der Regressionsgerade
			double	mGW;         ///< mittlerer Grauwert auf der Kette
			double	mX;          ///< mittlerer X-Wert der Kette
			int		minX;			///< minimaler X-Wert der Kette
			int		maxX;			///< maximaler X-Wert der Kette
			int		x1, y1, x2, y2; ///< Ecken der Box
			bool	KantenIO;	///< 0 = Kanten der Ketten-Minima sind gerade, 1 = Kanten der Ketten-Minima zappeln beide = Naht !!
		};

		struct SNoSeamKontur // Speichert die Daten der Minima-Kontur
		{
			SNoSeamKontur();
			int		XL;               ///< X-Position linke Kante von Grauwertmin
			int		XR;               ///< X-Position rechte Kante von Grauwertmin
			int		xPos;             ///< X-Position von Grauwertmin
			int		zeile;            ///< Zeile des Minimum, in 10-er Schritten
			int		GW;               ///< Grauwert des Minimum
			bool	Del;              ///< Diesen Kontur-Wert weglassen/nicht beruecksichtigen
			int     DispCol;          ///< 0 = gruen = OK, 1 = rot = FDS

			int     KetteNr;          ///< Zaehler fuer zusammenhaengende Minima
			int     KetteGruppeVor;   ///< Passende Minima-Gruppe der vorherigen Zeile
			int     KetteGruppeNach;  ///< Passende Minima-Gruppe der nachfolgenden Zeile
		};

		struct SNoSeamPara
		{
			int		SeamWidth;
			//int		minLengthGSF; // minimale Laenge gute Nahtfindung
			int		minLength[4]; // minimale Laenge schlechte Nahtfindung
			int		maxGreyVal[4]; // maximiale Grauwert auf potentieller fehlenden Durchschweissung
			int		minWidth[4]; // minimale Breite
			int		maxWidth[4]; // maximale Breite
			int		minGradient[4]; // minimaler Gradient an den Seiten der fehlenden DS
			int		Sensitivity; // Empfindlichkeit, 100 sehr empfindlich, 1 uebersieht alles
			//double	PerCentKill; // Ausreisser, die verworfen werden
			int		StdAbwMax[4];
			double	MinValRatio[4];
			double	MaxLL[4];
			int		GueteSchwelle;
			int		ProzentPos;
			int   Konkavitaet; // Speichert die Geo-
			int   Konvexitaet; // metriedaten der Naht
			int   SeamQuality; //haelt die SeamQuality, kommt von der Texturnahtsuche
		};

		struct SNoSeamResult //Speichert die Ergebnisse der NoSeam-Detection
		{
			int		nr; //Nr. des Results
			int		bFilled; // steht ueberhaupt was drin?

			int		NoSeamProb; // Wahrscheinlichkeit, dass eine fehlende Durchschweissung da ist.
			int		box_x1, box_y1, box_x2, box_y2; // Umschreibendes Rechteck der fehlenden Durchschweissung
			int		left_x; // linke Kante der fehlenden Durchschweissung
			int		right_x; // rechte Kante
			int		width; // Breite der f. Durchschweissung
			double	StdAbwLinks;//zugehoerige Std-Abweichungen
			double	StdAbwRechts;
			double	StdAbwBreite;

			double	Gradient;

			double	greyvalAreaIn; //mittlerer Grauwert auf der evtl. FDS (ganze Flaeche) Box
			double	greyvalAreaInEdge; //mittlerer Grauwert auf der evtl. FDS (ganze Flaeche) max. Gradient Kanten (rot/gruen)
			double	greyvalAreaOut;
			int		greyvalChain; // (nur entlang der Kette)

			double	LauflaengeLinks; //Lauflaengen nach der
			double	LauflaengeRechts;//Bereinigung (PerCentKill)

			//void	PrintIt();
			void	Reset();
		};


		class ImgSeamPos
		{
		public:
			ImgSeamPos();
			void	reset();
			void	addPos(int pos);
			int		getPos();
			bool	isFull();

		private:
			std::vector<int> _pos;
			int		_counter;
			bool	_full;
			int		_sumPos;
		};

		class SeamPositions
		{
		public:
			SeamPositions();
			void reset();
			std::vector<ImgSeamPos> imgSeamPos;
			void addPos(int pos, bool isFirst);
			void addNoPos(bool isFirst);
			int getPos();

		private:
			int _curImg;
		};

		class ProductPositions
		{
		public:
			ProductPositions();
			void	reset();
			void	addPos(int seamNr, int pos, bool isFirst);
			void	addNoPos(int seamNr, bool isFirst);
			int		getPos(int seamNr);

		private:
			std::vector<SeamPositions> _seamPositions;
		};

		class RingBuffer
		{
		public:
			RingBuffer();
			~RingBuffer();
			void reset(int size);
			void addValue(int value);
			int getMean();
			bool isFull();
			bool isValid();

		private:
			std::vector<int> _ringBufferEntry;
			int _maxSize;
			bool _isFull;
			int _curPos;
			bool _isValid;
			double _curSum;
		};

		class BoxSummer
		{
		public:
			BoxSummer(precitec::image::BImage image);
			~BoxSummer();
			long getSum(int x1, int y1, int x2, int y2, int speederX = 1, int speederY = 1);

		private:
			BImage _image;
			int _width;
			int _height;
			long _oldSum;
			int _oldX1, _oldX2, _oldY1, _oldY2;
		};

		class Statistic1Dim
		{
		public:
			Statistic1Dim(int size, int noOfValues);
			virtual ~Statistic1Dim();
			int		addValue(int i);
			void	reset();
			int		getNumber(int place);
			int		getMedian();

		private:
			std::vector<int> _valArray;
			std::vector<int> _bucket;

			int		_maxSize;
			int		_curSize;
			int		_numberOfValues;

		};

		class GeradenRegression
		{
		public:
			GeradenRegression();
			virtual ~GeradenRegression();
			double	getYAbschnitt();
			double	getSteigung();
			void	calcGerade();
			bool	ergebnisDa();
			void	addPoint(double x, double y);
			double	getError();
			void	calcGerade(double &m, double &b);
			void	reset();
			double	getAbstand(double c1, double c2);

		private:
			double	_m, _b;
			bool	_bErgebnis;
			double	_sx, _sy, _sxx, _syy, _sxy;
			int		_iAnzahl;
		};

		class SimpleMedian
		{
		public:
			SimpleMedian(int size, int noOfValues);
			virtual ~SimpleMedian();
			int		addValue(int i);
			void	resetBuckets(void);
			void	resetArray(void);
			int		getNumber(int place);
			int		getMedian();
			double	getMean();

		private:
			int		_maxSize;
			int		_curSize;
			int		_numberOfValues;
			int		_curMin;
			int		_curMax;
			double	_sum;

			std::vector<int>	_valArray;
			std::vector<int>	_bucket;
		};

		struct SDisplayBadPen
		{
			bool bValid; //speichert, ob ueberhaupt alles belegt ist

			int x1, y1, x2, y2; //speichert die Koordinaten der bounding Box

			int iWidth, iWidthCol; //Breite

			int iLength, iLengthCol; //Laenge

			int iGradient, iGradientCol; //Gradient

			int iGreyValGap, iGreyValGapCol; //Grauwert Spalt

			int iGreyValIn, iGreyValOut; //Grauwert Innen / Aussen
			int iGreyValInCol, iGreyValOutCol;

			int iStdDev, iStdDevCol; //Standard-Abweichung

			int iLauflaengeLinks, iLauflaengeRechts; // Lauflaengen
			int iLauflaengeLinksCol, iLauflaengeRechtsCol;

			int iResult, iResultCol; // Ergebnis

			inline void reset()
			{
				bValid = false;
				x1 = x2 = y1 = y2 = 0;
				iWidth = iWidthCol = 0;
				iLength = iLengthCol = 0;
				iGradient = iGradientCol = 0;
				iGreyValGap = iGreyValGapCol = 0;
				iGreyValIn = iGreyValOut = 0;
				iGreyValInCol = iGreyValOutCol = 0;
				iStdDev = iStdDevCol = 0;
				iLauflaengeLinks = iLauflaengeRechts = 0;
				iLauflaengeLinksCol = iLauflaengeRechtsCol = 0;
				iResult = iResultCol = 0;
			}

			//ueberfluessig:

			int iStdAbwLinks, iStdAbwRechts, iStdAbwBreite;
			int iStdAbwLinksCol, iStdAbwRechtsCol, iStdAbwBreiteCol;

		};

		class NoSeamFind
		{
		public:
			NoSeamFind(BImage image);
			NoSeamFind();
			~NoSeamFind();

			// Hauptfunktion
			void CheckForErrors(SDisplayBadPen displayBadPen[], int display, int mode,
				int windowX,int windowY,
				int anzPosUpper, int anzPosUnder,
				int lowerThreshold,int upperThreshold,int minWidth);

			void setImage(BImage image);

			void Reset();
			void SetParams(int iSeamPos, int iSeamWidth, int iGuete, /*ImageInfo imageInfo,*/ int StartY, int EndY, int direction,
			               int iKonkav, int iKonvex, int iCalcMode, int iSeamQuality);
			void SetBildPointer(unsigned char * pBild);

			PoorPenetrationOverlay getOverlay();
			std::vector<geo2d::PoorPenetrationCandidate> getCandidates();

			int getErrorPosition(int chainNo);


		private:
			void FillKonturMulti(int x1, int x2, int y1, int y2, const int &mode);
			void ScanLineMulti(int line,  
				               int startx, int endx,
			                   int &iMin1XL, int &iMin1XR, int &iMin1Pos, int &iMin1Val,
				               int &iMin2XL, int &iMin2XR, int &iMin2Pos, int &iMin2Val,
				               int &iMin3XL, int &iMin3XR, int &iMin3Pos, int &iMin3Val, int iDisplayStreifen, const int &mode);
			void AddPointMulti(int zeile, int iMin1XL, int iMin1XR, int iMin1Pos, int iMin1Val,
				               int iMin2XL, int iMin2XR, int iMin2Pos, int iMin2Val,
				               int iMin3XL, int iMin3XR, int iMin3Pos, int iMin3Val, int iDispCol = 0);
			void	KillSingleMinima(int iMinMinima);
			void	PackMultiMinima(void);
			void	EliminateMultiMinima(int iMaxDiff);
			void	MoveMin3NachMin1(int iKonturElement);
			void	MoveMin3NachMin2(int iKonturElement);
			void	MoveMin2NachMin1(int iKonturElement);
            bool    noBadMinimum(const SNoSeamKontur& contour1, const SNoSeamKontur& contour2, const int maxDiff);
            /**
            * Description:  Hauptfunktion fuer die Suche nach zusammenhaengenden
            *               Ketten. Allenfalls zusammensetzen der nach der
            *               'EliminateMultiMinima' uebrig gebliebenen Min's.
            *
            * Parameter:    iMaxDiff:          IRob.Res[2] = Reserve 3
            *
            * Returns:
            */
			void	SearchForChainsCont(int maxDiff1);
            /**
             * Computes for the edges of a chain the left and right maximum x-position
			 * If Delta > iMaxKantenDelta for both edges => fidgets strongly = is seam = IO
             */
			void	CalcChainsKanten(int iMaxKantenDelta);
			void	CalcBoxMulti(int nr);
			void	CheckBoxMulti(int nr);

			PoorPenetrationOverlay _overlay;

			// Speziell fuer Projekt Yudigar
//			void	FillKontur(int x1, int x2, int y1, int y2, int iBreite = 0);
			//void	ScanLine4(int line, int startx, int endx, int sizex, int sizey, int &index, int &gw, int & iDoppel);
			void	AddPoint(int cor1, int cor2, int gw, int iDispCol = 0);
			void	KillSingles2Rect(int maxDiff1);
			int		CountNeighborsRect(int co1, int co2, int maxDiff1, int maxDiff2);
			void	SearchForChains2();
			//void	CalcBox(int nr);
			//int		CheckBox(int nr);

			// Generell fuer alle Projekte
			void	InitKontur();
			void	AddNeueKette(int start, int laenge);
			void	CalcChainsMB();
			void	DeleteAllButChains();
			void	CalcBoxes(bool draw);
			//void	FillDisplay(SDisplayBadPen displayBadPen[]);

			long	getSum(int x1, int y1, int x2, int y2);

			int MinY_, MaxY_;

			int iCalcMode_;
			int x1old_, x2old_, y1old_, y2old_;
			long oldSum_;

            enum {
             ANZAHL_KONTUR = 1000,
             ANZAHL_PARAMETER_GRUPPEN = 3,
             ANZAHL_KETTEN = 3
            };

			//void	ScanLine(int line, int startx, int endx, int &index, int &gw, bool bDisplayStreifen);
			//void	ScanLine2(int line, int startx, int endx, int sizex, int sizey, int &index, int &gw);
			//void	ScanLine3(int line, int startx, int endx, int sizex, int sizey, int &index, int &gw);
			//int		SumCandidates();

			void	DrawKontur_1();
			void	DrawKontur_2();
			void	DrawKontur_3();
			void	DrawXPos();
			void	DrawXPosKanten();
			void	DrawXPosMin1(void);
			void	DrawXPosMin1Kanten(void);
			void	DrawBoxes(bool drawIO);
			void	DrawKettePos(void);
			void	DrawKetteKante(void);
			//	void	SearchForChains();
			int		GetNextOben(int ort);
			int		GetNextUnten(int ort);
			int		GetRealInKontur();
			void	ChainInfo();
			double	CalcSurfaceY(int x1, int y1, int x2, int y2);
			double	GetSurfaceY(int lk);
			//	bool	CheckBox(int x1, int y1, int x2, int y2);

			//	void	WalkThroughColumns(int x1, int y1, int x2, int y2);
			//	void	ScanGreyROI(int x1, int y1, int x2, int y2);

			//	void	DrawColMeans(int x1, int y1, int x2, int y2);
			//	void	CalcColumnMean(int x1, int y1, int x2, int y2);

			//	void	GenerateStripes(int x1, int y1, int x2, int y2);
			//	void	GenerateTotalStripe();

			//	int		smoothStripe(CStreifen* pSourceStripe, CStreifen* pDestinationStripe, int nThreshold, int nMin[2]);
			// protected:

			SChain LK[3]; ///< long chain

			void	clamp(int & var, int lower, int upper);
			void	change(int & var1, int & var2);
			double	GetMeanGreyVal(int x1, int y1, int x2, int y2);
			int		CalcGreyVals(int x1, int y1, int x2, int y2, int left, int right, double & ValIn, double & ValOut);

			SNoSeamPara		NoSeamPara;
			//paramBlockDef	SeamPar;
			//ROI		roiGreyDim_;
			int Gx1, Gy1, Gx2, Gy2;

			public:
			// Kontur-Parameter
			// ----------------

			// Erstes Minimum
			std::vector<SNoSeamKontur>  Kontur1;

			// Zweites Minimum
			std::vector<SNoSeamKontur>  Kontur2;

			// Drittes Minimum
			std::vector<SNoSeamKontur>  Kontur3;

			int		iKonturAnzahl;
			//bool bKonturOrdered;
			bool	bKonturXY;

			private:

			double	dSteigung, dAchse, dFehler;

			GeradenRegression geradenRegression;
			SNoSeamResult NoSeamResult[ANZAHL_PARAMETER_GRUPPEN];

			BImage _image;
			int _width, _height;  // Bilddimensionen
			int _windowX, _windowY; ///< Dim. fuer Fensterung (Minimumfindung)

			int _anzPosUpper;
			int _anzPosUnder;

			int _lowerThreshold;
			int _upperThreshold;
			int _minimaWidth;

			int		AnzahlArray[1024];
			int		m_MaxIndex;

			int		iAnzahlStreifen;

			int absInt(int i);
			int _displayParameter;
		};

		class FILTER_API PoorPenetration : public fliplib::TransformFilter
		{

		public:

			PoorPenetration();
			virtual ~PoorPenetration();

			static const std::string m_oFilterName;				///< Name Filter
			static const std::string PIPENAME_RESULT_OUT;		///< Name Out-Pipe

			/// Set filter parameters as defined in database / xml file.
			void setParameter();
			/// paints overerlay primitives
			void paint();

		protected:

			/// In-pipe registration.
			bool subscribe(fliplib::BasePipe& pipe, int group);
			/// In-pipe event processing.
			void proceed(const void* sender, fliplib::PipeEventArgs& e);

		private:

			const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;		///< In pipe
			//const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;	///< In pipe
			//const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble1;		///< In pipe
			//const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble2;		///< In pipe
			//const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble3;		///< In pipe

			fliplib::SynchronePipe< interface::GeoPoorPenetrationCandidatearray >			 * m_pPipeOutResult;		///< Out pipe 

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation

			int m_oMode;
			int m_oDisplay;
			int m_oWindowX;
			int m_oWindowY;
			int m_oAnzPosUnder;
			int m_oAnzPosUpper;
			int m_oLowerThreshold;
			int m_oUpperThreshold;
			int m_oMaxWidth;


			geo2d::PoorPenetrationCandidatearray m_oResultOut;

			bool m_hasPainting;

			PoorPenetrationOverlay _overlay;
		};

	} // namespace precitec
} // namespace filter

#endif /* PoorPenetration_H_ */
