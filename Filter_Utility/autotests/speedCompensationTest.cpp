#include "../speedCompensation.h"
#include "geo/geo.h"
#include <fliplib/NullSourceFilter.h>
#include <QTest>
#include <filter/productData.h>
#include <filter/armStates.h>

using precitec::filter::SpeedCompensation;
using point_vector_t = std::vector<precitec::geo2d::DPoint>;
using double_vector_t = std::vector<double>;
Q_DECLARE_METATYPE(point_vector_t)
Q_DECLARE_METATYPE(double_vector_t)

using namespace precitec::interface;
using namespace precitec::geo2d;

struct TestPosition
{
    double pos;
    int rankPos;
};

class SpeedCompensationTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
};

struct DummyInput
{

    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray> pipeInContour{&sourceFilter, "ContourIn"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInX{&sourceFilter, "SpeedInXDirection"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInY{&sourceFilter, "SpeedInYDirection"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInPositionX{&sourceFilter, "AbsolutePositionX"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInPositionY{&sourceFilter, "AbsolutePositionY"};

    DummyInput()
    {
        pipeInContour.setTag("ContourIn");
        pipeInX.setTag("SpeedInXDirection");
        pipeInY.setTag("SpeedInYDirection");
        pipeInPositionX.setTag("AbsolutePositionX");
        pipeInPositionY.setTag("AbsolutePositionY");
    }
    GeoVecAnnotatedDPointarray m_geoContourIn;
    GeoDoublearray m_geoXIn;
    GeoDoublearray m_geoYIn;
    GeoDoublearray m_geoXPositionIn;
    GeoDoublearray m_geoYPositionIn;

    bool connectToFilter(fliplib::BaseFilter* pFilter)
    {
        //connect pipes
        const int group = 1;
        bool ok = pFilter->connectPipe(&(pipeInContour), group);
        ok &= pFilter->connectPipe(&(pipeInX), group);
        ok &= pFilter->connectPipe(&(pipeInY), group);
        ok &= pFilter->connectPipe(&(pipeInPositionX), group);
        ok &= pFilter->connectPipe(&(pipeInPositionY), group);
        return ok;
    }


    void fillDataAndSignal(int imageNumber, double speedX, double speedY, point_vector_t points, int laserVelocity, double x_mm, double y_mm)
    {
        ImageContext context;
        context.setImageNumber(imageNumber);
        context.m_ScannerInfo = ScannerContextInfo{true, x_mm, y_mm};

        precitec::geo2d::AnnotatedDPointarray pointArray;
        pointArray.getData() = points;
        pointArray.getRank() = std::vector<int> (points.size(), 255);
        pointArray.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity).emplace_back(static_cast<double>(laserVelocity));

        m_geoContourIn = GeoVecAnnotatedDPointarray
        {
            context,
            {pointArray},
            ResultType::AnalysisOK,
            Limit
        };
        m_geoXIn = GeoDoublearray
        {
            context,
            Doublearray(1, speedX, 255),
            ResultType::AnalysisOK,
            Limit
        };
        m_geoYIn = GeoDoublearray
        {
            context,
            Doublearray(1, speedY, 255),
            ResultType::AnalysisOK,
            Limit
        };
        m_geoXPositionIn = GeoDoublearray
        {
            context,
            Doublearray(1,(x_mm * 1000),255),
            ResultType::AnalysisOK,
            Limit
        };
        m_geoYPositionIn = GeoDoublearray
        {
            context,
            Doublearray(1,(y_mm * 1000),255),
            ResultType::AnalysisOK,
            Limit
        };
    }

    void signal()
    {
        pipeInX.signal(m_geoXIn);
        pipeInY.signal(m_geoYIn);
        pipeInPositionX.signal(m_geoXPositionIn);
        pipeInPositionY.signal(m_geoYPositionIn);
        pipeInContour.signal(m_geoContourIn);
    }
};


class DummyOutFilter: public fliplib::BaseFilter
{
public:

    DummyOutFilter(): fliplib::BaseFilter("dummy") {}
    void proceed(const void *sender, fliplib::PipeEventArgs &e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    void proceedGroup (const void * sender, fliplib::PipeGroupEventArgs& e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    int getFilterType() const override
    {
        return BaseFilterInterface::SINK;
    }

    bool isProceedCalled() const
    {
        return m_proceedCalled;
    }

    void resetProceedCalled()
    {
        m_proceedCalled = false;
    }

private:
    bool m_proceedCalled = false;
};



void SpeedCompensationTest::testCtor()
{
    SpeedCompensation filter;
    QCOMPARE(filter.name(), std::string("SpeedCompensation"));
    QVERIFY(filter.findPipe("ContourOut") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
}

void SpeedCompensationTest::testProceed_data()
{
    QTest::addColumn<point_vector_t>("inputPoints");
    QTest::addColumn<double>("inputLaserVelocity");
    QTest::addColumn<double>("scannerPositionX_mm");
    QTest::addColumn<double>("scannerPositionY_mm");
    QTest::addColumn<double>("speedX");
    QTest::addColumn<double>("speedY");
    QTest::addColumn<int>("rateOfFeed");
    QTest::addColumn<int>("targetVelocity");
    QTest::addColumn<point_vector_t>("expectedPoints");
    QTest::addColumn<double_vector_t>("expectedLaserVelocity");
    QTest::addColumn<int>("expectedResultSize");

    double scannerPositionX_mm = 1.3;
    double scannerPositionY_mm = 1.9;

     QTest::addRow("verticalLineNegativSpeedX") << point_vector_t{{0., 0.}, {0., 10.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << -60. << 0. << 50 << 0 << point_vector_t{{0., 0.}, {12, 10.}} << double_vector_t{78.1025, 0} << 1;
     QTest::addRow("verticalLinePositivSpeedX") << point_vector_t{{0., 0.}, {0., 10.}} <<  -1. << scannerPositionX_mm << scannerPositionY_mm << 60. << 0. << 50 << 0 << point_vector_t{{0., 0.}, {-12, 10.}} << double_vector_t{78.1025, 0} << 1;
    QTest::addRow("horizontalLineNegativSpeedY") << point_vector_t{{0., 0.}, {10., 0.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << 0. << -60. << 50 << 0 << point_vector_t{{0., 0.}, {10, 12.}} << double_vector_t{78.1025, 0} << 1;
    QTest::addRow("horizontalLinePositivSpeedY") << point_vector_t{{0., 0.}, {10., 0.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << 0. << 60. << 50 << 0 << point_vector_t{{0., 0.}, {10, -12.}} << double_vector_t{78.1025, 0} << 1;
    QTest::addRow("diagonalPostivSpeedXY") << point_vector_t{{0., 0.}, {10., 10.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << 60. << 60. << 50 << 0 << point_vector_t{{0., 0.}, {-6.9706, -6.9706}} << double_vector_t{34.8528, 0} << 1;
    QTest::addRow("diagonal0XY") << point_vector_t{{0., 0.}, {10., 10.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << 0. << 0. << 50 << 0 << point_vector_t{{0., 0.}, {10, 10}} << double_vector_t{50, 0} << 1;
    QTest::addRow("0LaserVelocity") << point_vector_t{{0., 0.}, {10., 0.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << 0. << 60. << 0 << 0 << point_vector_t{{0., 0.}, {10, 10.}} << double_vector_t{78.1025, 0} << 0;
    QTest::addRow("SpeedYEqualLaserVelocity") << point_vector_t{{0., 0.}, {10., 0.}} << 50. << scannerPositionX_mm << scannerPositionY_mm << 0. << 50. << 0 << 50 << point_vector_t{{0., 0.}, {10, 10.}} << double_vector_t{70.7107, 0} << 0;
    QTest::addRow("SpeedYEqualLaserVelocity") << point_vector_t{{0., 0.}, {10., 0.}} << 50. << scannerPositionX_mm << scannerPositionY_mm << 0. << -50. << 0 << 50 << point_vector_t{{0., 0.}, {10, 10.}} << double_vector_t{70.7107, 0} << 0;
    QTest::addRow("horizontalLineTargetVelocity") << point_vector_t{{0., 0.}, {10., 0.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << 0. << -50. << 60 << 20 << point_vector_t{{0., 0.}, {10, 25.}} << double_vector_t{53.8516, 0} << 1;
    QTest::addRow("horizontalLineTargetVelocity") << point_vector_t{{0., 0.}, {10., 0.}} << -1. << scannerPositionX_mm << scannerPositionY_mm << 0. << 50. << 60 << 20 << point_vector_t{{0., 0.}, {10, -25.}} << double_vector_t{53.8516, 0} << 1;
    QTest::addRow("laserVelocityFromContourElse0") << point_vector_t{{0., 0.}, {0., 10.}} << 10. << scannerPositionX_mm << scannerPositionY_mm << 0. << 0. << 0 << 0 << point_vector_t{{0., 0.}, {0., 10.}} << double_vector_t{10., 0} << 1;
    QTest::addRow("laserVelocityFromContour") << point_vector_t{{0., 0.}, {10., 0.}} << 20. << scannerPositionX_mm << scannerPositionY_mm << 0. << -50. << 100 << 120 << point_vector_t{{0., 0.}, {10, 25.}} << double_vector_t{53.8516, 0} << 1;
    QTest::addRow("diagonal00_+10+10") << point_vector_t{{0., 0.}, {1.11111, 1.11111}, {2.22222, 2.22222}, {3.33333, 3.33333}, {4.44444, 4.44444}, {5.55555, 5.55555}, {6.66666, 6.66666}, {7.77777, 7.77777}, {8.88888, 8.88888}, {10, 10}} << 60. << scannerPositionX_mm << scannerPositionY_mm << 0. << 50. << 0 << 60 << point_vector_t{{0., 0.}, {1.11111, -0.198346}, {2.22222, -0.396692}, {3.33333, -0.595038}, {4.44444, -0.793384}, {5.55555, -0.99173}, {6.66666, -1.19008}, {7.77777, -1.38842}, {8.88888, -1.58677}, {10, -1.78511}} << double_vector_t{43.0971, 43.0971, 43.0971, 43.0971, 43.0971, 43.0971, 43.0971, 43.0971, 43.0971, 0} << 1;
    QTest::addRow("diagonal00_+10-10") << point_vector_t{{0., 0.}, {1.11111, -1.11111}, {2.22222, -2.22222}, {3.33333, -3.33333}, {4.44444, -4.44444}, {5.55555, -5.55555}, {6.66666, -6.66666}, {7.77777, -7.77777}, {8.88888, -8.88888}, {10, -10}} << 60. << scannerPositionX_mm << scannerPositionY_mm << 0. << 50. << 0 << 60 << point_vector_t{{0., 0.}, {1.11111, -2.42057}, {2.22222, -4.84113}, {3.33333, -7.2617}, {4.44444, -9.68226}, {5.55555, -12.1028}, {6.66666, -14.5234}, {7.77777, -16.944}, {8.88888, -19.3645}, {10, -21.7851}} << double_vector_t{101.699, 101.699, 101.699, 101.699, 101.699, 101.699, 101.699, 101.699, 101.699, 0} << 1;
    QTest::addRow("diagonal00_+3+3NegativSpeedY") << point_vector_t{{0., 0.}, {1.11111, -1.11111}, {2.22222, -2.22222}} << 60. << scannerPositionX_mm << scannerPositionY_mm << 0. << -50. << 0 << 60 << point_vector_t{{0., 0.}, {1.11111, 0.198346}, {2.22222, 0.396692}} << double_vector_t{43.0971, 43.0971, 0} << 1;
    QTest::addRow("arc+SpeedX-SpeedY") << point_vector_t{{10., 10.}, {12.7052, 6.21118}, {14.0335, 1.74928}, {13.8411, -2.90218}, {12.1489, -7.23915}, {9.14005, -10.7916}, {5.14078, -13.1747}} << 100. << scannerPositionX_mm << scannerPositionY_mm << 70. << -50. << 0 << 100 << point_vector_t{{10., 10.}, {9.44638, 8.53891}, {7.51592, 6.40472}, {4.06468, 4.08098}, {-0.886306, 2.07171}, {-7.15396, 0.846979}, {-14.4121, 0.79161}} << double_vector_t{33.5619, 61.8155, 89.3708, 114.773, 137.177, 155.909, 0} << 1;
}

void SpeedCompensationTest::testProceed()
{
    using namespace precitec::interface;
    SpeedCompensation filter;
    filter.setParameter();
    const auto seamSeries = 0;
    const auto seam = 0;
    const auto numTrigger = 1;
    const auto triggerDelta = 10000;

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());

    // create and connect pipes
    DummyInput dummyInput;
    DummyOutFilter dummyOutFilter;

    QVERIFY(dummyInput.connectToFilter(&filter));
    QVERIFY(dummyOutFilter.connectPipe(filter.findPipe("ContourOut"), 0));


    QFETCH(point_vector_t, inputPoints);
    QFETCH(double, inputLaserVelocity);
    QFETCH(double, scannerPositionX_mm);
    QFETCH(double, scannerPositionY_mm);
    QFETCH(double, speedX);
    QFETCH(double, speedY);
    QFETCH(int, rateOfFeed);
    QFETCH(int, targetVelocity);
    QFETCH(point_vector_t, expectedPoints);
    QFETCH(double_vector_t, expectedLaserVelocity);
    QFETCH(int, expectedResultSize);

    const int imageNumber = 0;
    dummyInput.fillDataAndSignal(imageNumber, speedX, speedY, inputPoints, inputLaserVelocity, scannerPositionX_mm, scannerPositionY_mm);
    rateOfFeed *= 1000;
    auto externalProductData = precitec::analyzer::ProductData {
    seamSeries, seam, static_cast<int>(rateOfFeed), triggerDelta, numTrigger};

    QVERIFY(filter.getParameters().exists(std::string("TargetVelocity")));
    QCOMPARE(filter.getParameters().findParameter(std::string("TargetVelocity")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("TargetVelocity")).getValue().convert<int>(), 0);

    filter.setExternalData(&externalProductData);
    filter.arm(precitec::filter::ArmState::eSeamStart);
    filter.getParameters().update(std::string("TargetVelocity"), fliplib::Parameter::TYPE_int, targetVelocity);
    filter.setParameter();

    QCOMPARE(dummyOutFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyOutFilter.isProceedCalled(), true);
    dummyOutFilter.resetProceedCalled();
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<GeoVecAnnotatedDPointarray>*>(filter.findPipe("ContourOut"));
    QVERIFY(outPipe);

    auto result = outPipe->read(0);
    QCOMPARE(result.ref().size(), expectedResultSize);
    if(static_cast<int>(result.ref().size()) != 0)
    {
        auto& firstContour = result.ref()[0];
        QCOMPARE(firstContour.getData().size(), expectedPoints.size());
        auto laserVelocity = firstContour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity);
        QCOMPARE(laserVelocity.size(), expectedLaserVelocity.size());
        for(std::size_t i = 0; i < expectedPoints.size(); ++i)
        {
            QVERIFY(qFuzzyCompare(float(firstContour.getData().at(i).x), float(expectedPoints.at(i).x)));
            QVERIFY(qFuzzyCompare(float(firstContour.getData().at(i).y), float(expectedPoints.at(i).y)));
            QVERIFY(qFuzzyCompare(float(laserVelocity.at(i)), float(expectedLaserVelocity.at(i))));
        }
    }
}

QTEST_GUILESS_MAIN(SpeedCompensationTest)
#include "speedCompensationTest.moc"
