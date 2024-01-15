#include <QTest>
#include "analyzer/sumError.h"
#include "analyzer/SignalSumError.h"
#include "analyzer/product.h"
#include <filter/armStates.h>

using namespace precitec;
using namespace precitec::analyzer;
using namespace precitec::interface;
using namespace precitec::geo2d;
class TestSumError : public QObject
{
    Q_OBJECT
public:
    using SumErrorParams = precitec::analyzer::Product::SumErrorParams;
    using ArmState = precitec::filter::ArmState;

    enum NioPercentage
    {
        Z, //Zero (0%)
        P, //Partial
        F  //Full (100%)
    };
    static void printSumErrorEvaluation(const precitec::interface::ResultDoubleArray& result, precitec::analyzer::ResultEvaluationData sumErrorResult);
    static std::vector<precitec::interface::ResultDoubleArray> generateResults(int trigger_um, precitec::interface::ResultType resultType, const std::vector<double>& values);
    static NioPercentage evaluateNioPercentage(precitec::analyzer::ResultEvaluationData sumErrorResult);

    void verifySumErrors(SmpSumError error, bool allowMultipleEvaluation, const std::vector<ResultDoubleArray>& geoInput, const std::vector<NioPercentage>& expectedNioPercentage);
private Q_SLOTS:
    void testCtor_data();
    void testCtor();
    void testSingleOutlierStaticBoundary_data();
    void testSingleOutlierStaticBoundary();
    void testAccumulatedOutlierStaticBoundary_data();
    void testAccumulatedOutlierStaticBoundary();
    void testDualOutlierStaticBoundary_data();
    void testDualOutlierStaticBoundary();
    void testSingleInlierStaticBoundary_data();
    void testSingleInlierStaticBoundary();
};


void TestSumError::printSumErrorEvaluation(const precitec::interface::ResultDoubleArray& result, precitec::analyzer::ResultEvaluationData sumErrorResult)
{
    auto nioPercentage = sumErrorResult.nioPercentage.empty() ?
            std::vector<geo2d::DPoint>(result.value().size(), {NAN, NAN}) : sumErrorResult.nioPercentage;
    auto signalQuality = sumErrorResult.signalQuality.empty() ?
            std::vector<float>(result.value().size(), NAN) : sumErrorResult.signalQuality;
    for (unsigned int i = 0; i < nioPercentage.size(); i++)
    {
        qDebug() << result.context().imageNumber() << ") Value " << result.value()[i]
            << "\t" << nioPercentage[i].x << "mm\t: " << nioPercentage[i].y << "%" << " "
            << " quality " << signalQuality[i];
    }
}

void TestSumError::verifySumErrors(SmpSumError error, bool allowMultipleEvaluation, const std::vector<ResultDoubleArray>& geoInput, const std::vector<NioPercentage>& expectedNioPercentage)
{
    auto signalSumError = dynamic_cast<SignalSumError*>(error.get());
    QVERIFY(signalSumError);
    signalSumError->allowMultipleEvaluation(allowMultipleEvaluation);

    ResultArgs dummyLWMTriggerSignal = {};

    int numFrames = geoInput.size();
    QCOMPARE(expectedNioPercentage.size(), numFrames);
    bool sumErrorAlreadyTriggered = false;

    for (int imageNumber = 0; imageNumber < numFrames; imageNumber++)
    {
        ResultDoubleArray result = geoInput[imageNumber];

        //qDebug() << result.context().imageNumber() << ") " << result.value().front();
        ResultEvaluationData sumErrorResult = error->testResult(result, dummyLWMTriggerSignal);
        //printSumErrorEvaluation(result, sumErrorResult);
        QVERIFY(sumErrorResult.evaluationStatus >= eSumErrorNotTriggered);

        if (sumErrorResult.evaluationStatus >= eSumErrorNotTriggered)
        {
            error->keepResultAsLastResult(result);
            error->trackScopeOnProduct(result);
        }

        auto actualPercentage = evaluateNioPercentage(sumErrorResult);
        QCOMPARE(int(actualPercentage), int(expectedNioPercentage[imageNumber]));
        auto expectedTriggered = (actualPercentage==NioPercentage::F) && (!sumErrorAlreadyTriggered || allowMultipleEvaluation);
        QCOMPARE((sumErrorResult.evaluationStatus == eSumErrorTriggered), expectedTriggered);
        if (sumErrorResult.evaluationStatus == eSumErrorTriggered)
        {
            sumErrorAlreadyTriggered = true;
        }
    }

}

std::vector<ResultDoubleArray> TestSumError::generateResults(int trigger_um, ResultType resultType, const std::vector<double>& inputValues)
{
    //create data in the format expected by the filters
    auto dummyResultFilterID = Poco::UUIDGenerator{}.createRandom();
    auto dummyProductUUID = Poco::UUIDGenerator{}.createRandom();
    auto dummyTaskId = Poco::UUIDGenerator{}.createRandom();
    auto dummyGraphId = Poco::UUIDGenerator{}.createRandom();
    int dummySerialNumber = 100;
    int seamseries = 0;
    int seam = 0;
    int seaminterval = 0;
    const auto triggerContext = TriggerContext{dummyProductUUID, dummySerialNumber, seamseries, seam, 0 /*imageNumber*/};
    auto taskContext = TaskContext(new Trafo, dummyProductUUID,
                                   new MeasureTask(dummyTaskId, dummyGraphId, seamseries, seam, seaminterval, 2 /*level*/, trigger_um));

    std::vector<ResultDoubleArray> geoInput;
    for (unsigned int imageNumber = 0; imageNumber < inputValues.size(); imageNumber++)
    {
        ImageContext context(triggerContext);
        context.setImageNumber(imageNumber);
        context.setPosition(trigger_um * imageNumber); //um
        context.setTaskContext(taskContext);

        GeoDoublearray value{context, Doublearray(1, inputValues[imageNumber], 255), AnalysisOK, 1.0};
        geoInput.emplace_back(dummyResultFilterID,
                                 resultType, ResultType::ValueOutOfLimits,
                                 context, value, precitec::geo2d::Range1d(0, 1), false);
    }
    return geoInput;

}

TestSumError::NioPercentage TestSumError::evaluateNioPercentage (precitec::analyzer::ResultEvaluationData sumErrorResult)
{
    NioPercentage value = NioPercentage::Z; //zero
    for (auto perc : sumErrorResult.nioPercentage)
    {
        if (perc.y >= 100)
        {
            value = NioPercentage::F; //full
            break;
        }
        if (perc.y > 0)
        {
            value = NioPercentage::P; //partial
        }
    }
    return value;
}

Q_DECLARE_METATYPE(ResultType);
void TestSumError::testCtor_data()
{
    QTest::addColumn<ResultType>("errorType");
    for (int errorType = 1501; errorType < 1522; errorType++)
    {
        if (errorType == 1519)
        {
            //LevelTwoErrorSelected doesn't seem to be implemented
            continue;
        }
        QTest::addRow("%d", errorType) << ResultType(errorType);
    }
}

void TestSumError::testCtor()
{
    Poco::UUID oErrorInstance;
    precitec::analyzer::Product::SumErrorParams params;
    params.m_oScope = eScopeSeam;
    SmpSumError error;

    //see Product::createSumErrors
    QFETCH(ResultType, errorType);
    const interface::ReferenceCurveSet referenceSet;
    //    QVERIFY(!referenceSet.m_middle.m_curve.empty());
    switch (errorType)
    {
        //---SIGNAL-SUM-ERRORS----

    case ResultType::SignalSumErrorAccumulatedOutlierStaticBoundary:
    case ResultType::SignalSumErrorAccumulatedAreaStaticBoundary:
    case ResultType::SignalSumErrorPeakStaticBoundary:
    {
        bool isAreaError = (errorType == SignalSumErrorAccumulatedAreaStaticBoundary);
        bool isPeakError = errorType == SignalSumErrorPeakStaticBoundary;
        error = SmpSumError(new SignalSumErrorAccumulatedOutlier(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError, isPeakError));
        break;
    }
    case ResultType::SignalSumErrorAccumulatedOutlierReferenceBoundary:
    case ResultType::SignalSumErrorAccumulatedAreaReferenceBoundary:
    case ResultType::SignalSumErrorPeakReferenceBoundary:
    {
        bool isAreaError = (errorType == SignalSumErrorAccumulatedAreaReferenceBoundary);
        bool isPeakError = (errorType == SignalSumErrorPeakReferenceBoundary);
        error = SmpSumError(new SignalSumErrorAccumulatedOutlier(referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError, isPeakError, params.m_oUseMiddleReference));
        break;
    }
    case ResultType::SignalSumErrorSingleOutlierStaticBoundary:
    case ResultType::SignalSumErrorSingleAreaStaticBoundary:
    {
        bool isAreaError = (errorType == SignalSumErrorSingleAreaStaticBoundary);
        error = SmpSumError(new SignalSumErrorSingleOutlier(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError));
        break;
    }

    case ResultType::SignalSumErrorSingleOutlierReferenceBoundary:
    case ResultType::SignalSumErrorSingleAreaReferenceBoundary:
    {
        bool isAreaError = (errorType == SignalSumErrorSingleAreaReferenceBoundary);
        error = SmpSumError(new SignalSumErrorSingleOutlier(referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError, params.m_oUseMiddleReference));
        break;
    }

    case ResultType::SignalSumErrorInlierStaticBoundary:
    {
        error = SmpSumError(new SignalSumErrorSingleInlier(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold));
        break;
    }
    case ResultType::SignalSumErrorInlierReferenceBoundary:
    {
        error = SmpSumError(new SignalSumErrorSingleInlier(referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oUseMiddleReference, params.m_oLwmSignalThreshold));
        break;
    }

    case ResultType::SignalSumErrorDualOutlierInRangeStaticBoundary:
    {
        error = SmpSumError(new SignalSumErrorDualOutlierInRange(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oSecondThreshold, params.m_oLwmSignalThreshold));
        break;
    }
    case ResultType::SignalSumErrorDualOutlierInRangeReferenceBoundary:
    {
        error = SmpSumError(new SignalSumErrorDualOutlierInRange(referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oSecondThreshold, params.m_oLwmSignalThreshold, params.m_oUseMiddleReference));
        break;
    }

        //---ANALYSIS-SUM-ERRORS-----
    case ResultType::AnalysisSumErrorAccumulatedOutlier:
    {
        error = SmpSumError(new analyzer::AnalysisSumErrorAccumulatedOutlier(params.m_oThreshold));
        break;
    }
    case ResultType::AnalysisSumErrorAdjacentOutlier:
    {
        error = SmpSumError(new AnalysisSumErrorSingleOutlier(params.m_oThreshold));
        break;
    }

        //---LEVEL-TWO-ERRORS-----
    case ResultType::LevelTwoErrorAccumulated:
    case ResultType::LevelTwoErrorErrorOnlyAccumulated:
    {
        bool p_oCheckSpecificError = (errorType == ResultType::LevelTwoErrorAccumulated);
        // set last parameter to true if we count NIO seams respect of the sumError value
        error = SmpSumError(new analyzer::LevelTwoErrorAccumulated((int)params.m_oThreshold, p_oCheckSpecificError));
        break;
    }
    case ResultType::LevelTwoErrorAdjacent:
    case ResultType::LevelTwoErrorErrorOnlyAdjacent:
    {
        bool p_oCheckSpecificError = (errorType == ResultType::LevelTwoErrorAdjacent);
        // set last parameter to true if we count NIO seams regardless of the sumError value
        error = SmpSumError(new analyzer::LevelTwoErrorAdjacent(params.m_oThreshold, p_oCheckSpecificError));
        break;
    }
    default:
    {
        QVERIFY2(false, "Invalid Sum Error ");
        break;
    }
    } // switch
    QVERIFY(!error.isNull());
    QVERIFY(error->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval));
    error->setGuid(oErrorInstance);
    error->setSumErrorType(errorType);

    if (errorType >= ResultType::SignalSumErrorAccumulatedOutlierStaticBoundary
        && errorType <= ResultType::SignalSumErrorDualOutlierInRangeReferenceBoundary)
    {
        QVERIFY(dynamic_cast<SignalSumError*>(error.get()) != nullptr);
    }
    if (errorType >= ResultType::AnalysisSumErrorAccumulatedOutlier
        && errorType <= ResultType::AnalysisSumErrorAdjacentOutlier)
    {
        QVERIFY(dynamic_cast<SignalSumError*>(error.get()) == nullptr);
    }
    if (errorType >= ResultType::LevelTwoErrorAccumulated
        && errorType <= ResultType::LevelTwoErrorErrorOnlyAdjacent)
    {
        QVERIFY(dynamic_cast<LevelTwoError*>(error.get()) != nullptr);
    }
}


Q_DECLARE_METATYPE(ResultDoubleArray)
Q_DECLARE_METATYPE(TestSumError::NioPercentage)
Q_DECLARE_METATYPE(TestSumError::SumErrorParams);

void TestSumError::testSingleOutlierStaticBoundary_data()
{
    QTest::addColumn<SumErrorParams>("params");
    QTest::addColumn<std::vector<ResultDoubleArray>>("geoInput");
    QTest::addColumn<std::vector<NioPercentage>>("expectedNioPercentage");
    QTest::addColumn<bool>("allowMultipleEvaluation");

    int trigger_um = 200;
    //at least 3 consecutive frames need to outside of the 2-6 range
    double param_min = 2.0;
    double param_max = 6.0;
    double threshold_mm = 3 * trigger_um / 1000.0;

    SumErrorParams params;
    params.m_oErrorType = ResultType::QualityFaultTypeA;

    params.m_oScope = eScopeSeam;
    params.m_oSeamseries = 0;
    params.m_oSeam = 0;
    params.m_oSeaminterval = 0;

    params.m_oThreshold = threshold_mm;
    //params.m_oSecondThreshold = 0.0; //unused with this SumError
    params.m_oResultType =  ResultType::CoordPositionX;;
    params.m_oMin = param_min;
    params.m_oMax = param_max;
    //params.m_oReferenceSet = {}; //unused in this test
    //params.m_oUseMiddleReference = true; //unused in this test
    params.m_oLwmSignalThreshold = 0.0; //unused in this test


    //                              out of range?   *           *  *  *     *     *  *  *
    std::vector<double> inputValues             = { 8, 3, 5, 4, 1, 0, 0, 5, 0, 5, 0, 0, 8};
    std::vector<NioPercentage> allowOnce        = { P, Z, Z, Z, P, P, F, Z, F, Z, F, F, F}; //after the first full nio, everything is immediately nio
    std::vector<NioPercentage> allowMultiple    = { P, Z, Z, Z, P, P, F, Z, P, Z, P, P, F};

    auto geoInput = generateResults(trigger_um, params.m_oResultType, inputValues);

    QTest::addRow("MultipleEvaluation") << params << geoInput << allowMultiple << true;
    QTest::addRow("EvaluateOnlyOnce")   << params << geoInput << allowOnce << false;

    //the same data, but oversampled
    std::vector<ResultDoubleArray> geoInputOversampled;
    for (const auto & result : geoInput)
    {
        GeoDoublearray value{result.context(), Doublearray(10, result.value().front(), 255), AnalysisOK, 1.0};
        geoInputOversampled.emplace_back(result.filterId(),
                                 result.resultType(), ResultType::ValueOutOfLimits,
                                 result.context(), value, precitec::geo2d::Range1d(0, 1), false);
    }

    QTest::addRow("MultipleEvaluationOversampled") << params << geoInputOversampled << allowMultiple << true;
    QTest::addRow("EvaluateOnlyOnceOversampled")   << params << geoInputOversampled << allowOnce << false;

}

void TestSumError::testSingleOutlierStaticBoundary()
{
    //simulate Product::createSumErrors, Product::seHandleResul

    QFETCH(SumErrorParams, params);
    QFETCH(std::vector<ResultDoubleArray>, geoInput);
    QFETCH(std::vector<NioPercentage>, expectedNioPercentage );
    QFETCH(bool, allowMultipleEvaluation);


    SmpSumError error{new SignalSumErrorSingleOutlier{params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, /*isAreaError*/ false}};
    QVERIFY(error->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval));
    error->setGuid(Poco::UUID("3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"));
    error->setSumErrorType(ResultType::SignalSumErrorSingleOutlierStaticBoundary);
    error->addResultToSurveil(params.m_oResultType);
    error->setErrorType(params.m_oErrorType);

    verifySumErrors(error, allowMultipleEvaluation, geoInput, expectedNioPercentage);
}

void TestSumError::testAccumulatedOutlierStaticBoundary_data()
{
    QTest::addColumn<SumErrorParams>("params");
    QTest::addColumn<std::vector<ResultDoubleArray>>("geoInput");
    QTest::addColumn<std::vector<NioPercentage>>("expectedNioPercentage");
    QTest::addColumn<bool>("allowMultipleEvaluation");

    int trigger_um = 200;
    //at least 3 frames in total need to outside of the 2-6 range
    double param_min = 2.0;
    double param_max = 6.0;
    double threshold_mm = 3 * trigger_um / 1000.0;

    SumErrorParams params;
    params.m_oErrorType = ResultType::QualityFaultTypeA;

    params.m_oScope = eScopeSeam;
    params.m_oSeamseries = 0;
    params.m_oSeam = 0;
    params.m_oSeaminterval = 0;

    params.m_oThreshold = threshold_mm;
    //params.m_oSecondThreshold = 0.0; //unused with this SumError
    params.m_oResultType =  ResultType::CoordPositionX;;
    params.m_oMin = param_min;
    params.m_oMax = param_max;
    //params.m_oReferenceSet = {}; //unused in this test
    //params.m_oUseMiddleReference = true; //unused in this test
    params.m_oLwmSignalThreshold = 0.0; //unused in this test

    //                              out of range?   *           *  *  *     *     *  *  *
    std::vector<double> inputValues             = { 8, 3, 5, 4, 1, 0, 0, 5, 0, 5, 0, 0, 8};
    std::vector<NioPercentage> allowOnce        = { P, Z, Z, Z, P, F, F, Z, F, Z, F, F, F}; //after the first full nio, everything is immediately nio
    std::vector<NioPercentage> allowMultiple    = { P, Z, Z, Z, P, F, P, Z, P, Z, F, P, P};

    auto geoInput = generateResults(trigger_um, params.m_oResultType, inputValues);

    QTest::addRow("MultipleEvaluation") << params << geoInput << allowMultiple << true;
    QTest::addRow("EvaluateOnlyOnce")   << params << geoInput << allowOnce << false;

    //the same data, but oversampled
    std::vector<ResultDoubleArray> geoInputOversampled;
    for (const auto & result : geoInput)
    {
        GeoDoublearray value{result.context(), Doublearray(10, result.value().front(), 255), AnalysisOK, 1.0};
        geoInputOversampled.emplace_back(result.filterId(),
                                 result.resultType(), ResultType::ValueOutOfLimits,
                                 result.context(), value, precitec::geo2d::Range1d(0, 1), false);
    }

    QTest::addRow("MultipleEvaluationOversampled") << params << geoInputOversampled << allowMultiple << true;
    QTest::addRow("EvaluateOnlyOnceOversampled")   << params << geoInputOversampled << allowOnce << false;
}

void TestSumError::testAccumulatedOutlierStaticBoundary()
{
    //simulate Product::createSumErrors, Product::seHandleResult

    QFETCH(SumErrorParams, params);
    QFETCH(std::vector<ResultDoubleArray>, geoInput);
    QFETCH(std::vector<NioPercentage>, expectedNioPercentage );
    QFETCH(bool, allowMultipleEvaluation);

    SmpSumError error{new SignalSumErrorAccumulatedOutlier{params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, /*isAreaError*/ false, /*isPeakError*/ false}};
    QVERIFY(error->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval));
    error->setSumErrorType(ResultType::SignalSumErrorAccumulatedOutlierStaticBoundary);
    error->addResultToSurveil(params.m_oResultType);
    error->setErrorType(params.m_oErrorType);

    verifySumErrors(error, allowMultipleEvaluation, geoInput, expectedNioPercentage);
}

void TestSumError::testDualOutlierStaticBoundary_data()
{
    QTest::addColumn<SumErrorParams>("params");
    QTest::addColumn<std::vector<ResultDoubleArray>>("geoInput");
    QTest::addColumn<std::vector<NioPercentage>>("expectedNioPercentage");
    QTest::addColumn<bool>("allowMultipleEvaluation");

    int trigger_um = 200;
    //2 sectors of at least 3 frames need to outside of the 2-6 range, with less than 2 frames between each other
    double param_min = 2.0;
    double param_max = 6.0;
    double thresholdError_mm = 3 * trigger_um / 1000.0;
    double resettingThreshold_mm = 2 * trigger_um / 1000.0;

    SumErrorParams params;
    params.m_oErrorType = ResultType::QualityFaultTypeA;

    params.m_oScope = eScopeSeam;
    params.m_oSeamseries = 0;
    params.m_oSeam = 0;
    params.m_oSeaminterval = 0;

    params.m_oThreshold = thresholdError_mm;
    params.m_oSecondThreshold = resettingThreshold_mm;
    params.m_oResultType =  ResultType::CoordPositionX;;
    params.m_oMin = param_min;
    params.m_oMax = param_max;
    //params.m_oReferenceSet = {}; //unused in this test
    //params.m_oUseMiddleReference = true; //unused in this test
    params.m_oLwmSignalThreshold = 0.0; //unused in this test

    //                                          out of range?           *           *  *  1*   *0     *  *  1*    *  *  2* *  *  *  *  *
    std::vector<double> inputValues                                 = { 8, 3, 5, 4, 1, 0, 0, 5, 0, 5, 0, 0, 8, 5, 0, 0, 0, 8, 9, 8, 7, 9};
    std::vector<NioPercentage> expectedPercentOnce                  = { P, Z, Z, Z, P, P, P, P, P, Z, P, P, P, P, P, P, F, F, F, F, F, F};
    std::vector<NioPercentage> expectedPercentOnce_oversampled      = { P, Z, Z, Z, P, P, P, P, P, P, P, P, P, P, P, P, F, F, F, F, F, F};
    std::vector<NioPercentage> expectedPercentMultiple              = { P, Z, Z, Z, P, P, P, P, P, Z, P, P, P, P, P, P, F, P, P, F, P, P};
    std::vector<NioPercentage> expectedPercentMultiple_oversampled  = { P, Z, Z, Z, P, P, P, P, P, P, P, P, P, P, P, P, F, P, P, F, P, P};

    auto geoInput = generateResults(trigger_um, params.m_oResultType, inputValues);

    QTest::addRow("EvaluateOnlyOnce")   << params << geoInput << expectedPercentOnce << false;
    QTest::addRow("MultipleEvaluation") << params << geoInput << expectedPercentMultiple << true;

    //the same data, but oversampled
    std::vector<ResultDoubleArray> geoInputOversampled;
    for (const auto & result : geoInput)
    {
        GeoDoublearray value{result.context(), Doublearray(10, result.value().front(), 255), AnalysisOK, 1.0};
        geoInputOversampled.emplace_back(result.filterId(),
                                 result.resultType(), ResultType::ValueOutOfLimits,
                                 result.context(), value, precitec::geo2d::Range1d(0, 1), false);
    }

    QTest::addRow("EvaluateOnlyOnceOversampled")   << params << geoInputOversampled << expectedPercentOnce_oversampled << false;
    QTest::addRow("MultipleEvaluationOversampled")   << params << geoInputOversampled << expectedPercentMultiple_oversampled << true;

}

void TestSumError::testDualOutlierStaticBoundary()
{
    //simulate Product::createSumErrors, Product::seHandleResul

    QFETCH(SumErrorParams, params);
    QFETCH(std::vector<ResultDoubleArray>, geoInput);
    QFETCH(std::vector<NioPercentage>, expectedNioPercentage );
    QFETCH(bool, allowMultipleEvaluation);


    SmpSumError error{new SignalSumErrorDualOutlierInRange{params.m_oMin, params.m_oMax, params.m_oThreshold, params.m_oSecondThreshold, params.m_oLwmSignalThreshold}};
    QVERIFY(error->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval));
    error->setSumErrorType(ResultType::SignalSumErrorSingleOutlierStaticBoundary);
    error->addResultToSurveil(params.m_oResultType);
    error->setErrorType(params.m_oErrorType);

    verifySumErrors(error, allowMultipleEvaluation, geoInput, expectedNioPercentage);
}

void TestSumError::testSingleInlierStaticBoundary_data()
{
    QTest::addColumn<SumErrorParams>("params");
    QTest::addColumn<std::vector<ResultDoubleArray>>("geoInput");
    QTest::addColumn<std::vector<NioPercentage>>("expectedNioPercentage");
    QTest::addColumn<bool>("expectedNioAtSeamEnd");
    QTest::addColumn<bool>("allowMultipleEvaluation");

    int trigger_um = 200;
    //error if not at least 3 consecutive frames inside 2-6 range
    double param_min = 2.0;
    double param_max = 6.0;
    double threshold_mm = 3 * trigger_um / 1000.0;

    SumErrorParams params;
    params.m_oErrorType = ResultType::QualityFaultTypeA;

    params.m_oScope = eScopeSeam;
    params.m_oSeamseries = 0;
    params.m_oSeam = 0;
    params.m_oSeaminterval = 0;

    params.m_oThreshold = threshold_mm;
    //params.m_oSecondThreshold = 0.0; //unused with this SumError
    params.m_oResultType =  ResultType::CoordPositionX;;
    params.m_oMin = param_min;
    params.m_oMax = param_max;
    //params.m_oReferenceSet = {}; //unused in this test
    //params.m_oUseMiddleReference = true; //unused in this test
    params.m_oLwmSignalThreshold = 0.0; //unused in this test

    //nio percentage is 0 when "completely" in range, or when out of range

    {
        //                              out of range?   *           *  *  *     *               
        std::vector<double> inputValues             = { 8, 3, 5, 4, 1, 0, 0, 5, 0, 5, 4, 5, 3, 4};
        std::vector<NioPercentage> allowOnce        = { Z, P, P, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}; //after the first full nio, not evaluated anymore
        std::vector<NioPercentage> allowOnce_overs  = { Z, P, P, P, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}; //after the first full nio, everything is just io

        bool expectedNioAtSeamEnd = false;
        auto geoInput = generateResults(trigger_um, params.m_oResultType, inputValues);

        QTest::addRow("EvaluateOnlyOnce")   << params << geoInput << allowOnce << expectedNioAtSeamEnd << false;
        QTest::addRow("MultipleEvaluation") << params << geoInput << allowOnce << expectedNioAtSeamEnd << false;

        //the same data, but oversampled
        std::vector<ResultDoubleArray> geoInputOversampled;
        for (const auto & result : geoInput)
        {
            GeoDoublearray value{result.context(), Doublearray(10, result.value().front(), 255), AnalysisOK, 1.0};
            geoInputOversampled.emplace_back(result.filterId(),
                                    result.resultType(), ResultType::ValueOutOfLimits,
                                    result.context(), value, precitec::geo2d::Range1d(0, 1), false);
        }

        QTest::addRow("EvaluateOnlyOnceOversampled")   << params << geoInputOversampled << allowOnce_overs << expectedNioAtSeamEnd << false;
        QTest::addRow("MultipleEvaluationOversampled") << params << geoInputOversampled << allowOnce_overs << expectedNioAtSeamEnd << false;
    }
    {
        //                                                   all inliers
        std::vector<double> inputValues                   = { 5, 5, 5, 5, 5, 5, 5};
        std::vector<NioPercentage> percentage             = { P, P, Z, Z, Z, Z, Z};  //nio percentage is 0 when "completely" in range, or when out of range
        std::vector<NioPercentage> percentage_oversampled = { P, P, P, Z, Z, Z, Z};  //nio percentage is 0 when "completely" in range, or when out of range

        auto geoInput = generateResults(trigger_um, params.m_oResultType, inputValues);
        bool expectedNioAtSeamEnd = false;
        for (bool allowMultipleEvaluation : {false, true})
        {
            QTest::addRow("all_inliers_%d", int(allowMultipleEvaluation)) << params << geoInput << percentage << expectedNioAtSeamEnd << allowMultipleEvaluation;
        }
        //the same data, but oversampled
        std::vector<ResultDoubleArray> geoInputOversampled;
        for (const auto & result : geoInput)
        {
            GeoDoublearray value{result.context(), Doublearray(10, result.value().front(), 255), AnalysisOK, 1.0};
            geoInputOversampled.emplace_back(result.filterId(),
                                    result.resultType(), ResultType::ValueOutOfLimits,
                                    result.context(), value, precitec::geo2d::Range1d(0, 1), false);
        }
        for (bool allowMultipleEvaluation : {false, true})
        {
            QTest::addRow("all_inliers_oversampled_%d", int(allowMultipleEvaluation)) << params << geoInputOversampled << percentage_oversampled << expectedNioAtSeamEnd << allowMultipleEvaluation;
        }
    }

    {
        //                                       all outliers
        std::vector<double> inputValues       = { 0, 0, 0, 0, 0, 0, 0};
        std::vector<NioPercentage> percentage = { Z, Z, Z, Z, Z, Z, Z};  //nio percentage is 0 when "completely" in range, or when out of range

        auto geoInput = generateResults(trigger_um, params.m_oResultType, inputValues);
        bool expectedNioAtSeamEnd = true;

        for (bool allowMultipleEvaluation : {false, true})
        {
            QTest::addRow("all_outliers_%d", int(allowMultipleEvaluation)) << params << geoInput << percentage << expectedNioAtSeamEnd << allowMultipleEvaluation;
        }
        //the same data, but oversampled
        std::vector<ResultDoubleArray> geoInputOversampled;
        for (const auto & result : geoInput)
        {
            GeoDoublearray value{result.context(), Doublearray(10, result.value().front(), 255), AnalysisOK, 1.0};
            geoInputOversampled.emplace_back(result.filterId(),
                                    result.resultType(), ResultType::ValueOutOfLimits,
                                    result.context(), value, precitec::geo2d::Range1d(0, 1), false);
        }
        std::vector<NioPercentage> percentage_oversampled  = { Z, Z, Z, Z, Z, Z, Z};
        for (bool allowMultipleEvaluation : {false, true})
        {
            QTest::addRow("all_outliers_oversampled_%d", int(allowMultipleEvaluation)) << params << geoInputOversampled << percentage << expectedNioAtSeamEnd << allowMultipleEvaluation;
        }
    }
}

void TestSumError::testSingleInlierStaticBoundary()
{
    //simulate Product::createSumErrors, Product::seHandleResul

    QFETCH(SumErrorParams, params);
    QFETCH(std::vector<ResultDoubleArray>, geoInput);
    QFETCH(std::vector<NioPercentage>, expectedNioPercentage );
    QFETCH(bool, expectedNioAtSeamEnd);
    QFETCH(bool, allowMultipleEvaluation);


    SmpSumError error{new SignalSumErrorSingleInlier{params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold}};
    QVERIFY(error->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval));
    error->setSumErrorType(ResultType::SignalSumErrorSingleOutlierStaticBoundary);
    error->addResultToSurveil(params.m_oResultType);
    error->setErrorType(params.m_oErrorType);

    auto sumErrorResult = error->arm(ArmState::eSeamStart);
    QVERIFY(sumErrorResult == nullptr);
    sumErrorResult = error->arm(ArmState::eSeamIntervalStart);
    QVERIFY(sumErrorResult == nullptr);

    verifySumErrors(error, allowMultipleEvaluation, geoInput, expectedNioPercentage);
    
    sumErrorResult = error->arm(ArmState::eSeamIntervalEnd);
    if (expectedNioAtSeamEnd)
    {
        QVERIFY(sumErrorResult != nullptr);
        QCOMPARE(sumErrorResult->isNio(), true);
        QCOMPARE(sumErrorResult->resultType(), params.m_oErrorType);
    }
    else
    {
        QVERIFY(sumErrorResult == nullptr || !sumErrorResult->isNio());
    }
    sumErrorResult = error->arm(ArmState::eSeamEnd);
    if (expectedNioAtSeamEnd)
    {
        QVERIFY(sumErrorResult != nullptr);
        QCOMPARE(sumErrorResult->isNio(), true);
        QCOMPARE(sumErrorResult->resultType(), params.m_oErrorType);
    }
    else
    {
        QVERIFY(sumErrorResult == nullptr);
    }
}

QTEST_GUILESS_MAIN(TestSumError)
#include "testSumError.moc"
