#include <QTest>

#include "../parallelLocalExtremum.cpp"

namespace precitec
{
namespace filter
{

class TestParallelLocalExtremum : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPreprocessMinMax();
    void testLocalExtremum();
};

void TestParallelLocalExtremum::testPreprocessMinMax()
{
    const int height = 3;
    const int width = 7;
    const std::vector<uint8_t> image =
    {
        10, 5, 4, 4, 8, 20, 22,
        90, 80, 70, 60, 50, 40, 30,
        0, 0, 0, 1, 1, 1, 1,
    };

    const std::vector<uint8_t> expectedMinMax =
    {
        1, 0, 1, 1, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 1,
        0, 0, 1, 1, 0, 0, 0,
    };

    std::vector<uint8_t> minMax(image.size());

    preprocessMinMax(image.data(), height, width, width, minMax.data());

    for (std::size_t i = 0; i < image.size(); ++i)
    {
        QVERIFY((bool)expectedMinMax[i] == (bool)minMax[i]);
    }
}

void TestParallelLocalExtremum::testLocalExtremum()
{
    const std::vector<uint8_t> line =
    {
        5, 4, 5, 5, 6, 7, 4, 20, 100, 50, 30, 39, 20, 20, 50, 51, 30,
    };

    std::vector<uint8_t> minMax(line.size());
    preprocessMinMax(line.data(), 1, line.size(), line.size(), minMax.data());

    // Test case 1:
    // position 10 with value 30 and position 20 with value 20 should be local minimum
    // since it needs to climb from them more than 5 to reach a lower value.
    const std::vector<int> expectedMinimumIndex1 =
    {
        10, 12,
    };
    const auto minimumIndex1 = localMinimum(line.data(), minMax.data(), line.size(), 5, 5);
    QCOMPARE(expectedMinimumIndex1, minimumIndex1);

    // Test case 2:
    // Only position 12 with value 20 is local minimum. Position 10 is not local
    // minimum because it needs to climb only a value of 9 on the right side to reach
    // a lower value.
    const std::vector<int> expectedMinimumIndex2 =
    {
        12,
    };
    const auto minimumIndex2 = localMinimum(line.data(), minMax.data(), line.size(), 5, 10);
    QCOMPARE(expectedMinimumIndex2, minimumIndex2);

    // Test case 3:
    // All local peaks outputted
    const std::vector<int> expectedMinimumIndex3 =
    {
        1, 6, 10, 12,
    };
    const auto minimumIndex3 = localMinimum(line.data(), minMax.data(), line.size(), 0, 0);
    QCOMPARE(expectedMinimumIndex3, minimumIndex3);


    // Test case 4
    // Maximum peaks with prominence greater than 5
    const std::vector<int> expectedMaximumIndex1 =
    {
        8, 11, 15,
    };
    const auto maximumIndex1 = localMaximum(line.data(), minMax.data(), line.size(), 5, 5);
    QCOMPARE(expectedMaximumIndex1, maximumIndex1);

}

} //namespace filter
} //namespace precitec

QTEST_GUILESS_MAIN(precitec::filter::TestParallelLocalExtremum)
#include "testParallelLocalExtremum.moc"
