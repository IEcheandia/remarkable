/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief
 */

#ifndef LINETRACKING_H_
#define LINETRACKING_H_

#include <iostream>

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"
#include "geo/geo.h"

#include "laserlineTracker1.h"

#include "souvisSourceExportedTypes.h"

namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {


/**
* @brief Line tracking filter
* beinhaltet die proceed methode und das trackng ergebniss
* in einer result Klasse und im Ergebniss vekctor LaserLineOut
*/
class FILTER_API LineTracking  : public fliplib::TransformFilter
{
public:
	LineTracking();
	virtual ~LineTracking();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME1;
	static const std::string PIPENAME2;

	void setParameter();
	void paint();
	bool ic_Initialized;
	void arm(const fliplib::ArmStateBase& state);	///< arm filter

protected:
    /// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

private:

    typedef fliplib::SynchronePipe< interface::ImageFrame >			image_pipe_t;

    const image_pipe_t* m_pPipeInImageFrame;    ///< in pipe

	fliplib::SynchronePipe< GeoVecDoublearray >* pipeResY_;	//output: Neue Struktur Y mit Context

	geo2d::VecDoublearray		m_oLaserlineOutY;	///< output laser line, dieser Vector wird weitergegeben
	geo2d::VecDoublearray		m_oLaserlineOutI;	///< output laser line


	interface::SmpTrafo		m_oSpTrafo;				///< roi translation

	LaserlineTracker1T		laserlineTracker_;

	Size2d imageSize_;
	//int markcolor_;
	int trackstart_;
	int suchschwelle_;
	int doubleTracking_;
	int upperLower_;
	int pixelx_;
	int pixely_;
    int pixelYLower_;
	int mittelungy_;
	int aufloesungx_;
	int aufloesungy_;
	int MaxBreiteUnterbruch_;
	int MaxAnzahlUnterbrueche_;
	int MaxLinienspringy_;

	int startAreaX_;
	int startAreaY_;
};

}}

#endif /*LINETRACKING_H_*/
