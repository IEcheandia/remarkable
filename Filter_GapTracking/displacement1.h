/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Holger Kirschner (KiH), Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Moves a GeoPoint.
 */

#ifndef CONTROL_DISPLACEMENT_1_H_
#define CONTROL_DISPLACEMENT_1_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework.
#include <fliplib/TransformFilter.h>	///< Baseclass.
#include <fliplib/PipeEventArgs.h>		///< Events.
#include <fliplib/SynchronePipe.h>		///< Pipes.
#include <geo/geo.h>					///< Geo data types.

namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {

/**
 * @ingroup Filter_SampleSource
 * @brief This filter takes a GeoPoint/GeoPointarry and moves it to another pixel position: Xnew = Xold - TargetX, Ynew = 0.
 */
class FILTER_API displacement: public fliplib::TransformFilter {
public:
	displacement();
	virtual ~displacement();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME_POSX;
	static const std::string PIPENAME_POSY;

	/// Set filter parameters defined in database / xml file.
	void setParameter();
	void paint();

protected:

	/// In pipe registration.
	virtual bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:
	
	fliplib::SynchronePipe< GeoDoublearray >* 	m_pPipeArrayX;	///< Pointer to input pipe.
	fliplib::SynchronePipe< GeoDoublearray >* 	m_pPipeArrayY;	///< Pointer to input pipe.
	fliplib::SynchronePipe< GeoDoublearray >* 	m_pPipeOutPosX;	///< Pointer to output pipe.
	fliplib::SynchronePipe< GeoDoublearray >* 	m_pPipeOutPosY;	///< Pointer to output pipe.

	interface::SmpTrafo							m_oSpTrafo;		///< roi translation
	double 										m_oTargetX;		///< Target point, x coordinate.
	double 										m_oTargetY;		///< Target point, y coordinate (not used, y coord is set to zero).
	geo2d::Doublearray							m_oXOut;		///< Out x coordinate
	geo2d::Doublearray							m_oYOut;		///< Out y coordinate
};

} // namespace filter
} // namespace precitec

#endif /*displacement_H_*/
