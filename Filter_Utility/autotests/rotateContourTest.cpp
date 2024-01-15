#include <QTest>

#include "../rotateContour.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>

using precitec::filter::RotateContour;
using precitec::geo2d::Doublearray;

Q_DECLARE_METATYPE(precitec::geo2d::DPoint);

class RotateContourTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
};

// Dummy-Filter for the output
class DummyOutFilter : public fliplib::BaseFilter
{
public:
    DummyOutFilter() : fliplib::BaseFilter("dummy") {}
    void proceed(const void *sender, fliplib::PipeEventArgs &e) override
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

private:
    bool m_proceedCalled = false;
};


void RotateContourTest::testCtor()
{
    RotateContour testFilter;

    QCOMPARE(testFilter.name(), std::string("RotateContour"));

    // Outputs of the filter
    QVERIFY(testFilter.findPipe("ContourOut") != nullptr);
    QVERIFY(testFilter.findPipe("NotAValidPipe") == nullptr);

}


void RotateContourTest::testProceed_data()
{

    QTest::addColumn< std::vector< std::vector<precitec::geo2d::DPoint >>  > ("contour_in");
    QTest::addColumn< std::vector< double >> ("rotation_center_x");
    QTest::addColumn< std::vector< double >> ("rotation_center_y");
    QTest::addColumn< std::vector< double >> ("rotation_angle");
    QTest::addColumn< std::vector< std::vector< double >>  > ("laser_power");
    QTest::addColumn< std::vector< std::vector<precitec::geo2d::DPoint >> > ("contour_out_rotated");
    QTest::addColumn< std::vector< std::vector< double >>  > ("laser_power_expected");

    QTest::newRow("0 degree rotation")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(5.2, 5.2), precitec::geo2d::DPoint(7.2, 5.2),  precitec::geo2d::DPoint(15.2, 5.2),
                                                               precitec::geo2d::DPoint(12.2, 21.2), precitec::geo2d::DPoint(54.2,985.2),
                                                               precitec::geo2d::DPoint(4315.2,42334235.2)}}
    << std::vector< double >  { 0 }
    << std::vector< double >  { 0 }
    << std::vector< double >  { 0 }
    << std::vector< std::vector< double >>  { {1,2,3,4,5,6} }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(5.2, 5.2), precitec::geo2d::DPoint(7.2, 5.2),  precitec::geo2d::DPoint(15.2, 5.2),
                                                               precitec::geo2d::DPoint(12.2, 21.2), precitec::geo2d::DPoint(54.2,985.2),
                                                               precitec::geo2d::DPoint(4315.2,42334235.2)}}
    << std::vector< std::vector< double >>  { {1,2,3,4,5,6} };

    QTest::newRow("Empty contour")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  {  }
    << std::vector< double >  { 0 }
    << std::vector< double >  { 0 }
    << std::vector< double >  { 0 }
    << std::vector< std::vector< double >>  {  }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  {  }
    << std::vector< std::vector< double >>  {  };

    QTest::newRow("Empty rotation center")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(121, 121)} }
    << std::vector< double >  {  }
    << std::vector< double >  {  }
    << std::vector< double >  { 180 }
    << std::vector< std::vector< double >>  { 13 }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(121, 121) }}
    << std::vector< std::vector< double >>  { 13 };

    QTest::newRow("Missing rotation angle")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(0.4323423, -4230.534345)} }
    << std::vector< double >  { 0 }
    << std::vector< double >  { 0 }
    << std::vector< double >  {   }
    << std::vector< std::vector< double >> {  }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(0.4323423, -4230.534345) }}
    << std::vector< std::vector< double >> {  };

    QTest::newRow("180 degree rotation")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(1, 1)} }
    << std::vector< double >  { 0 }
    << std::vector< double >  { 0 }
    << std::vector< double >  { 180 }
    << std::vector< std::vector< double >> {  }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(-1, -1) }}
    << std::vector< std::vector< double >> {  };

    QTest::newRow("center 5/1, 150 degree rotation")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(1.4, 3.14), precitec::geo2d::DPoint(2.4, 4.14), precitec::geo2d::DPoint(3.58, 1.48),
                                                                 precitec::geo2d::DPoint(-132.58, -119.38), precitec::geo2d::DPoint(2540.39, 2789.84)} }
    << std::vector< double >  { 5 }
    << std::vector< double >  { 1 }
    << std::vector< double >  { 150 }
    << std::vector< std::vector< double >>  { {-1,-2,-3,-4,-999} }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(7.0476914536239796, -2.6532943640986986), precitec::geo2d::DPoint(5.6816660498395, -3.019319767883),
                                                               precitec::geo2d::DPoint(5.9897560733739, -0.125692193816530), precitec::geo2d::DPoint(184.337775052663, 36.4621381075707376),
                                                               precitec::geo2d::DPoint(-3585.1321485, -1146.51128709)} }
    << std::vector< std::vector< double >>  { {-1,-2,-3,-4,-999} };

    QTest::newRow("center -301.09/-368.29, 1366.6 degree rotation, oversampling (not supported)")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(2290.55, 1551), precitec::geo2d::DPoint( -6614.88, -4504.07),
                                                               precitec::geo2d::DPoint( 15789.53, 9332.52), precitec::geo2d::DPoint(-12872.19, 7957.6)} }
    << std::vector< double >  { -301.09, 321, 55,0.32 }
    << std::vector< double >  { -368.29, 99, 43242 }
    << std::vector< double >  { 1366.6, 123 }
    << std::vector< std::vector< double >>  { {1,2,3,4} }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(2278.6103344466, -2303.59829021013), precitec::geo2d::DPoint( -6068.27769425924, 4500.813251286),
                                                               precitec::geo2d::DPoint(13592.31817193089, -13016.88581173436), precitec::geo2d::DPoint(4086.38130402888965, 14057.48883715201)} }
    << std::vector< std::vector< double >>  { {1,2,3,4} };

    QTest::newRow("Very Big Point")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(-140482.34, -53111.89) }}
    << std::vector< double >  { 29846,31 }
    << std::vector< double >  { -108844.43 }
    << std::vector< double >  { 666.66 }
    << std::vector< std::vector< double >>  { {-123.7486} }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(-27142.8989305238, 61067.679105542396) } }
    << std::vector< std::vector< double >>  { {-123.7486} };

    QTest::newRow("Several Contours")
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(-111.97, 147.65), precitec::geo2d::DPoint(54.66, 40.56)},
                                                              {precitec::geo2d::DPoint(85.33, -141.08), precitec::geo2d::DPoint(-200, 0)},
                                                              {precitec::geo2d::DPoint(-200, 0), precitec::geo2d::DPoint(400, 200)},
                                                              {precitec::geo2d::DPoint(1000, -600), precitec::geo2d::DPoint(-4000, 2000)}}
    << std::vector< double >  { 68.42 }
    << std::vector< double >  { -54.52 }
    << std::vector< double >  { -140 }
    << std::vector< std::vector< double >>  { {1,2}, {}, {4,5}, {7,666} }
    << std::vector< std::vector<precitec::geo2d::DPoint >>  { {precitec::geo2d::DPoint(336.5591281446, -93.438748154), precitec::geo2d::DPoint(140.0770174663133, -118.5107481424656)},
                                                              {precitec::geo2d::DPoint(-0.17350702760876, 0.919268516579), precitec::geo2d::DPoint(309.086429902106, 76.252307153214)},
                                                              {precitec::geo2d::DPoint(309.086429902106, 76.252307153214), precitec::geo2d::DPoint(-21.98271403197266, -462.6291472825)},
                                                              {precitec::geo2d::DPoint(-995.83946765259, -235.466158599246), precitec::geo2d::DPoint(4505.6305331273015, 986.7563377241088)}}
    << std::vector< std::vector< double >>  { {1,2}, {}, {4,5}, {7,666} };

}

void RotateContourTest::testProceed()
{
    // Create RotateContour filter
    RotateContour testFilter;

    // Create In-Pipes and connect to Conditional filter
    // -------------------------------------------------

    // In-Pipe "rotation_center_x"
    fliplib::NullSourceFilter sourceFilterRotateCenterX;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeRCX{ &sourceFilterRotateCenterX, "rotation_center_x"};
    pipeRCX.setTag("rotation_center_x");
    QVERIFY(testFilter.connectPipe(&pipeRCX, 1));

    // In-Pipe "rotation_center_y"
    fliplib::NullSourceFilter sourceFilterRotateCenterY;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeRCY{ &sourceFilterRotateCenterY, "rotation_center_y"};
    pipeRCY.setTag("rotation_center_y");
    QVERIFY(testFilter.connectPipe(&pipeRCY, 1));

    // In-Pipe "rotation_angle"
    fliplib::NullSourceFilter sourceFilterRotationCenter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeRA{ &sourceFilterRotationCenter, "rotation_angle"};
    pipeRA.setTag("rotation_angle");
    QVERIFY(testFilter.connectPipe(&pipeRA, 1));

    // In-Pipe "contour"
    fliplib::NullSourceFilter sourceFilterContour;
    fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray> pipeContour{ &sourceFilterContour, "contour"};
    pipeContour.setTag("contour");
    QVERIFY(testFilter.connectPipe(&pipeContour, 1));

    // Create Out-Pipes and connect to Conditional filter
    // --------------------------------------------------

    DummyOutFilter filterContourOut;
    QVERIFY(filterContourOut.connectPipe(testFilter.findPipe("ContourOut"), 0));

    // Set parameters for the In-Pipes and the filter
    // ----------------------------------------------

    QFETCH(std::vector< std::vector<precitec::geo2d::DPoint>>, contour_in);
    QFETCH(std::vector< double >, rotation_center_x);
    QFETCH(std::vector< double >, rotation_center_y);
    QFETCH(std::vector< double >, rotation_angle);
    QFETCH(std::vector< std::vector< double >>, laser_power);
    QFETCH(std::vector< std::vector<precitec::geo2d::DPoint>>, contour_out_rotated);
    QFETCH(std::vector< std::vector< double >>, laser_power_expected);

    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    testFilter.setParameter();

    int    iDummyRank = 255;   // int 0..255

    // Filter not yet triggered!
    QCOMPARE(filterContourOut.isProceedCalled(),  false);

    Doublearray rotCenter_X_Arr(rotation_center_x.size(), 0, iDummyRank);
    rotCenter_X_Arr.getData() = rotation_center_x;

    // Set new values for the pipes
    auto inPipe = precitec::interface::GeoDoublearray {
        context,
        rotCenter_X_Arr,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeRCX.signal(inPipe);

    Doublearray rotCenter_Y_Arr(rotation_center_y.size(), 0, iDummyRank);
    rotCenter_Y_Arr.getData() = rotation_center_y;

    inPipe = precitec::interface::GeoDoublearray {
        context,
        rotCenter_Y_Arr,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeRCY.signal(inPipe);

    Doublearray rotAngle_Arr(rotation_angle.size(), 0, iDummyRank);
    rotAngle_Arr.getData() = rotation_angle;

    inPipe = precitec::interface::GeoDoublearray {
        context,
        rotAngle_Arr,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeRA.signal(inPipe);

    std::vector<precitec::geo2d::TAnnotatedArray<precitec::geo2d::DPoint>> contourOut;
    auto laserPowerIter = laser_power.begin();

    auto add = [&](const std::vector<precitec::geo2d::DPoint>& a)
    {
        precitec::geo2d::TAnnotatedArray<precitec::geo2d::DPoint> contourTAA;
        contourTAA.getData().insert( contourTAA.getData().begin(), a.begin(), a.end());
        contourTAA.getRank().assign(contourTAA.getData().size(), 255.0);
        if(laserPowerIter != laser_power.end())
        {
            contourTAA.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower) = *laserPowerIter;
            laserPowerIter++;
        }
        contourOut.push_back(contourTAA);
    };

    std::for_each(contour_in.begin(), contour_in.end(), add);

    auto contour_GeoVecAnnotatedDPointarray = precitec::interface::GeoVecAnnotatedDPointarray {
            context,
            contourOut,
            precitec::interface::ResultType::AnalysisOK,
            precitec::interface::Limit
    };
    pipeContour.signal(contour_GeoVecAnnotatedDPointarray);

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray>*>(testFilter.findPipe("ContourOut"));
    QVERIFY(outPipe);

    precitec::interface::GeoDoublearray resDataOut;
    const auto resFilterDataOut = outPipe->read(resDataOut.context().imageNumber());
    laserPowerIter = laser_power_expected.begin();

    auto compareRoundedFloatingPoints = [&](const precitec::geo2d::DPoint &a, const precitec::geo2d::DPoint &b)
    {
       return  qFuzzyCompare(a.x, b.x) && qFuzzyCompare(a.y, b.y);
    };

    auto compareManyContourArrays = [&](const precitec::geo2d::TAnnotatedArray<precitec::geo2d::DPoint>& a, const std::vector<precitec::geo2d::DPoint>& b)
    {
        // check laserpower attribute
        if(laserPowerIter != laser_power_expected.end())
        {
            if(a.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower))
            {
                if(!std::equal( a.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower).begin(),
                                a.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower).end(), laserPowerIter->begin())  ||
                                laserPowerIter->size() != a.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower).size())
                {
                    laserPowerIter++;
                    return false;
                }
            }
            else
            {
                laserPowerIter++;
                return false;
            }
            laserPowerIter++;
        }
        else
        {
            if(a.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower))
            {
                return false;
            }
        }

        return std::equal( a.getData().begin(), a.getData().end(), b.begin(), compareRoundedFloatingPoints) &&
                           a.getData().size() == b.size();
    };

    QVERIFY(std::equal( resFilterDataOut.ref().begin(), resFilterDataOut.ref().end(), contour_out_rotated.begin(), compareManyContourArrays));

    QCOMPARE(resFilterDataOut.ref().size() , contour_out_rotated.size() );
    QVERIFY( filterContourOut.isProceedCalled() );
}

QTEST_GUILESS_MAIN(RotateContourTest)
#include "rotateContourTest.moc"
