/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		
 *  @date		
 *  @brief		write laser-line objects to csv.
 */

#ifndef LINWRITE_H
#define LINWRITE_H

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

namespace precitec {
	using image::BImage;
	using interface::ImageFrame;
	using interface::Size2D;
	using interface::GeoVecDoublearray;
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief This filter writes to disk a 1D signal / laser line pipe.
 */
class FILTER_API LineWrite  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineWrite();
	/// DTor.
	virtual ~LineWrite();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	void setParameter();


protected:
	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// In-pipe event processing.
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

private:

	const fliplib::SynchronePipe< GeoVecDoublearray >* 	m_pPipeLineIn;    	///< In pipe
	fliplib::SynchronePipe< GeoVecDoublearray >*       	m_pPipeLineOut;		///< Out pipe
	int	 m_oStart;			///< Start index
	int	 m_oEnd;				///< End index
	int m_curNumber;
	std::string m_oOutputFolder;
	std::string m_oOutputFilename;
	geo2d::VecDoublearray m_oLineOut;			///< Output signal
	void write();

};

} // namespace precitec
} // namespace filter

#endif /* LINWRITE_H */
