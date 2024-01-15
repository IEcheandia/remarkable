/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2015
 * 	@brief		This is a conditional filter.
 * 				Inputs: data_a, data_b, quality_a, quality_b
 * 				Outputs: data_out, compareResult
 * 				Compare Mode:
 * 								if quality_a > quality_b then data_out == data_a and operationResult = 1
 * 								if quality_a < quality_b then data_out == data_b and operationResult = -1
 * 								if quality_a == quality_b then data_out == data_a and Result = 1
 *
 * 	 	 	 	Encoder-Compare Mode:
 * 								if quality_a < quality_b then data_out == data_a and operationResult = 1
 * 								if quality_a >= quality_b then data_out == lastValid value of data_a and operationResult = 0
 *
 */

#ifndef CONDITIONALIMAGE_H_
#define CONDITIONALIMAGE_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

#include "common/frame.h"				// ImageFrame
#include "image/image.h"				// BImage

namespace precitec {
namespace filter {

/**
 * @brief This is a conditional filter.
 */
class ConditionalImage : public fliplib::TransformFilter
{
public:
	ConditionalImage();
	virtual ~ConditionalImage();
    static const std::string m_oFilterName;
    static const std::string m_oPipeOutDataName;
    static const std::string m_oPipeOutOperationResultName;
    //static const std::string m_oParamOperation;
    void setParameter();
protected:
    bool subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup);
    void proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & p_rEvent);
protected:
	const fliplib::SynchronePipe<interface::ImageFrame> *m_pPipeInImageA;
	const fliplib::SynchronePipe<interface::ImageFrame> *m_pPipeInImageB;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityA;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityB;
	fliplib::SynchronePipe<interface::ImageFrame> m_oPipeOutDataOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutOperationResult;
    //int m_oOperation;
    double m_lastValidValueDataA;
private:
	void SignalOperationResultOut(double operationResult, const interface::ImageFrame &rImageInData);
    //void SetLastValidValueOfDataA();
    //void SignalLastValidValueOfDataA();
    void ProceedCompare();
    //void ProceedEncoderCompare();

}; // class Conditional

} // namespace filter
} // namespace precitec

#endif // CONDITIONALIMAGE_H_
