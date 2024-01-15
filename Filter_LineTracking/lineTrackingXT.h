/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		
 */

#ifndef LINETRACKINGXT_H_
#define LINETRACKINGXT_H_

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
class FILTER_API LineTrackingXT  : public fliplib::TransformFilter
{
public:
	LineTrackingXT();
	virtual ~LineTrackingXT();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME1;

	void setParameter();
	void paint();
	bool ic_Initialized;
	void arm(const fliplib::ArmStateBase& state);	///< arm filter

protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	// Verarbeitungsmethode IN Pipe
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);
	//void proceed(const void* sender, fliplib::PipeEventArgs& e);

private:
	typedef fliplib::SynchronePipe< interface::ImageFrame >			image_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;

	const image_pipe_t*	        m_pPipeInImageFrame;		///< in pipe
	const scalar_pipe_t*		m_pPipeInSearchThreshold;	///< in pipe Search threshold
	const scalar_pipe_t*		m_pPipeInTrackStart;	    ///< in pipe Track start
	const scalar_pipe_t*		m_pPipeInUpperLower;	    ///< in pipe Line geometry upper/lower

	fliplib::SynchronePipe< GeoVecDoublearray >* pipeResY_;	//output: Neue Struktur Y mit Context
	
	geo2d::VecDoublearray		m_oLaserlineOutY;	///< output laser line, dieser Vector wird weitergegeben
	geo2d::VecDoublearray		m_oLaserlineOutI;	///< output laser line

	
	interface::SmpTrafo		m_oSpTrafo;				///< roi translation

	LaserlineTracker1T		laserlineTracker_;

	Size2d imageSize_;
	//int markcolor_;
	int debuglevel_;
	int trackstart_;
	int suchschwelle_;
	int doubleTracking_;
	int upperLower_;
	int pixelx_;
	int pixely_;
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

#endif /*LINETRACKINGXT_H_*/
