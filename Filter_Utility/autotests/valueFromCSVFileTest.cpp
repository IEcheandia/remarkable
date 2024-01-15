#include <QTest>

#include "../valueFromCSVFile.h"

#include "fliplib/NullSourceFilter.h"

#include <filesystem>

using precitec::filter::ValueFromCSVFile;
using precitec::interface::GeoDoublearray;

namespace fs = std::filesystem;

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter()
        : fliplib::BaseFilter("dummy")
    {
    }
    void proceed(const void* sender, fliplib::PipeEventArgs& e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }
    int getFilterType() const override { return BaseFilterInterface::SINK; }

    bool isProceedCalled() const { return m_proceedCalled; }

    void resetProceedCalled() { m_proceedCalled = false; }

private:
    bool m_proceedCalled = false;
};

struct CSVFilterTestInput
{
    double row;
    double col;
    std::string filename;
    int decimalSeparator = 0;
};

Q_DECLARE_METATYPE(CSVFilterTestInput)

class ValueFromCSVFileTest : public QObject
{
    Q_OBJECT
private:
    inline static constexpr int m_imageNumber = 0;
    inline static constexpr int m_pipeGroup = 1;
    inline static const fs::path m_testDataFolder = QFINDTESTDATA("testData").toStdString();

    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<GeoDoublearray> rowIn{&sourceFilter, "Row"};
    fliplib::SynchronePipe<GeoDoublearray> colIn{&sourceFilter, "Column"};

    DummyFilter dummyFilter;
    precitec::interface::ImageContext context;

    const GeoDoublearray validGeo{context,
                                  precitec::geo2d::Doublearray{1, 1.0, precitec::filter::eRankMax},
                                  precitec::interface::ResultType::AnalysisOK, precitec::interface::Perfect};

    bool setUpPipes(ValueFromCSVFile& underTest)
    {
        bool success = underTest.connectPipe(&rowIn, m_pipeGroup);
        success &= underTest.connectPipe(&colIn, m_pipeGroup);
        success &= dummyFilter.connectPipe(underTest.findPipe("ValueOut"), 0);
        return success;
    }

    void setParameters(const CSVFilterTestInput& input, ValueFromCSVFile& underTest)
    {
        underTest.getParameters().update("Filename", fliplib::Parameter::TYPE_string, input.filename);
        underTest.getParameters().update("DecimalSeparator", fliplib::Parameter::TYPE_int, input.decimalSeparator);
        underTest.setParameter();
    }

    void signalInput(const CSVFilterTestInput& input)
    {
        using namespace precitec::interface;

        int rowRank = precitec::filter::eRankMax;
        int colRank = precitec::filter::eRankMax;

        GeoDoublearray rowGeo{context,
                              precitec::geo2d::Doublearray{1, input.row, rowRank},
                              ResultType::AnalysisOK, Perfect};
        GeoDoublearray colGeo{context,
                              precitec::geo2d::Doublearray{1, input.col, colRank},
                              ResultType::AnalysisOK, Perfect};
        rowIn.signal(rowGeo);
        colIn.signal(colGeo);
    }

public:
    ValueFromCSVFileTest()
    {
        rowIn.setTag("RowIn");
        colIn.setTag("ColumnIn");
        context.setImageNumber(m_imageNumber);
    }

private Q_SLOTS:
    void testDoubleIndexedTable_withValidInput_hasInputRank()
    {
        using namespace precitec::interface;
        precitec::filter::DoubleIndexedTable underTest{};
        underTest.emplace_row({0.1, 0.2, 0.3});
        underTest.emplace_row({0.4, 0.5, 0.6});
        const auto result = underTest.valueOfCell(validGeo, validGeo);
        QCOMPARE(result.rank(), Perfect);
    }

    void testDoubleIndexedTable_withoutRowOrColumnIndex_hasBadRank()
    {
        using namespace precitec::interface;
        precitec::filter::DoubleIndexedTable underTest{};

        const GeoDoublearray emptyGeo{context,
                                      precitec::geo2d::Doublearray{},
                                      ResultType::AnalysisOK, Perfect};
        const auto resultRow = underTest.valueOfCell(emptyGeo, validGeo);
        QCOMPARE(resultRow.rank(), NotPresent);

        const auto resultCol = underTest.valueOfCell(validGeo, emptyGeo);
        QCOMPARE(resultCol.rank(), NotPresent);
    }

    void testDoubleIndexedTable_withBadOuterRank_hasBadRank()
    {
        using namespace precitec::interface;
        precitec::filter::DoubleIndexedTable underTest{};

        const GeoDoublearray badOuterRank{context,
                                          precitec::geo2d::Doublearray{1, 1.0, precitec::filter::eRankMax},
                                          ResultType::AnalysisOK, NotPresent};
        const auto resultCol = underTest.valueOfCell(badOuterRank, validGeo);
        QCOMPARE(resultCol.rank(), NotPresent);

        const auto resultRow = underTest.valueOfCell(validGeo, badOuterRank);
        QCOMPARE(resultRow.rank(), NotPresent);
    }

    void testDoubleIndexedTable_withBadInnerRank_hasBadRank()
    {
        using namespace precitec::interface;
        precitec::filter::DoubleIndexedTable underTest{};
        const GeoDoublearray badInnerRank{context,
                                          precitec::geo2d::Doublearray{1, 1.0, precitec::filter::eRankMin},
                                          ResultType::AnalysisOK, Limit};
        const auto resultCol = underTest.valueOfCell(badInnerRank, validGeo);
        QCOMPARE(resultCol.rank(), NotPresent);

        const auto resultRow = underTest.valueOfCell(validGeo, badInnerRank);
        QCOMPARE(resultRow.rank(), NotPresent);
    }

    void testCtor()
    {
        ValueFromCSVFile underTest{};
        QCOMPARE(underTest.name(), std::string("ValueFromCSVFile"));
        QVERIFY(underTest.findPipe("ValueOut") != nullptr);
        QVERIFY(underTest.findPipe("NotAValidPipe") == nullptr);

        auto& parameters = underTest.getParameters();
        QVERIFY(parameters.exists("Filename"));
        QCOMPARE(parameters.findParameter("Filename").getType(),
                 fliplib::Parameter::TYPE_string);
        QCOMPARE(
            parameters.findParameter("Filename").getValue().convert<std::string>(),
            std::string{});

        QVERIFY(parameters.exists("DecimalSeparator"));
        QCOMPARE(parameters.findParameter("DecimalSeparator").getType(),
                 fliplib::Parameter::TYPE_int);
        QCOMPARE(
            parameters.findParameter("DecimalSeparator").getValue().convert<int>(),
            0);
    }

    void testProceed_data()
    {
        using namespace precitec::interface;
        using namespace precitec::filter;

        QTest::addColumn<CSVFilterTestInput>("input");
        QTest::addColumn<double>("expected");
        QTest::addColumn<double>("expectedRank");
        QTest::addColumn<int>("expectedInnerRank");

        QTest::newRow("Integer from row 0 col 2") << CSVFilterTestInput{0.0, 2.0, "valueFromCSVFile_integers.csv"} << 8.0 << Perfect << 255;
        QTest::newRow("Integer from row 1 col 3") << CSVFilterTestInput{1.0, 3.0, "valueFromCSVFile_integers.csv"} << 14.0 << Perfect << 255;
        QTest::newRow("Integer from row 1.5 col 3.9") << CSVFilterTestInput{1.5, 3.9, "valueFromCSVFile_integers.csv"} << 14.0 << Perfect << 255;
        QTest::newRow("Double") << CSVFilterTestInput{0.0, 0.0, "valueFromCSVFile_doubles.csv"} << 0.678 << Perfect << 255;
        QTest::newRow("Doubles with spaces") << CSVFilterTestInput{0.0, 0.0, "valueFromCSVFile_doubles_with_spaces.csv"} << 0.678 << Perfect << 255;
        QTest::newRow("Double negative") << CSVFilterTestInput{0.0, 1.0, "valueFromCSVFile_doubles.csv"} << -42.42 << Perfect << 255;
        QTest::newRow("Double scientific negative exponent") << CSVFilterTestInput{0.0, 2.0, "valueFromCSVFile_doubles.csv"} << 0.0725 << Perfect << 255;
        QTest::newRow("Double scientific negative exponent") << CSVFilterTestInput{0.0, 2.0, "valueFromCSVFile_doubles.csv"} << 0.0725 << Perfect << 255;
        QTest::newRow("Tab-separated") << CSVFilterTestInput{0.0, 3.0, "valueFromCSVFile_tab_separated.csv"} << 8765.0 << Perfect << 255;
        QTest::newRow("Semicolon-separated") << CSVFilterTestInput{0.0, 3.0, "valueFromCSVFile_semicolon_separated.csv"} << 8765.0 << Perfect << 255;
        QTest::newRow("Decimal comma semicolon-separated") << CSVFilterTestInput{1.0, 2.0, "valueFromCSVFile_decimal_comma.csv", 1} << 0.0725 << Perfect << 255;
        QTest::newRow("Comment") << CSVFilterTestInput{0.0, 3.0, "valueFromCSVFile_comment.csv"} << 8765.0 << Perfect << 255;
        // Error cases
        QTest::newRow("Missing file") << CSVFilterTestInput{0.0, 0.0, "missing.csv"} << 0.0 << NotPresent << 0;
        QTest::newRow("Not parsable as double array") << CSVFilterTestInput{0.0, 0.0, "valueFromCSVFile_strings.csv"} << 0.0 << NotPresent << 0;
        QTest::newRow("Row index out of range") << CSVFilterTestInput{2.0, 3.0, "valueFromCSVFile_integers.csv"} << 0.0 << NotPresent << 0;
        QTest::newRow("Col index out of range") << CSVFilterTestInput{1.0, 7.0, "valueFromCSVFile_integers.csv"} << 0.0 << NotPresent << 0;
        QTest::newRow("Wrong decimal separator") << CSVFilterTestInput{1.0, 3.0, "valueFromCSVFile_integers.csv", 1} << 0.0 << NotPresent << 0;
        QTest::newRow("Line too long") << CSVFilterTestInput{1.0, 3.0, "valueFromCSVFile_line_too_long.csv"} << 0.0 << NotPresent << 0;
        QTest::newRow("Line too short") << CSVFilterTestInput{1.0, 3.0, "valueFromCSVFile_line_too_short.csv"} << 0.0 << NotPresent << 0;
    }

    void testProceed()
    {
        QFETCH(CSVFilterTestInput, input);

        ValueFromCSVFile underTest{m_testDataFolder};

        QVERIFY(setUpPipes(underTest));
        setParameters(input, underTest);
        signalInput(input);

        auto resultPipe = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(
            underTest.findPipe("ValueOut"));

        QVERIFY(resultPipe != nullptr);
        QVERIFY(resultPipe->dataAvailable(m_imageNumber));
        const auto& result = resultPipe->read(m_imageNumber);
        QCOMPARE(result.ref().size(), 1ul);

        double resultValue = result.ref().getData().front();
        QTEST(resultValue, "expected");

        double resultRank = result.rank();
        QTEST(resultRank, "expectedRank");

        int resultRankInner = result.ref().getRank().front();
        QTEST(resultRankInner, "expectedInnerRank");
    }
};

QTEST_APPLESS_MAIN(ValueFromCSVFileTest)
#include "valueFromCSVFileTest.moc"