/*!
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Stefan Birmanns (SB)
 *  @date			2017
 *	@brief			Special idm profile source filter that emits a line profile vector.
 */

#ifndef IDMPROFILESOURCE_H_
#define IDMPROFILESOURCE_H_

#include "fliplib/Fliplib.h"
#include "fliplib/SourceFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "geo/geo.h"
#include "geo/array.h"


namespace precitec {
namespace filter {

class FILTER_API IDMProfileSource : public fliplib::SourceFilter
{
public:
	IDMProfileSource();

	static const std::string m_oFilterName;
	static const std::string PIPENAME_LINEOUT;
    static const std::string PIPENAME_IMAGEOUT;


protected:
	void setParameter();
	bool subscribe(fliplib::BasePipe&, int);
	void proceed(const void*, fliplib::PipeEventArgs&);

private:
	const fliplib::SynchronePipe< interface::SampleFrame >	*m_pSampleIn;		///< In Connector,  SampleFrame
	fliplib::SynchronePipe< interface::GeoVecDoublearray >	m_oLinePipeOut;			///< Out Connector
    fliplib::SynchronePipe< interface::ImageFrame >	m_oImagePipeOut;			///< Out Connector


	geo2d::VecDoublearray									m_oLineOut;			///< Output laser line
};

} // namespace filter
} // namespace precitec

#endif /* IDMPROFILESOURCE_H_ */
