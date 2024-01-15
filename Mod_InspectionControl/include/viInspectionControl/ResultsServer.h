/*
 * ResultsServer.h
 *
 *  Created on: 03.04.2017
 *      Author: a.egger
 */

#ifndef RESULTCONTROL_H_
#define RESULTCONTROL_H_

#include "event/results.interface.h"

#include "VI_InspectionControl.h"

#define NUMBER_RESULTS_FOR_OK     5

namespace precitec
{
    const int CS_MAX_RESULT_BLOCKS = 10;

	using namespace interface;

namespace ethercat{

/**
 * ResultServer, hier schlagen die Resultate auf.
 *
 * @attention Hier schlagen ALLE abgesendeten Resultate auf (parallel zum Windows Host) -> PERFORMANCE?
 *
 **/
class ResultsServer  : public TResults<AbstractInterface>{
public:

	/**
	 * Ctor
	 * @param weldingHeadControl weldHead-Publisher
	 * @param axisLearn Achslern-Objekt
	 * @param p_rOutDataPub Publisher zur Ausgabe der Results
	 * @return void
	 **/
	ResultsServer(VI_InspectionControl& p_rVI_InspectionControl);
	virtual ~ResultsServer();

private:

	void result(interface::ResultIntArray const&) override;
	void result(interface::ResultDoubleArray const&) override;
	void result(interface::ResultRangeArray const&) override;
	void result(interface::ResultRange1dArray const&) override;
	void result(interface::ResultPointArray const&) override;
	void nio(interface::NIOResult const&) override;
	void inspectionStarted(Poco::UUID productID, Poco::UUID instanceProductID, uint32_t productNr, MeasureTaskIDs measureTaskIDs, IntRange range, int p_oSeamNo , int triggerDeltaInMicrons, Poco::UUID seamId, const std::string &seamLabel) override;
	void inspectionEnded(MeasureTaskIDs measureTaskIDs) override;
	void inspectionAutomaticStart(Poco::UUID productID, Poco::UUID instanceProductID, const std::string &extendedProductInfo) override;
	void inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID) override;

    void S6K_CS_CheckForTimeDuration(int p_oImageNumber);
    void S6K_CS_CheckForBufferFull(void);
    void S6K_CS_CheckForLastBuffer(void);

    bool isSOUVIS6000_Application(void) { return m_oIsSOUVIS6000_Application; }
    bool isSOUVIS6000_Is_PreInspection(void) { return m_oSOUVIS6000_Is_PreInspection; }
    bool isSOUVIS6000_Is_PostInspection_Top(void) { return m_oSOUVIS6000_Is_PostInspection_Top; }
    bool isSOUVIS6000_Is_PostInspection_Bottom(void) { return m_oSOUVIS6000_Is_PostInspection_Bottom; }
    bool isSOUVIS6000_CrossSectionMeasurementEnable(void) { return m_oSOUVIS6000_CrossSectionMeasurementEnable; }
    bool isSOUVIS6000_CrossSection_Leading_System(void) { return m_oSOUVIS6000_CrossSection_Leading_System; }

	VI_InspectionControl &m_rVI_InspectionControl;
	unsigned long m_oResultCounter;
	bool m_oOK;

    bool m_oIsSOUVIS6000_Application;
    bool m_oSOUVIS6000_Is_PreInspection;
    bool m_oSOUVIS6000_Is_PostInspection_Top;
    bool m_oSOUVIS6000_Is_PostInspection_Bottom;
    bool m_oSOUVIS6000_CrossSectionMeasurementEnable;
    bool m_oSOUVIS6000_CrossSection_Leading_System;

    int32_t m_oSeamNoIndex;
    uint32_t m_oQualityErrorAccu;
    uint32_t m_oQualityErrorCat1Accu;
    uint32_t m_oQualityErrorCat2Accu;
    int32_t m_oFirstErrorPosition;
    uint64_t m_oElementWidthSum;
    uint32_t m_oElementWidthCounter;
    uint64_t m_oHeightDiffSum;
    uint32_t m_oHeightDiffCounter;
    uint64_t m_oConcavitySum;
    uint32_t m_oConcavityCounter;
    uint64_t m_oConvexitySum;
    uint32_t m_oConvexityCounter;
    int32_t m_oFirstPoreWidth;

    int m_oS6K_CS_ActualResultBlock;
    uint16_t m_oS6K_CS_MaxResultCountInBlock;
    int m_oS6K_CS_CurrentNoResult[CS_VALUES_PER_MEASURE];
    int m_oS6K_CS_FirstImageInBlock[CS_MAX_RESULT_BLOCKS];
    CS_BlockType m_oS6K_CS_ResultBlock[CS_MAX_RESULT_BLOCKS];
    int m_oS6K_CS_ResultCountPerImage[CS_MAX_RESULT_BLOCKS][CS_MEASURE_COUNT_PER_TCP_BLOCK];
    uint16_t m_oS6K_CS_BlockNo;
    uint16_t m_oS6K_CS_MeasuresPerResult;
    int m_S6K_CS_resultsPerImage{1};

    bool m_oS6K_CS_SeamStartFlag;
    std::chrono::time_point<std::chrono::steady_clock> m_oS6K_CS_StartTime;
    std::chrono::time_point<std::chrono::steady_clock> m_oS6K_CS_CurrentTime;
};

} // namespace ethercat
} // namespace precitec

#endif /* RESULTSERVER_H_ */

