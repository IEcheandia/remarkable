
#include <QtTest/QtTest>

#include "../blobDetection.h"
#include <geo/blob.h>
#include <fliplib/NullSourceFilter.h>
#include <overlay/overlayCanvas.h>

#include <fliplib/BaseFilter.h>

Q_DECLARE_METATYPE(precitec::geo2d::Blob)
Q_DECLARE_METATYPE(precitec::interface::GeoDoublearray)
Q_DECLARE_METATYPE(precitec::interface::GeoBlobarray)

// If some printed output is required
#define  TEXT_OUT  1

class BlobDetectionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
};

void fillImage(precitec::image::BImage& image)
{
    const uint width = 9;
    const uint height = 9;
    image.resizeFill(precitec::geo2d::Size(width, height), 0);
    // blob 1
    image[1][3] = 255;
    image[2][3] = 255;
    image[2][4] = 255;
    image[3][4] = 255;

    // blob 2
    image[6][1] = 255;
    image[6][2] = 255;
    image[6][3] = 255;

    // blob 4
    image[5][6] = 255;
    image[6][6] = 255;

    // blob 5
    image[3][8] = 255;
}

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

    void proceedGroup ( const void * sender, fliplib::PipeGroupEventArgs & e ) override
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

void BlobDetectionTest::testCtor()
{
    precitec::filter::BlobDetection filter;

    QCOMPARE(filter.name(), std::string("BlobDetection"));
    QVERIFY(filter.findPipe("Blobs") != nullptr);
    QVERIFY(filter.findPipe("PosX") != nullptr);
    QVERIFY(filter.findPipe("PosY") != nullptr);
}


void BlobDetectionTest::testProceed_data()
{
    QTest::addColumn< std::vector<precitec::geo2d::Blob> > ("expectedBlobs");
    QTest::addColumn< std::vector<double> > ("expectedPosX");
    QTest::addColumn< std::vector<double> > ("expectedPosY");

    // used values of Blob are xmin, xmax, ymin, ymax and npix
    precitec::geo2d::Blob biggerBlob;
    biggerBlob.npix = 4;
    biggerBlob.startx = 4;
    biggerBlob.starty = 3;
    biggerBlob.sx = 14;
    biggerBlob.sy = 8;
    biggerBlob.xmin = 3;
    biggerBlob.xmax = 4;
    biggerBlob.ymin = 1;
    biggerBlob.ymax = 3;
    precitec::geo2d::Blob smallerBlob;
    smallerBlob.npix = 3;
    smallerBlob.startx = 1;
    smallerBlob.starty = 6;
    smallerBlob.sx = 6;
    smallerBlob.sy = 18;
    smallerBlob.xmin = 1;
    smallerBlob.xmax = 3;
    smallerBlob.ymin = 6;
    smallerBlob.ymax = 6;

    //                  blobs                                                    pos x                                      pos y
    QTest::newRow("2 erkannte Blobs, 2 zu kleine")
        << std::vector<precitec::geo2d::Blob>{ biggerBlob, smallerBlob } << std::vector<double>{ 3, 2 } << std::vector<double>{ 2, 6 };
}

void BlobDetectionTest::testProceed()
{
    precitec::filter::BlobDetection filter;

    std::unique_ptr<precitec::image::OverlayCanvas> canvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(canvas.get());

    // In-Pipes
    fliplib::NullSourceFilter sourceFilterImage;
    fliplib::SynchronePipe<precitec::interface::ImageFrame>  pipeImage{ &sourceFilterImage, "ImageFrame" };
    pipeImage.setTag("ImageFrame");
    QVERIFY(filter.connectPipe(&pipeImage, 0));

    // Create Out-Pipes and connect to PoreClassifierOutputTriple filter
    // --------------------------------------------------

    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(filter.findPipe("Blobs"), 0));
    QVERIFY(filterOutData.connectPipe(filter.findPipe("PosX"), 0));
    QVERIFY(filterOutData.connectPipe(filter.findPipe("PosY"), 0));

    filter.getParameters().update(std::string("nspotsmax"), fliplib::Parameter::TYPE_UInt32, 100);
    filter.getParameters().update(std::string("MinBlobSize"), fliplib::Parameter::TYPE_UInt32, 3);
    filter.setParameter();

    // dummy data
    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    precitec::image::BImage image;
    fillImage(image);
    precitec::interface::ImageFrame imageFrame;
    imageFrame.data().swap(image);
    imageFrame.context().swap(context);

    // parse test data
    //QFETCH(std::vector<double>, bounding_box_dx);

    // signal pipes
    QCOMPARE(filterOutData.isProceedCalled(), false);
    pipeImage.signal(imageFrame);

    //verify that the filter has run
    QVERIFY(filterOutData.isProceedCalled());

    // compare signaled data

    // check if PoreCount is correct
    auto outPipeBlobs = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoBlobarray>*>(filter.findPipe("Blobs"));
    QVERIFY(outPipeBlobs);
    auto outPipePosX = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PosX"));
    QVERIFY(outPipePosX);
    auto outPipePosY = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PosY"));
    QVERIFY(outPipePosY);

    QFETCH(std::vector<precitec::geo2d::Blob>, expectedBlobs);
    QFETCH(std::vector<double>, expectedPosX);
    QFETCH(std::vector<double>, expectedPosY);

    QCOMPARE(outPipeBlobs->read(context.imageNumber()).ref().size(), expectedBlobs.size());
    const std::vector<precitec::geo2d::Blob> blobs = outPipeBlobs->read(context.imageNumber()).ref().getData();
    for (size_t i= 0; i < outPipeBlobs->read(context.imageNumber()).ref().size() && i < expectedBlobs.size(); i++)
    {
        precitec::geo2d::Blob blob = blobs[i];
        precitec::geo2d::Blob expectedBlob = expectedBlobs[i];

        QCOMPARE(blob.npix, expectedBlob.npix);
        QCOMPARE(blob.startx, expectedBlob.startx);
        QCOMPARE(blob.starty, expectedBlob.starty);
        QCOMPARE(blob.sx, expectedBlob.sx);
        QCOMPARE(blob.sy, expectedBlob.sy);
        QCOMPARE(blob.xmax, expectedBlob.xmax);
        QCOMPARE(blob.xmin, expectedBlob.xmin);
        QCOMPARE(blob.ymax, expectedBlob.ymax);
        QCOMPARE(blob.ymin, expectedBlob.ymin);
    }

    // check if maxSize is correct
    outPipePosX = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PosX"));
    QVERIFY(outPipePosX);
    QCOMPARE(outPipePosX->read(context.imageNumber()).ref().size(), expectedPosX.size());
    for (size_t i = 0; i < outPipePosX->read(context.imageNumber()).ref().size() && i < expectedPosX.size(); i++)
    {
        QCOMPARE(outPipePosX->read(context.imageNumber()).ref().getData()[i], expectedPosX[i]);
    }

    outPipePosY = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PosY"));
    QVERIFY(outPipePosY);
    QCOMPARE(outPipePosY->read(context.imageNumber()).ref().size(), expectedPosY.size());
    for (size_t i = 0; i < outPipePosY->read(context.imageNumber()).ref().size() && i < expectedPosY.size(); i++)
    {
        QCOMPARE(outPipePosY->read(context.imageNumber()).ref().getData()[i], expectedPosY[i]);
    }
}

QTEST_GUILESS_MAIN(BlobDetectionTest)

#include "blobDetectionTest.moc"
