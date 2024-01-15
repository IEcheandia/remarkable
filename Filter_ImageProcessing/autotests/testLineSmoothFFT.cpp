#include <QTest>

#include "../lineSmoothFFT.cpp"

namespace precitec
{
namespace filter
{

class TestLineSmoothFFT : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSmoothLine();
};

void TestLineSmoothFFT::testSmoothLine()
{
    const int N = 20;

    std::vector<double> sin1; //base frequency
    std::vector<double> sin2; // 2 * base frequency
    std::vector<double> sinSum; //sum of sin1 and sin2

    for (int i = 0; i < N; ++i)
    {
        sin1.emplace_back(std::sin(i * 2 * M_PI / N));
        sin2.emplace_back(std::sin(2 * i * 2 * M_PI / N));
        sinSum.emplace_back(sin1.back() + sin2.back());
    }

    // filter sin2 from sinSum and compare sinSum with sin1
    const auto sinSumFiltered = smoothLine(sinSum, 1);

    QCOMPARE(sinSumFiltered.size(), sin1.size());

    // filtered function has to be close to sin1
    for (std::size_t i = 0; i < sinSumFiltered.size(); ++i)
    {
        QCOMPARE(sinSumFiltered[i], sin1[i]);
    }

    // smoothLine call with -1 as argument, should return the original line
    const auto sinSumNoFilter = smoothLine(sinSum, -1);

    QCOMPARE(sinSumNoFilter, sinSum);
}

} //namespace filter
} //namespace precitec

QTEST_GUILESS_MAIN(precitec::filter::TestLineSmoothFFT)
#include "testLineSmoothFFT.moc"
