#include "../spotWeldingResult.h"
#include <iostream>
#include <QTest>
#include <fliplib/BaseFilter.h>
#include <fliplib/NullSourceFilter.h>
#include <optional>
#include "DummyResultHandler.h"


using precitec::interface::ResultType;
using precitec::interface::ResultDoubleArray;
using precitec::interface::GeoDoublearray;
using precitec::filter::SpotWeldingResult;
using precitec::filter::eRankMax;
using precitec::filter::eRankMin;
using precitec::geo2d::Doublearray;

class TestSpotWeldingResult: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
};


struct DummyInput
{
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<GeoDoublearray> m_pipeInLaserPowerCenter;
    std::optional<fliplib::SynchronePipe<GeoDoublearray>> m_pipeInLaserPowerRing;
    fliplib::SynchronePipe<GeoDoublearray> m_pipeInWeldingDuration;

    GeoDoublearray m_powerCenterIn;
    GeoDoublearray m_powerRingIn;
    GeoDoublearray m_durationIn;

    DummyInput():
    m_pipeInLaserPowerCenter{&m_sourceFilter, "laserPowerCenterIn"}
    , m_pipeInWeldingDuration{&m_sourceFilter, "weldingDurationIn"}
    {
    }

    void setPowerRing()
    {
        m_pipeInLaserPowerRing.emplace(&m_sourceFilter, "optionalLaserPowerRingIn");
    }

    bool connectToFilter(fliplib::BaseFilter* filter)
    {
        m_pipeInLaserPowerCenter.setTag("laserPowerCenterIn");
        m_pipeInWeldingDuration.setTag("weldingDurationIn");
        int group{1};
        //connect  pipes
        bool ok = filter->connectPipe(&(m_pipeInLaserPowerCenter), group);
        ok &= filter->connectPipe(&(m_pipeInWeldingDuration), group);
        if (m_pipeInLaserPowerRing.has_value())
        {
            m_pipeInLaserPowerRing.value().setTag("optionalLaserPowerRingIn");
            ok &= filter->connectPipe(&(m_pipeInLaserPowerRing.value()), group);
        }
        return ok;
    }

    void fillData(int imageNumber, double laserPowerCenter, int weldingDuration, std::optional<double> laserPowerRing, int rank)
    {
        using namespace precitec::interface;

        ImageContext context;
        context.setImageNumber(imageNumber);
        context.m_ScannerInfo = ScannerContextInfo{true, 0, 0};

        m_powerCenterIn = GeoDoublearray
        {
            context,
            Doublearray(1, laserPowerCenter, rank),
            ResultType::AnalysisOK,
            Limit
        };

        m_durationIn = GeoDoublearray 
        {
            context,
            Doublearray(1, weldingDuration, rank),
            ResultType::AnalysisOK, 
            Limit
        };

        if (laserPowerRing.has_value())
        {
            m_powerRingIn = GeoDoublearray
            {
                context,
                Doublearray(1, laserPowerRing.value(), rank),
                ResultType::AnalysisOK,
                Limit
            };
        }
    }

    void signal()
    {
        m_pipeInLaserPowerCenter.signal(m_powerCenterIn);
        m_pipeInWeldingDuration.signal(m_durationIn);
        if (m_pipeInLaserPowerRing.has_value())
        {
            m_pipeInLaserPowerRing.value().signal(m_powerRingIn);
        }
    }
};

void TestSpotWeldingResult::testCtor()
{
    SpotWeldingResult filter{};
    QCOMPARE(filter.name(), std::string{"SpotWeldingResult"});
    QVERIFY(filter.findPipe("SpotWeldingResultOutput") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
}

void TestSpotWeldingResult::testProceed_data()
{
    QTest::addColumn<bool>("hasPowerRing");
    QTest::addColumn<double>("inputPowerCenter");
    QTest::addColumn<double>("inputPowerRing");
    QTest::addColumn<double>("inputDuration");
    QTest::addColumn<int>("rankIn");
    QTest::addColumn<double>("expectedPowerCenter");
    QTest::addColumn<double>("expectedPowerRing");
    QTest::addColumn<double>("expectedDuration");
    QTest::addColumn<int>("expectedPowerCenterRank");
    QTest::addColumn<int>("expectedPowerRingRank");
    QTest::addColumn<int>("expectedDurationRank");
    QTest::addColumn<bool>("expectedValid");

    const auto rankMax = static_cast<int>(eRankMax);
    const auto rankMin = static_cast<int>(eRankMin);
    QTest::newRow("powerCenter") << false << 25. << 0. << 500. << rankMax << 25. << 0. << 500. << rankMax << 0 << rankMax << true;
    QTest::newRow("badRankPowerCenterNotValid") << false << 25. << 0. << 500. << rankMin << 25. << 0. << 500. << rankMin << 0 << rankMin << false;
    QTest::newRow("zeroPowerValid") << false << 0. << 0. << 500. << rankMax << 0. << 0. << 500. << rankMax  << 0 << rankMax << true;
    QTest::newRow("negativPowerNotValid") << false << -1. << -1. << 500. << rankMax << -1. << -1. << 500. << rankMin << 0 << rankMin << false;
    QTest::newRow("zeroDurationNotValid") << false << 0.5 << 0. << 0. << rankMax << 0.5 << 0. << 0. << rankMin << 0 << rankMin << false;
    QTest::newRow("negativPowerCenterNotValid") << false << -0.5 << 0. << 500. << rankMax << -0.5 << 0. << 500. << rankMin << 0 << rankMin << false;
    QTest::newRow("negativDurationNoValid")<< false << 0.5 << 0. << -1000. << rankMax << 0.5 << 0. << -1000. << rankMin << 0 << rankMin << false;
    QTest::newRow("PowerRing") << true << 1.5 << 2.4 << 500. << rankMax << 1.5 << 2.4 << 500. << rankMax << rankMax << rankMax << true;
    QTest::newRow("badRankPowerRingNotValid") << true << 1.5 << 2.4 << 500. << rankMin << 1.5 << 2.4 << 500. << rankMin << rankMin << rankMin << false;
    QTest::newRow("negativPowerRingNotValid") << true << 1.5 << -1. << 500. << rankMax << 1.5 << -1. << 500. << rankMin << rankMin << rankMin << false;
}

void TestSpotWeldingResult::testProceed()
{

    QFETCH(bool, hasPowerRing);
    QFETCH(double, inputPowerCenter);
    QFETCH(double, inputPowerRing);
    QFETCH(double, inputDuration);
    QFETCH(int, rankIn);
    QFETCH(double, expectedPowerCenter);
    QFETCH(double, expectedPowerRing);
    QFETCH(double, expectedDuration);
    QFETCH(int, expectedPowerCenterRank);
    QFETCH(int, expectedPowerRingRank);
    QFETCH(int, expectedDurationRank);
    QFETCH(bool, expectedValid);

    SpotWeldingResult filter{};
    DummyResultHandler dummyFilter{};
    DummyInput dummyInput{};
    if (hasPowerRing)
    {
        dummyInput.setPowerRing();
    }
    QVERIFY(dummyInput.connectToFilter(&filter));
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("SpotWeldingResultOutput"), 0));

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<ResultDoubleArray>*>(filter.findPipe("SpotWeldingResultOutput"));
    QVERIFY(outPipe);

    filter.setParameter();
    int imageNumber{0};

    if (hasPowerRing)
    {
        dummyInput.fillData(imageNumber, inputPowerCenter, inputDuration, inputPowerRing, rankIn);
    }
    else
    {
        dummyInput.fillData(imageNumber, inputPowerCenter, inputDuration, std::nullopt, rankIn);
    }

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);

    const auto result = outPipe->read(imageNumber);
    QCOMPARE(result.resultType(), ResultType::ScanmasterSpotWelding);
    QVERIFY(!result.isNio());
    QCOMPARE(result.value().size(), 3);
    QCOMPARE(result.value()[0], expectedDuration);
    QCOMPARE(result.value()[1], expectedPowerCenter);
    QCOMPARE(result.rank()[0], expectedDurationRank);
    QCOMPARE(result.rank()[1], expectedPowerCenterRank);
    QCOMPARE(result.isValid(), expectedValid);
    if (hasPowerRing)
    {
        QCOMPARE(result.value()[2], expectedPowerRing);
        QCOMPARE(result.rank()[2], expectedPowerRingRank);
    }
}
QTEST_GUILESS_MAIN(TestSpotWeldingResult)
#include "testSpotWeldingResult.moc"
