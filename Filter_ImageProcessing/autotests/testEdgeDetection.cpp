#include <QTest>

#include "../edgeDetectionImpl.h"
#include "image/image.h"

namespace precitec
{
namespace filter
{

class TestEdgeDetection : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRobertsBorder();
    void testKirschBorder();
    void testSobelBorder();
};

void TestEdgeDetection::testRobertsBorder()
{
    const auto height = 1536u;
    const auto width = 2048u;

    image::BImage image;
    image.resizeFill(geo2d::Size(width, height), 255);

    image::BImage imageOut;
    imageOut.resizeFill(geo2d::Size(width, height), 255);

    roberts(image, imageOut, 0);

    //Assure the last row and column is 0
    //and not the initial 'garbage value' of '255'
    for (auto i = 0u; i < height; i++)
    {
        QVERIFY(imageOut[i][width - 1] == 0);
    }

    for (auto j = 0u; j < width; j++)
    {
        QVERIFY(imageOut[height - 1][j] == 0);
    }
}

void TestEdgeDetection::testKirschBorder()
{
    const auto height = 1536u;
    const auto width = 2048u;

    image::BImage image;
    image.resizeFill(geo2d::Size(width, height), 255);

    image::BImage imageOut;
    imageOut.resizeFill(geo2d::Size(width, height), 255);

    kirsch(image, imageOut, 0);

    //Assure the border is 0 and not the initial 'garbage value' of '255'
    for (auto i = 0u; i < height; i++)
    {
        QVERIFY(imageOut[i][0] == 0);
        QVERIFY(imageOut[i][width - 1] == 0);
    }

    for (auto j = 0u; j < width; j++)
    {
        QVERIFY(imageOut[0][j] == 0);
        QVERIFY(imageOut[height - 1][j] == 0);
    }
}

void TestEdgeDetection::testSobelBorder()
{
    const auto height = 1536u;
    const auto width = 2048u;

    image::BImage image;
    image.resizeFill(geo2d::Size(width, height), 255);

    image::BImage imageOut;
    imageOut.resizeFill(geo2d::Size(width, height), 255);

    sobel(image, imageOut, 0);

    //Assure the border is 0 and not the initial 'garbage value' of '255'
    for (auto i = 0u; i < height; i++)
    {
        QVERIFY(imageOut[i][0] == 0);
        QVERIFY(imageOut[i][width - 1] == 0);
    }

    for (auto j = 0u; j < width; j++)
    {
        QVERIFY(imageOut[0][j] == 0);
        QVERIFY(imageOut[height - 1][j] == 0);
    }
}

} //namespace filter
} //namespace precitec

QTEST_MAIN(precitec::filter::TestEdgeDetection)
#include "testEdgeDetection.moc"
