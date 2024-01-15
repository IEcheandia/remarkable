#include <QTest>
#include "../intersect2Lines.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>
#include <util/calibDataSingleton.h>
#include <Calibration3DCoordsLoader.h>
#include <overlay/overlayCanvas.h>
#include <geo/lineModel.h>

using namespace precitec::interface;
using precitec::filter::Intersect2Lines;
using precitec::math::Calibration3DCoords;
using precitec::math::CalibrationParamMap;
using precitec::system::CalibDataSingleton;
using precitec::geo2d::DPoint;
using precitec::geo2d::LineModel;
using precitec::math::LineEquation;

Q_DECLARE_METATYPE(Calibration3DCoords);
Q_DECLARE_METATYPE(CalibrationParamMap);
Q_DECLARE_METATYPE(LineModel);

LineModel getLineModel (const precitec::math::LineEquation & rLine, double x, double y)
{
    double a, b, c;
    rLine.getCoefficients(a, b, c);
    return LineModel{x, y, a, b, c};
}


class Intersect2LinesTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
private:
    QTemporaryDir m_dir;
};


struct DummyInput
{
    DummyInput()
    :m_sourceFilter{},
    m_pipeInImage {&m_sourceFilter, "image"},
    m_pipeInFirstLine {&m_sourceFilter, "line1"},
    m_pipeInSecondLine {&m_sourceFilter, "line2"}
    {
        m_pipeInImage.setTag("ImageFrame");
        m_pipeInFirstLine.setTag("FirstLine");
        m_pipeInSecondLine.setTag("SecondLine");
    }

    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group =1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&(m_pipeInImage), group);
        ok &= pFilter->connectPipe(&(m_pipeInFirstLine), group);
        ok &= pFilter->connectPipe(&(m_pipeInSecondLine), group);
        return ok;
    }

    GeoLineModelarray createLineModelarray(ImageContext context, const std::vector<LineModel> & inputData)
    {
        precitec::geo2d::LineModelarray output(inputData.size(), {}, precitec::filter::eRankMax);
        output.getData() = inputData;

        return GeoLineModelarray( context, output, ResultType::AnalysisOK, Limit);
    }

    void fillData( int imageNumber, const std::vector<LineModel> & input1, const std::vector<LineModel> & input2)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        ImageContext context;
        context.setImageNumber(imageNumber);

        m_imageFrame = ImageFrame{context, precitec::image::BImage{precitec::geo2d::Size{500,500}} };
        m_geoLines1 = createLineModelarray(context, input1);
        m_geoLines2 = createLineModelarray(context, input2);

    }
    void signal()
    {
        m_pipeInImage.signal(m_imageFrame);
        m_pipeInFirstLine.signal(m_geoLines1);
        m_pipeInSecondLine.signal(m_geoLines2);
    }

    ImageFrame m_imageFrame;
    GeoLineModelarray m_geoLines1;
    GeoLineModelarray m_geoLines2;

    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe< ImageFrame> m_pipeInImage;
    fliplib::SynchronePipe< GeoLineModelarray> m_pipeInFirstLine;
    fliplib::SynchronePipe< GeoLineModelarray> m_pipeInSecondLine;


};


class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}
    void proceed(const void *sender, fliplib::PipeEventArgs &e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }
    void proceedGroup(const void * sender, fliplib::PipeGroupEventArgs & e) override
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
void Intersect2LinesTest::testCtor()
{
    Intersect2Lines filter;
    QCOMPARE(filter.name(), std::string("Intersect2Lines"));
    QVERIFY(filter.findPipe("IntersectionPresent") != nullptr);
    QVERIFY(filter.findPipe("IntersectionPositionX") != nullptr);
    QVERIFY(filter.findPipe("IntersectionPositionY") != nullptr);
    QVERIFY(filter.findPipe("AngleBetweenLines") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.getParameters().exists(std::string("ValidOnlyIfInRoi")));
    QVERIFY(filter.getParameters().exists(std::string("PlaneForAngleComputation")));

    QCOMPARE(filter.getParameters().findParameter(std::string("ValidOnlyIfInRoi")).getType(), fliplib::Parameter::TYPE_bool);
    QCOMPARE(filter.getParameters().findParameter(std::string("ValidOnlyIfInRoi")).getValue().convert<bool>(), false);
    QCOMPARE(filter.getParameters().findParameter(std::string("PlaneForAngleComputation")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("PlaneForAngleComputation")).getValue().convert<int>(), 3);
}


void Intersect2LinesTest::testProceed_data()
{
    QTest::addColumn<Calibration3DCoords>("calib3DCoords");
    QTest::addColumn<CalibrationParamMap>("calibParams");
    QTest::addColumn<bool>("paramValidOnlyInROI");
    QTest::addColumn<int>("paramPlaneForAngleComputation");
    QTest::addColumn<std::vector<LineModel>>("input1");
    QTest::addColumn<std::vector<LineModel>>("input2");
    QTest::addColumn<std::vector<double>>("expectedIntersection");
    QTest::addColumn<std::vector<double>>("expectedX");
    QTest::addColumn<std::vector<double>>("expectedY");
    QTest::addColumn<std::vector<double>>("expectedAngle");


    // coax calibration
    Calibration3DCoords coordsCoax;
    CalibrationParamMap paramsCoax;
    {
        std::string testdatafolder = QFINDTESTDATA("../../Analyzer_Interface/autotests/testdata/coax/").toStdString();
        std::string calibConfigFilename = testdatafolder + "/config/calibrationData0.xml";


        bool ok = paramsCoax.readParamMapFromFile(calibConfigFilename);
        QVERIFY(ok && paramsCoax.size() > 0);

        bool useOrientedLine = false;
        ok = precitec::math::loadCoaxModel(coordsCoax, {paramsCoax}, useOrientedLine);
        QVERIFY(ok);
        QVERIFY(!coordsCoax.isScheimpflugCase());
    }

    // scheimpflug calibration
    CalibrationParamMap paramsScheimpflug;
    Calibration3DCoords coordsScheimpflug;
    {
        std::string testdatafolder = QFINDTESTDATA("../../Analyzer_Interface/autotests/testdata/scheimpflug/").toStdString();
        precitec::system::CamGridData oCamGridData;
        std::string p_oCamGridImageFilename = testdatafolder + "/config/calibImgData0fallback.csv";
        std::string oMsgError = oCamGridData.loadFromCSV(p_oCamGridImageFilename);
        QVERIFY(oMsgError.empty());
        bool ok = loadCamGridData(coordsScheimpflug, oCamGridData);
        QVERIFY(ok);
    }



    DPoint intersection_pix {220.0, 230.0};
    DPoint center1 {300.0, 250.0};
    DPoint center2 {230.0, 220.0};
    double angle_pix = 59.0362434679265;
    double angle_line1_coax = 78.0756000292;
    double angle_line1_scheimpflug = 73.8348637856;


    LineEquation line1 {std::vector<double>{intersection_pix.x, center1.x}, std::vector<double>{intersection_pix.y, center1.y} };
    LineEquation line2 {std::vector<double>{intersection_pix.x, center2.x}, std::vector<double>{intersection_pix.y, center2.y} };
    auto lineModel1 = getLineModel(line1, center1.x, center1.y);
    auto lineModel2 = getLineModel(line2, center2.x, center2.y);


    QTest::addRow("SingleIntersection_screen_coax") << coordsCoax << paramsCoax
                                        << false << 3
                                        << std::vector<LineModel>{lineModel1} << std::vector<LineModel>{lineModel2}
                                        << std::vector<double>{1.0} << std::vector<double>{intersection_pix.x} << std::vector<double> {intersection_pix.y} << std::vector<double> {angle_pix};

    QTest::addRow("SingleIntersection_screen_scheimpflug") << coordsScheimpflug << paramsScheimpflug
                                        << false << 3
                                        << std::vector<LineModel>{lineModel1} << std::vector<LineModel>{lineModel2}
                                        << std::vector<double>{1.0} << std::vector<double>{intersection_pix.x} << std::vector<double> {intersection_pix.y} << std::vector<double> {angle_pix};
    QTest::addRow("SingleIntersection_line1_coax") << coordsCoax << paramsCoax
                                        << false << 0
                                        << std::vector<LineModel>{lineModel1} << std::vector<LineModel>{lineModel2}
                                        << std::vector<double>{1.0} << std::vector<double>{intersection_pix.x} << std::vector<double> {intersection_pix.y} << std::vector<double> {angle_line1_coax};

    QTest::addRow("SingleIntersection_line1_scheimpflug") << coordsScheimpflug << paramsScheimpflug
                                        << false << 0
                                        << std::vector<LineModel>{lineModel1} << std::vector<LineModel>{lineModel2}
                                        << std::vector<double>{1.0} << std::vector<double>{intersection_pix.x} << std::vector<double> {intersection_pix.y} << std::vector<double> {angle_line1_scheimpflug};

    QTest::addRow("SingleIntersection_line1Vertical_coax") << coordsCoax << paramsCoax
                                        << false << 0
                                        << std::vector<LineModel> {{630, 257.4423049219688, -6.8353901560624273, -1, 4563.7381032412973 }} << std::vector<LineModel>{{656, 159, 80.999999999999972, -1,-52976.999999999978}}
                                        << std::vector<double>{1.0} << std::vector<double>{655.097427142} << std::vector<double> {85.8915984937} << std::vector<double> {5.88388874801};


    QTest::addRow("SingleIntersection_line1Vertical_scheimpflug") << coordsScheimpflug << paramsScheimpflug
                                        << false << 0
                                        << std::vector<LineModel> {{630, 257.4423049219688, -6.8353901560624273, -1, 4563.7381032412973 }} << std::vector<LineModel>{{656, 159, 80.999999999999972, -1,-52976.999999999978}}
                                        << std::vector<double>{1.0} << std::vector<double>{655.097427142} << std::vector<double> {85.8915984937} << std::vector<double> {6.51837934798};


    QTest::addRow("SingleIntersection_screen_stack") << coordsCoax << paramsCoax
                                        << false << 3
                                        << std::vector<LineModel>{lineModel1, lineModel1, lineModel1} << std::vector<LineModel>{lineModel2, lineModel2, lineModel2}
                                        << std::vector<double>{1.0, 1.0, 1.0} << std::vector<double>{intersection_pix.x, intersection_pix.x, intersection_pix.x} << std::vector<double> {intersection_pix.y, intersection_pix.y, intersection_pix.y} << std::vector<double> {angle_pix, angle_pix, angle_pix};


    QTest::addRow("SingleIntersection_screen_constant1") << coordsCoax << paramsCoax
                                        << false << 3
                                        << std::vector<LineModel>{lineModel1} << std::vector<LineModel>{lineModel2, lineModel2, lineModel2}
                                        << std::vector<double>{1.0, 1.0, 1.0} << std::vector<double>{intersection_pix.x, intersection_pix.x, intersection_pix.x} << std::vector<double> {intersection_pix.y, intersection_pix.y, intersection_pix.y} << std::vector<double> {angle_pix, angle_pix, angle_pix};

    QTest::addRow("SingleIntersection_screen_constant2") << coordsCoax << paramsCoax
                                        << false << 3
                                        << std::vector<LineModel>{lineModel1, lineModel1, lineModel1} << std::vector<LineModel>{lineModel2}
                                        << std::vector<double>{1.0, 1.0, 1.0} << std::vector<double>{intersection_pix.x, intersection_pix.x, intersection_pix.x} << std::vector<double> {intersection_pix.y, intersection_pix.y, intersection_pix.y} << std::vector<double> {angle_pix, angle_pix, angle_pix};

}

void Intersect2LinesTest::testProceed()
{
    QFETCH(Calibration3DCoords, calib3DCoords);
    QFETCH(CalibrationParamMap, calibParams);
    auto &calibData = CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
    calibData.resetConfig();
    calibData.reload(calib3DCoords, calibParams);

    Intersect2Lines filter;
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());


    auto outPipePresent = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("IntersectionPresent"));
    auto outPipeX = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("IntersectionPositionX"));
    auto outPipeY = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("IntersectionPositionY"));
    auto outPipeAngle = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("AngleBetweenLines"));

    QVERIFY(outPipePresent);
    QVERIFY(outPipeX);
    QVERIFY(outPipeY);
    QVERIFY(outPipeAngle);

    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipePresent, 1));
    QVERIFY(dummyFilter.connectPipe(outPipeX, 1));
    QVERIFY(dummyFilter.connectPipe(outPipeY, 1));
    QVERIFY(dummyFilter.connectPipe(outPipeAngle, 1));


    QFETCH(bool, paramValidOnlyInROI);
    QFETCH(int, paramPlaneForAngleComputation);
    QFETCH(std::vector<LineModel>, input1);
    QFETCH(std::vector<LineModel>, input2);
    QFETCH(std::vector<double>, expectedIntersection);
    QFETCH(std::vector<double>, expectedX);
    QFETCH(std::vector<double>, expectedY);
    QFETCH(std::vector<double>, expectedAngle);


    filter.getParameters().update(std::string("ValidOnlyIfInRoi"), fliplib::Parameter::TYPE_bool, paramValidOnlyInROI);
    filter.getParameters().update(std::string("PlaneForAngleComputation"), fliplib::Parameter::TYPE_int, paramPlaneForAngleComputation);
    filter.setParameter();

    int imageNumber = 0;

    dummyInput.fillData(0, input1, input2);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);


    auto resultPresent = outPipePresent->read(imageNumber);
    auto resultX = outPipeX->read(imageNumber);
    auto resultY = outPipeY->read(imageNumber);
    auto resultAngle = outPipeAngle->read(imageNumber);

    auto expectedResultSize = expectedIntersection.size();
    QVERIFY2(expectedX.size() == expectedResultSize, "test data inconsistent");
    QVERIFY2(expectedY.size() == expectedResultSize, "test data inconsistent");
    QVERIFY2(expectedAngle.size() == expectedResultSize, "test data inconsistent");

    QCOMPARE(resultPresent.ref().size(), expectedResultSize);
    QCOMPARE(resultX.ref().size(), expectedResultSize);
    QCOMPARE(resultY.ref().size(), expectedResultSize);
    QCOMPARE(resultAngle.ref().size(), expectedResultSize);

    for (unsigned int i = 0; i < expectedResultSize; i++)
    {
        auto present = resultPresent.ref().getData()[i];
        auto x = resultX.ref().getData()[i];
        auto y = resultY.ref().getData()[i];
        auto angle = resultAngle.ref().getData()[i];

        QCOMPARE(present, expectedIntersection[i]);
        QCOMPARE(x, expectedX[i]);
        QCOMPARE(y, expectedY[i]);
        QCOMPARE(angle, expectedAngle[i]);
    }

}


QTEST_GUILESS_MAIN(Intersect2LinesTest)
#include "intersect2LinesTest.moc"
