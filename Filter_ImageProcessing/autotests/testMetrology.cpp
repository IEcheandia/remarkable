#include <QTest>

#include "../metrology.h"

#include <string>

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(cv::Point2f)
Q_DECLARE_METATYPE(cv::Rect2f)

namespace precitec
{
namespace filter
{

class TestMetrology : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParseArray_data();
    void testParseArray();
    void testIntegrateContour_data();
    void testIntegrateContour();
    void testSampleContour_data();
    void testSampleContour();
    void testRotateContour_data();
    void testRotateContour();
    void testContourBoundingBox_data();
    void testContourBoundingBox();
};

void TestMetrology::testParseArray_data()
{
    QTest::addColumn<std::string>("contourString");
    QTest::addColumn<std::vector<cv::Point2f>>("expectedContour");

    QTest::newRow("valid contour of one point")
        << std::string("[1.123 -1.123]")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(1.123, -1.123),
            };

    QTest::newRow("valid contour of one point and semicolon at the end")
        << std::string("[1.123 -1.123;]")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(1.123, -1.123),
            };

    QTest::newRow("invalid contour with only one coordinate")
        << std::string("[1.123]")
        << std::vector<cv::Point2f>{};

    QTest::newRow("valid contour with 3 points")
        << std::string("[1.123 -1.123; 3 4; 5 6]")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(1.123, -1.123),
                cv::Point2f(3, 4),
                cv::Point2f(5, 6),
            };

    QTest::newRow("invalid contour that has a 3-D coordinate")
        << std::string("[1.123 -1.123; 3 4; 5 6 7]")
        << std::vector<cv::Point2f>{};

    QTest::newRow("invalid decimal separator")
        << std::string("[1,123 -1,123]")
        << std::vector<cv::Point2f>{};
}

void TestMetrology::testParseArray()
{
    QFETCH(std::string, contourString);
    QFETCH(std::vector<cv::Point2f>, expectedContour);

    QCOMPARE(parseArray(contourString), expectedContour);
}

void TestMetrology::testIntegrateContour_data()
{
    QTest::addColumn<std::vector<cv::Point2f>>("contour");
    QTest::addColumn<std::vector<float>>("expectedLengths");

    QTest::newRow("contour with zero point")
        << std::vector<cv::Point2f>
            {
            }
        << std::vector<float>
            {
            };

    QTest::newRow("contour with one point")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(-1.0, 0),
            }
        << std::vector<float>
            {
                0.0,
            };

    QTest::newRow("contour with two points")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(-1.0, 0),
                cv::Point2f(-1.0, 1.5),
            }
        << std::vector<float>
            {
                0.0,
                1.5,
            };

    QTest::newRow("contour with three points")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(-1.0, 0),
                cv::Point2f(-1.0, 1.5),
                cv::Point2f(1.0, 1.5),
            }
        << std::vector<float>
            {
                0.0,
                1.5,
                1.5 + 2,
            };
}

void TestMetrology::testIntegrateContour()
{
    QFETCH(std::vector<cv::Point2f>, contour);
    QFETCH(std::vector<float>, expectedLengths);

    QCOMPARE(integrateContour(contour), expectedLengths);
}

void TestMetrology::testSampleContour_data()
{
    QTest::addColumn<std::vector<cv::Point2f>>("contour");
    QTest::addColumn<unsigned int>("nSample");
    QTest::addColumn<std::vector<cv::Point2f>>("expectedContour");

    QTest::newRow("zero sample")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0),
                cv::Point2f(0, 1),
            }
        << (unsigned int) 0
        << std::vector<cv::Point2f>
            {
            };

    QTest::newRow("one sample")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0),
                cv::Point2f(0, 1),
            }
        << (unsigned int) 1
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0.5),
            };

    QTest::newRow("two samples")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0),
                cv::Point2f(0, 1),
            }
        << (unsigned int) 2
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0.25),
                cv::Point2f(0, 0.75),
            };

}

void TestMetrology::testSampleContour()
{
    QFETCH(std::vector<cv::Point2f>, contour);
    QFETCH(unsigned int, nSample);
    QFETCH(std::vector<cv::Point2f>, expectedContour);

    QCOMPARE(sampleContour(contour, nSample), expectedContour);
}

void TestMetrology::testRotateContour_data()
{
    QTest::addColumn<std::vector<cv::Point2f>>("contour");
    QTest::addColumn<float>("angle");
    QTest::addColumn<std::vector<cv::Point2f>>("expectedContour");

    QTest::newRow("rotate by 90 degrees")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0),
                cv::Point2f(0, 1),
            }
        << (float)M_PI / 2
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0),
                cv::Point2f(-1, 0),
            };
}

void TestMetrology::testRotateContour()
{
    QFETCH(std::vector<cv::Point2f>, contour);
    QFETCH(float, angle);
    QFETCH(std::vector<cv::Point2f>, expectedContour);

    auto rotatedContour = rotateContour(contour, angle);
    QVERIFY(rotatedContour.size() == expectedContour.size());

    for (size_t i = 0; i < rotatedContour.size(); i++)
    {
        QCOMPARE(rotatedContour[i].x, expectedContour[i].x);
        QCOMPARE(rotatedContour[i].y, expectedContour[i].y);
    }
}

void TestMetrology::testContourBoundingBox_data()
{
    QTest::addColumn<std::vector<cv::Point2f>>("contour");
    QTest::addColumn<cv::Rect2f>("expectedBoundingBox");

    QTest::newRow("empty contour")
        << std::vector<cv::Point2f>
            {
            }
        << cv::Rect2f
            {
                cv::Point2f(0, 0),
                cv::Point2f(0, 0),
            };

    QTest::newRow("single point")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0),
            }
        << cv::Rect2f
            {
                cv::Point2f(0, 0),
                cv::Point2f(0, 0),
            };

    QTest::newRow("multiple points")
        << std::vector<cv::Point2f>
            {
                cv::Point2f(0, 0),
                cv::Point2f(1.123, -2.5),
                cv::Point2f(3.05, 2),
            }
        << cv::Rect2f
            {
                cv::Point2f(0, -2.5),
                cv::Point2f(3.05, 2),
            };
}

void TestMetrology::testContourBoundingBox()
{
    QFETCH(std::vector<cv::Point2f>, contour);
    QFETCH(cv::Rect2f, expectedBoundingBox);

    QCOMPARE(contourBoundingBox(contour), expectedBoundingBox);
}

} //namespace filter
} //namespace precitec

QTEST_GUILESS_MAIN(precitec::filter::TestMetrology)
#include "testMetrology.moc"

