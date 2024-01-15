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

#ifndef CONDITIONAL_H_
#define CONDITIONAL_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {

/**
 * @brief This is a conditional filter.
 */
class Conditional : public fliplib::TransformFilter
{
public:
    Conditional();
    virtual ~Conditional();
    static const std::string m_oFilterName;
    static const std::string m_oPipeOutDataName;
    static const std::string m_oPipeOutOtherDataName;
    static const std::string m_oPipeOutOperationResultName;
    static const std::string m_oParamOperation;
    void setParameter();
protected:
    bool subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup);
    void proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & p_rEvent);
protected:
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInDataA;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInDataB;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityA;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityB;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutDataOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutOperationResult;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutOtherDataOut;
    int m_oOperation;
    double m_lastValidValueDataA;
private:
	void SignalOperationResultOut(double operationResult, const interface::GeoDoublearray &rGeoDoubleArrayInData);
    void SetLastValidValueOfDataA();
    void SignalLastValidValueOfDataA();
    void ProceedCompare();
    void ProceedEncoderCompare();

}; // class Conditional

} // namespace filter
} // namespace precitec

#endif // CONDITIONAL_H_
