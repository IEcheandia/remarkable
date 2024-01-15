/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief
 */

#include "histogram.h"

#include "image/image.h"
#include "overlay/overlayPrimitive.h"
#include "module/moduleLogger.h"
#include "geo/array.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {



const std::string Histogram::m_oFilterName 		= std::string("Histogram");
const std::string Histogram::PIPENAME			= std::string("ImageFrame");
const std::string Histogram::PIPENAME_MINIMUM	= std::string("Minimum");
const std::string Histogram::PIPENAME_MAXIMUM	= std::string("Maximum");

// Die urspruenglichen Grauwerte des Datenvolumens werden gleichmaessig auf das Intervall von Null
// bis zum maximal moeglichen Grauwert gmax verteilt. Dazu wird eine Look-Up-Table aufgebaut,
// aus der die neuen Grauwerte f(g) ausgelesen werden und die alten Grauwerte g ersetzen.

Histogram::Histogram() :
	TransformFilter		( Histogram::m_oFilterName, Poco::UUID{"83562932-1260-4a20-9D8E-2252828C688E"} ),
	m_pPipeInImageFrame	(NULL),
	pipeImageFrame_		( new SynchronePipe< ImageFrame >( this, Histogram::PIPENAME ) ),
	pipeMinimum_		( new SynchronePipe< GeoDoublearray >( this, Histogram::PIPENAME_MINIMUM ) ),
	pipeMaximum_		( new SynchronePipe< GeoDoublearray >( this, Histogram::PIPENAME_MAXIMUM ) )
{
	// Defaultwerte der Parameter setzen
	//parameters_.add("Gmax", Parameter::TYPE_int, 800);
    setInPipeConnectors({{Poco::UUID("13CB4C94-2D87-4c8a-AF3D-1E320AC211D3"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("FA84C716-9ED3-4047-A44B-1512D3AB81B9"), pipeImageFrame_, "ImageFrame", 0, ""},
    {Poco::UUID("23567442-D065-4496-A504-CEA9A842AF56"), pipeMinimum_, "Minimum", 0, ""},
    {Poco::UUID("004F2FEE-59D5-44e2-93B8-398EE22EBA7A"), pipeMaximum_, "Maximum", 0, ""}});
    setVariantID(Poco::UUID("15F410C4-C1E6-441a-B265-25E92FAEF890"));
}

Histogram::~Histogram()
{
	delete pipeImageFrame_;
	delete pipeMinimum_;
	delete pipeMaximum_;
}


void Histogram::setParameter()
{
	TransformFilter::setParameter();
}

void Histogram::paint()
{
	if(m_oVerbosity <= eNone){
		return;
	} // if

	OverlayCanvas&			rOverlayCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer			&rLayerLine				( rOverlayCanvas.getLayerLine());
	OverlayLayer & rLayerText(rOverlayCanvas.getLayerText());


	// Das Histogram wird verkleinert ins Bild gemalt
	int width = int((rOverlayCanvas.width() - 10) / 2);
	int height = int(width / 1.3333333);

	// Rahmen zeichnen
	geo2d::Rect border (Point(10,10), Size(width, height));
	rLayerLine.add( new OverlayRectangle(border, Color::Red() ) );

	if (maxValue_ > 0)
	{
		// Box um ein Pixel verkleinern
		geo2d::Rect field(border.x().start() +1, border.y().start() +1, border.width() - 2, border.height() - 2);

		if (field.width() > 0)
		{
			// Histogram malen
			double f = double(LOOKUP_LENGTH) / double(field.width());
			int y = field.y().end();
			int h = field.height();
			int idx = -1;
			for (int x=field.x().start(), j=0; x<field.x().end();++x,++j)
			{
				// index in Lookup
				int i = int(j*f);
				if (idx != i && i >= 0 && i < LOOKUP_LENGTH && lookup_[i] != 0 )
				{
					//std::cout << "y1:" << y- (h * lookup_[i] / maxValue_) <<  " y:" << y << " j:" << j << " lookup_[i]:" << lookup_[i] << " maxValue:" << maxValue_ << " x:" << x << " y:" << y << " field:" << field << std::endl;
					rLayerLine.add<OverlayLine> (
							x, y, x, y - (h * lookup_[i] / maxValue_),
							Color::Blue()
							);
				}
				if (i % 32 == 0 || i== LOOKUP_LENGTH)
				{
					std::string oTxt = std::to_string(i) + "[" + std::to_string(lookup_[i]) + "]";
					rLayerText.add<OverlayText>(oTxt, Font(), Rect(x, y - (h * lookup_[i] / maxValue_),60,30), Color::Orange());
				}
				idx = i;

			}
		}

	}
}


bool Histogram::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}



void Histogram::proceed(const void* sender, PipeEventArgs& e)
{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// Empfangenes Frame auslesen
	const ImageFrame &rFrame (m_pPipeInImageFrame->read(m_oCounter));
	m_oSpTrafo	= rFrame.context().trafo();

	BImage image = rFrame.data();
	Doublearray	oMin	(1, minValue_);
	Doublearray	oMax	(1, maxValue_);

	// input validity check

	if ( ! image.isValid() ) {
		preSignalAction();
		pipeMinimum_->signal( GeoDoublearray(rFrame.context(), oMin, rFrame.analysisResult(), 0.0) ); // bad rank
		pipeMaximum_->signal( GeoDoublearray(rFrame.context(), oMax, rFrame.analysisResult(), 0.0) ); // bad rank

		return; // RETURN
	}


	// Lookuptabelle erzeugen
	memset(lookup_, '\0', sizeof(lookup_));

    image.for_each([this](byte pixel){++(lookup_[pixel]);});


	// Min und Max suchen und als Resultate verschicken
	minValue_ = std::numeric_limits<int>::max();
	maxValue_ = std::numeric_limits<int>::min();

	for(unsigned int i=0; i < LOOKUP_LENGTH; ++i)
	{
		minValue_ = std::min(minValue_, lookup_[i]);
		maxValue_ = std::max(maxValue_, lookup_[i]);
	}

	oMin.getData().front()	= minValue_;
	oMax.getData().front()	= maxValue_;
	oMin.getRank().front()	= eRankMax;
	oMax.getRank().front()	= eRankMax;

	const auto oAnalysisResult	= rFrame.analysisResult() == AnalysisOK ? AnalysisOK : rFrame.analysisResult(); // replace 2nd AnalysisOK by your result type

	preSignalAction();
	pipeImageFrame_->signal( rFrame );// altes Frame vesenden
	pipeMinimum_->signal( GeoDoublearray(rFrame.context(), oMin, oAnalysisResult, 1.0) ); // full rank
	pipeMaximum_->signal( GeoDoublearray(rFrame.context(), oMax, oAnalysisResult, 1.0) ); // full rank

}

}}

