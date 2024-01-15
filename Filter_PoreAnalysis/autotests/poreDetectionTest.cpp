#include <QTest>

#include "../poreDetection.h"
#include "image/image.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>
#include <Calibration3DCoordsLoader.h>

class TestPoreDetection : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
};

Q_DECLARE_METATYPE(precitec::math::Calibration3DCoords);

struct DummyInput
{
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_pipeInImage;

    precitec::interface::ImageFrame m_image;

    DummyInput()
    : m_pipeInImage{&m_sourceFilter, "ImageFrame"}
    {
    }

    void connectToFilter(fliplib::BaseFilter * pFilter)
    {
        const auto group = 0;
        //connect  pipes
        m_pipeInImage.setTag("ImageFrame");
        QVERIFY(pFilter->connectPipe(&m_pipeInImage, group));
    }

    void fillData(int imageNumber, precitec::geo2d::Point trafoOffset)
    {
        const auto height = 12u;
        const auto width = 12u;
        precitec::image::BImage image;
        image.resizeFill(precitec::geo2d::Size(width, height), 255);

        precitec::interface::SmpTrafo oTrafo {new precitec::interface::LinearTrafo(trafoOffset)};
        precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        // blob 1
        for (int i = 3; i < 7; i++)
        {
            image[1][i] = 3;
        }
        for (int i = 2; i < 7; i++)
        {
            image[2][i] = 3;
        }
        image[3][2] = 10;
        image[3][3] = 10;
        image[3][4] = 10;

        // blob 2
        image[6][1] = 100;
        image[6][2] = 155;
        image[6][3] = 55;

        // blob 4
        image[5][6] = 5;
        image[6][6] = 20;

        // blob 5
        image[3][8] = 25;

        if (imageNumber == 1)
        {
            // make blob 1 bigger, so that it is a pore
            image[3][5] = 4;
            image[3][6] = 4;
            for (int j = 4; j < 7; j++)
            {
                for (int i = 1; i < 9; i++)
                {
                    image[j][i] = i + j;
                }
            }
        }

        m_image.data().swap(image);
        m_image.context() = context;
    }

    void signal()
    {
        m_pipeInImage.signal(m_image);
    }
};

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}
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

void TestPoreDetection::testCtor()
{
    precitec::filter::PoreDetection filter;
    QCOMPARE(filter.name(), std::string("PoreDetection"));
    QVERIFY(filter.findPipe("poreCount") != nullptr);
    QVERIFY(filter.findPipe("poreCount1") != nullptr);
    QVERIFY(filter.findPipe("poreCount2") != nullptr);
    QVERIFY(filter.findPipe("sizeMax") != nullptr);

    std::vector<std::pair<std::string, int>> intParameters = {{"ParameterScaling", 0}, {"ParameterScaling1", 0}, {"ParameterScaling2", 0},
                                                              {"Visualize", 0}, {"Visualize1", 0}, {"Visualize2", 0},
                                                              {"BinarizeType", 0}, {"BinarizeType1", 0}, {"BinarizeType2", 0}
    };
    std::vector<std::pair<std::string, uint>> uintParameters = {{"NumberParameterSets", 1}, {"DistToMeanIntensity", 128}, {"DistToMeanIntensity1", 128}, {"DistToMeanIntensity2", 128},
                                                                {"NumberIterationsMorphology", 2}, {"NumberIterationsMorphology1", 2}, {"NumberIterationsMorphology2", 2},
                                                                {"MaxNumberBlobs", 100}, {"MaxNumberBlobs1", 100}, {"MaxNumberBlobs2", 100},
                                                                {"MinBlobSize", 20}, {"MinBlobSize1", 20}, {"MinBlobSize2", 20},
                                                                {"NumberNeighbors", 1}, {"NumberNeighbors1", 1}, {"NumberNeighbors2", 1},
                                                                {"OuterNeighborDistance", 3}, {"OuterNeighborDistance1", 3}, {"OuterNeighborDistance2", 3},
                                                                {"MinimalContrast", 50}, {"MinimalContrast1", 50}, {"MinimalContrast2", 50},
                                                                {"MaximalContrast", 255}, {"MaximalContrast1", 255}, {"MaximalContrast2", 255},
                                                                {"MinimalSurface", 0}, {"MinimalSurface1", 0}, {"MinimalSurface2", 0},
                                                                {"MaximalSurface", 500}, {"MaximalSurface1", 500}, {"MaximalSurface2", 500}
    };
    std::vector<std::pair<std::string, double>> doubleParameters = {{"BoundingBoxScale", 0.5}, {"BoundingBoxScale1", 0.5}, {"BoundingBoxScale2", 0.5},
                                                                    {"MinimalWidth", 0.1}, {"MinimalWidth1", 0.1}, {"MinimalWidth2", 0.1},
                                                                    {"MaximalWidth", 3}, {"MaximalWidth1", 3}, {"MaximalWidth2", 3},
                                                                    {"MinimalHeight", 0.1}, {"MinimalHeight1", 0.1}, {"MinimalHeight2", 0.1},
                                                                    {"MaximalHeight", 3}, {"MaximalHeight1", 3}, {"MaximalHeight2", 3},
                                                                    {"MinimalPcRatio", 0.6}, {"MinimalPcRatio1", 0.6}, {"MinimalPcRatio2", 0.6},
                                                                    {"MaximalPcRatio", 1.4}, {"MaximalPcRatio1", 1.4}, {"MaximalPcRatio2", 1.4}
    };

    for (const auto& entry : intParameters)
    {
        std::string parameter = entry.first;
        int value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), value);
    }
    for (const auto& entry : uintParameters)
    {
        std::string parameter = entry.first;
        uint value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_uint);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue(), value);
    }
    for (const auto& entry : doubleParameters)
    {
        std::string parameter = entry.first;
        double value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_double);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue(), value);
    }
}

void TestPoreDetection::testProceed_data()
{
    QTest::addColumn<uint>("imageNumber");
    QTest::addColumn<uint>("activeParameterSets");
    QTest::addColumn<uint>("distToMeanIntensity");
    QTest::addColumn<uint>("minBlobSize");
    QTest::addColumn<uint>("numberIterationsMorphology");
    QTest::addColumn<double>("minWidth1");
    QTest::addColumn<double>("expectedPoreCount");
    QTest::addColumn<double>("expectedPoreCount1");
    QTest::addColumn<double>("expectedPoreCount2");
    QTest::addColumn<double>("expectedMaxPoreSize");

//                                      image     set   distToMeanIntensity  minBlobSize   nbIterationsMorph   minWidth1   expectedCount       expectedMaxSize
     QTest::newRow("No Pores, 1 Set") << 0u     << 1u   << 5u               << 3u           << 0u            <<  0.1     << 0. << -1. << -1.     << 0.;
     QTest::newRow("1 Pore, 3 Sets")  << 1u     << 3u   << 5u               << 3u           << 0u            <<  0.5     << 1. <<  0. <<  1.     << 0.055;
}

void TestPoreDetection::testProceed()
{
    // initialize calibration (needed to compute width and height of bounding box)
    std::string testdatafolder = QFINDTESTDATA("../../Analyzer_Interface/autotests/testdata/coax/").toStdString();
    std::string calibConfigFilename = testdatafolder + "/config/calibrationData0.xml";

    precitec::math::Calibration3DCoords coords;
    precitec::math::CalibrationParamMap params;

    bool ok = params.readParamMapFromFile(calibConfigFilename);
    QVERIFY(ok && params.size() > 0);
    QVERIFY(precitec::math::loadCoaxModel(coords, {params}, false));

    auto &calibData = precitec::system::CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
    calibData.resetConfig();
    calibData.reload(coords, params);

    precitec::filter::PoreDetection filter;

    std::unique_ptr<precitec::image::OverlayCanvas> canvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(canvas.get());

    // In-Pipe
    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    //connect  pipes
    auto outPipePoreCount = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("poreCount"));
    QVERIFY(outPipePoreCount);
    auto outPipePoreCount1 = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("poreCount1"));
    QVERIFY(outPipePoreCount1);
    auto outPipePoreCount2 = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("poreCount2"));
    QVERIFY(outPipePoreCount2);
    auto outPipeSizeMax = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("sizeMax"));
    QVERIFY(outPipeSizeMax);

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipePoreCount, 1));
    QVERIFY(dummyFilter.connectPipe(outPipePoreCount1, 1));
    QVERIFY(dummyFilter.connectPipe(outPipePoreCount2, 1));
    QVERIFY(dummyFilter.connectPipe(outPipeSizeMax, 1));

    QFETCH(uint, activeParameterSets);
    QFETCH(uint, distToMeanIntensity);
    QFETCH(uint, minBlobSize);
    QFETCH(uint, numberIterationsMorphology);
    QFETCH(double, minWidth1);
    filter.getParameters().update(std::string("NumberParameterSets"), fliplib::Parameter::TYPE_uint, activeParameterSets);
    filter.getParameters().update(std::string("DistToMeanIntensity"), fliplib::Parameter::TYPE_uint, distToMeanIntensity);
    filter.getParameters().update(std::string("DistToMeanIntensity1"), fliplib::Parameter::TYPE_uint, distToMeanIntensity);
    filter.getParameters().update(std::string("DistToMeanIntensity2"), fliplib::Parameter::TYPE_uint, distToMeanIntensity);
    filter.getParameters().update(std::string("MinBlobSize"), fliplib::Parameter::TYPE_uint, minBlobSize);
    filter.getParameters().update(std::string("MinBlobSize1"), fliplib::Parameter::TYPE_uint, minBlobSize);
    filter.getParameters().update(std::string("MinBlobSize2"), fliplib::Parameter::TYPE_uint, minBlobSize);
    filter.getParameters().update(std::string("NumberIterationsMorphology"), fliplib::Parameter::TYPE_uint, numberIterationsMorphology);
    filter.getParameters().update(std::string("NumberIterationsMorphology1"), fliplib::Parameter::TYPE_uint, numberIterationsMorphology);
    filter.getParameters().update(std::string("NumberIterationsMorphology2"), fliplib::Parameter::TYPE_uint, numberIterationsMorphology);
    filter.getParameters().update(std::string("MinimalWidth1"), fliplib::Parameter::TYPE_double, minWidth1);
    filter.setParameter();

    QFETCH(uint, imageNumber);
    precitec::geo2d::Point trafoOffset {510,100};
    precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
    precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
    context.setImageNumber(imageNumber);
    dummyInput.fillData(imageNumber, trafoOffset);

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    QFETCH(double, expectedPoreCount);
    QFETCH(double, expectedPoreCount1);
    QFETCH(double, expectedPoreCount2);
    QFETCH(double, expectedMaxPoreSize);

    outPipePoreCount = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("poreCount"));
    const auto resultPoreCount = outPipePoreCount->read(context.imageNumber()).ref().getData();
    QCOMPARE(resultPoreCount[0], expectedPoreCount);
    outPipePoreCount1 = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("poreCount1"));
    const auto resultPoreCount1 = outPipePoreCount1->read(context.imageNumber()).ref().getData();
    QCOMPARE(resultPoreCount1[0], expectedPoreCount1);
    outPipePoreCount2 = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("poreCount2"));
    const auto resultPoreCount2 = outPipePoreCount2->read(context.imageNumber()).ref().getData();
    QCOMPARE(resultPoreCount2[0], expectedPoreCount2);
    outPipeSizeMax = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("sizeMax"));
    QVERIFY(outPipeSizeMax);
    const auto resultSizeMax = outPipeSizeMax->read(context.imageNumber()).ref().getData();
    QCOMPARE(std::round(resultSizeMax[0] * 1000), expectedMaxPoreSize * 1000);

    if (resultPoreCount[0] > 0)
    {
        std::vector<precitec::geo2d::Doublearray> dx, dy, pcRatio, gradient, variance;
        filter.getFeatures(dx, dy, pcRatio, gradient, variance);
        for (uint i = 0; i < dx.size(); i++)
        {
            std::cout << "Parameter set " << i << std::endl;
            for (uint j = 0; j < dx[i].size(); j++)
            {
                std::cout << "dx: " << dx[i].getData()[j] << "\tdy: " << dy[i].getData()[j] << "\tpc ratio: " << pcRatio[i].getData()[j] << "\tgradient: " << gradient[i].getData()[j] << "\tvariance: "
                << variance[i].getData()[j] << std::endl;
            }
        }
    }
}

QTEST_GUILESS_MAIN(TestPoreDetection)
#include "poreDetectionTest.moc"
