#include <QTest>
#include <QSignalSpy>

#include "../src/resultsSerializer.h"
#include "../src/resultsWriterCommand.h"

#include <QDataStream>
#include <QTemporaryFile>

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

#include <unistd.h>

using precitec::interface::ImageContext;
using precitec::interface::ResultArgs;
using precitec::interface::ResultDoubleArray;
using precitec::interface::GeoDoublearray;
using precitec::storage::ResultsSerializer;
using precitec::storage::ResultsWriterCommand;

std::map< std::string, int> mapImageContextFields(ImageContext context)
{
    std::map< std::string, int> fields;
    fields["imageNumber"] = context.imageNumber();
    fields["position"] = context.position();
    fields["relativeTime"] = context.relativeTime();
    fields["measureTaskPosFlag"] = context.measureTaskPosFlag();
    fields["HW_ROI_x0"] = context.HW_ROI_x0;
    fields["HW_ROI_y0"] = context.HW_ROI_y0;
    fields["SamplingX_"] = static_cast<int>(context.SamplingX_);  //this field is actually a double
    fields["SamplingY_"] = static_cast<int>(context.SamplingY_); //this field is actually a double
    return fields;
}

bool compareContextFields( ImageContext actual, ImageContext expected)
{
    auto actualFields = mapImageContextFields(actual);
    auto expectedFields = mapImageContextFields(expected);
    bool equal = true;
    for (auto & it: actualFields)
    {
        auto fieldName = it.first;
        if (it.second != expectedFields[fieldName])
        {
            qDebug() << fieldName.c_str() << " actual " << it.second << " expected " <<  expectedFields[fieldName];
            equal = false;
        }
    }
    return equal;

}


class TestResultsSerializer : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSerializeUuid();
    void testSerializeImageContext();
    void testSerializeEmptyResult_data();
    void testSerializeEmptyResult();
    void testSerializeResult();
    void testSerializeToExistingFile();
    void testSerializeToNonWritableFile();
    void testDeserializeNotExistingFile();
    void testDeserializeNonReadableFile();
    void testDeserializeNotCompressedFile();
    void testReadMagicVersionFailure_data();
    void testReadMagicVersionFailure();
    void testSerialization();
    void testResultsWriterCommand();
    void testResultsDeserializeForOldFormatCompressLevel();
};

void TestResultsSerializer::testSerializeUuid()
{
    Poco::UUID id = Poco::UUIDGenerator().createRandom();
    QCOMPARE(id.isNull(), false);

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << id;

    QDataStream in(data);
    Poco::UUID readId;
    in >> readId;
    QCOMPARE(id, readId);
}

void TestResultsSerializer::testSerializeImageContext()
{
    ImageContext ctx;
    ctx.setImageNumber(3);
    ctx.setPosition(4);
    ctx.setTime(5);
    ctx.setMeasureTaskPositionFlag(precitec::interface::eMiddleImage);
    ctx.HW_ROI_x0 = 6;
    ctx.HW_ROI_y0 = 7;
    ctx.SamplingX_ = 8.1;
    ctx.SamplingY_ = 9.2;

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << ctx;

    QDataStream in(data);
    ImageContext deserialized;
    in >> deserialized;
    QCOMPARE(deserialized.imageNumber(), 3);
    QCOMPARE(deserialized.position(), 4l);
    QCOMPARE(deserialized.relativeTime(), 5);
    QCOMPARE(deserialized.measureTaskPosFlag(), precitec::interface::eMiddleImage);
    QCOMPARE(deserialized.HW_ROI_x0, 6);
    QCOMPARE(deserialized.HW_ROI_y0, 7);
    QCOMPARE(deserialized.SamplingX_, 8.1);
    QCOMPARE(deserialized.SamplingY_, 9.2);
    QVERIFY(compareContextFields(deserialized, ctx));
}

void TestResultsSerializer::testSerializeEmptyResult_data()
{
    QTest::addColumn<bool>("isNio");

    QTest::newRow("nio") << true;
    QTest::newRow("io") << false;
}

void TestResultsSerializer::testSerializeEmptyResult()
{
    QFETCH(bool, isNio);
    ImageContext ctx;
    ctx.setImageNumber(3);
    ctx.setPosition(4);
    ctx.setTime(5);
    ctx.setMeasureTaskPositionFlag(precitec::interface::eMiddleImage);
    ctx.HW_ROI_x0 = 6;
    ctx.HW_ROI_y0 = 7;
    ctx.SamplingX_ = 8.1;
    ctx.SamplingY_ = 9.2;

    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(),
        precitec::interface::Missmatch,
        precitec::interface::XCoordOutOfLimits,
        ctx,
        GeoDoublearray{},
        precitec::geo2d::TRange<double>{}, isNio};
    // empty data
    QCOMPARE(result.isValid(), false);
    QCOMPARE(result.type(), int(precitec::interface::RegTypes::RegDoubleArray));

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << result;

    QDataStream in(data);
    ResultArgs deserialized;
    in >> deserialized;
    QCOMPARE(deserialized.type(), int(precitec::interface::RegTypes::RegDoubleArray));


    QCOMPARE(deserialized.filterId(), result.filterId());
    QCOMPARE(deserialized.resultType(), precitec::interface::Missmatch);
    QCOMPARE(deserialized.nioType(), precitec::interface::XCoordOutOfLimits);
    QCOMPARE(deserialized.isNio(), isNio);
    QCOMPARE(deserialized.isValid(), false);
    QVERIFY(compareContextFields(deserialized.context(), result.context()));
    QCOMPARE(deserialized.value<double>().size(), 0u);
    QCOMPARE(deserialized.rank().size(), 0u);


}

void TestResultsSerializer::testSerializeResult()
{
    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());
    values.ref().getRank().assign(3,200);

    ImageContext ctx;
    ctx.setImageNumber(3);
    ctx.setPosition(4);
    ctx.setTime(5);
    ctx.setMeasureTaskPositionFlag(precitec::interface::eMiddleImage);
    ctx.HW_ROI_x0 = 6;
    ctx.HW_ROI_y0 = 7;
    ctx.SamplingX_ = 8.1;
    ctx.SamplingY_ = 9.2;

    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ctx, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);
    QCOMPARE(result.type(), int(precitec::interface::RegTypes::RegDoubleArray));

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << result;

    QDataStream in(data);
    ResultArgs deserialized;
    in >> deserialized;

    QCOMPARE(deserialized.type(), int(precitec::interface::RegTypes::RegDoubleArray));


    QCOMPARE(deserialized.filterId(), result.filterId());
    QCOMPARE(deserialized.resultType(), precitec::interface::Missmatch);
    QCOMPARE(deserialized.nioType(), precitec::interface::XCoordOutOfLimits);
    QCOMPARE(deserialized.isNio(), true);
    QCOMPARE(deserialized.isValid(), true);
    QCOMPARE(deserialized.deviation<double>().start(), 0.0);
    QCOMPARE(deserialized.deviation<double>().end(), 2.0);
    const auto deserializedValues = deserialized.value<double>();
    QCOMPARE(deserializedValues.size(), 3u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);

    QVERIFY(compareContextFields(deserialized.context(), result.context()));
}

void TestResultsSerializer::testSerializeToExistingFile()
{
    ResultsSerializer serializer;
    QCOMPARE(serializer.fileName(), QString());
    QCOMPARE(serializer.directory(), QDir());

    QTemporaryFile file;
    QVERIFY(file.open());
    QFileInfo info(file.fileName());
    serializer.setDirectory(info.absoluteDir());
    QCOMPARE(serializer.directory(), info.absoluteDir());
    serializer.setFileName(info.fileName());
    QCOMPARE(serializer.fileName(), info.fileName());

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());

    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};

    // serializing should fail as the file already exists
    QCOMPARE(serializer.serialize(std::vector<ResultArgs>{result}), false);
}

void TestResultsSerializer::testSerializeToNonWritableFile()
{
    if (getuid() == 0)
    {
        QSKIP("Test cannot be run as root");
        return;
    }
    ResultsSerializer serializer;
    serializer.setDirectory(QDir(QStringLiteral("/etc/")));
    serializer.setFileName(QStringLiteral("passwd"));

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());

    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};

    // serializing should fail as writing to passwd is not allowed
    QCOMPARE(serializer.serialize(std::vector<ResultArgs>{result}), false);
}

void TestResultsSerializer::testDeserializeNotExistingFile()
{
    QTemporaryDir dir;
    ResultsSerializer serializer;
    serializer.setDirectory(QDir(dir.path()));
    serializer.setFileName(QStringLiteral("thisfiledoesnotexist"));

    const auto results = serializer.deserialize<std::vector>();
    QCOMPARE(results.empty(), true);
}

void TestResultsSerializer::testDeserializeNonReadableFile()
{
    if (getuid() == 0)
    {
        QSKIP("Test cannot be run as root");
        return;
    }
    ResultsSerializer serializer;
    serializer.setDirectory(QDir(QStringLiteral("/etc/")));
    serializer.setFileName(QStringLiteral("shadow"));

    const auto results = serializer.deserialize<std::vector>();
    QCOMPARE(results.empty(), true);
}

void TestResultsSerializer::testDeserializeNotCompressedFile()
{
    ResultsSerializer serializer;
    serializer.setDirectory(QDir(QStringLiteral("/etc/")));
    serializer.setFileName(QStringLiteral("passwd"));

    const auto results = serializer.deserialize<std::vector>();
    QCOMPARE(results.empty(), true);
}

void TestResultsSerializer::testReadMagicVersionFailure_data()
{
    QTest::addColumn<quint32>("magic");
    QTest::addColumn<quint32>("version");

    QTest::newRow("invalid magic") << quint32(0xCE983827) << quint32(1);
    QTest::newRow("invalid version") << quint32(0xCE983826) << quint32(100);
}

void TestResultsSerializer::testReadMagicVersionFailure()
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QFETCH(quint32, magic);
    QFETCH(quint32, version);
    out << magic;
    out << version;

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());

    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    out << std::vector<ResultArgs>{result};

    QTemporaryFile file;
    QVERIFY(file.open());
    file.write(qCompress(data, 5));
    file.flush();

    ResultsSerializer serializer;
    QFileInfo info(file.fileName());
    serializer.setDirectory(info.absoluteDir());
    serializer.setFileName(info.fileName());

    const auto results = serializer.deserialize<std::vector>();
    QCOMPARE(results.empty(), true);
}

void TestResultsSerializer::testSerialization()
{
    QTemporaryDir dir;
    ResultsSerializer serializer;
    serializer.setDirectory(QDir(dir.path()));
    serializer.setFileName(QStringLiteral("test"));

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    values.ref().getData().push_back(2.0);
    values.ref().getRank().push_back(255);
    ResultDoubleArray result2{Poco::UUIDGenerator().createRandom(), precitec::interface::AnalysisOK, precitec::interface::AnalysisErrBadROI, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 3.0), false};
    values.ref().getData().push_back(3.0);
    values.ref().getRank().push_back(255);
    ResultDoubleArray result3{Poco::UUIDGenerator().createRandom(), precitec::interface::AnalysisOK, precitec::interface::AnalysisErrBadROI, ImageContext{}, values, precitec::geo2d::TRange<double>(1.0, 3.5), false};

    serializer.serialize(std::vector<ResultArgs>{result, result2, result3});

    const auto deserialized = serializer.deserialize<std::vector>();
    QCOMPARE(deserialized.size(), 3u);
    QCOMPARE(deserialized[0].filterId(), result.filterId());
    QCOMPARE(deserialized[0].resultType(), precitec::interface::Missmatch);
    QCOMPARE(deserialized[0].nioType(), precitec::interface::XCoordOutOfLimits);
    QCOMPARE(deserialized[0].isNio(), true);
    QCOMPARE(deserialized[0].isValid(), true);
    QCOMPARE(deserialized[0].deviation<double>().start(), 0.0);
    QCOMPARE(deserialized[0].deviation<double>().end(), 2.0);
    auto deserializedValues = deserialized[0].value<double>();
    QCOMPARE(deserializedValues.size(), 3u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);

    QVERIFY(compareContextFields(deserialized[0].context(), result.context()));

    QCOMPARE(deserialized[1].filterId(), result2.filterId());
    QCOMPARE(deserialized[1].resultType(), precitec::interface::AnalysisOK);
    QCOMPARE(deserialized[1].nioType(), precitec::interface::AnalysisErrBadROI);
    QCOMPARE(deserialized[1].isNio(), false);
    QCOMPARE(deserialized[1].isValid(), true);
    QCOMPARE(deserialized[1].deviation<double>().start(), 0.0);
    QCOMPARE(deserialized[1].deviation<double>().end(), 3.0);
    deserializedValues = deserialized[1].value<double>();
    QCOMPARE(deserializedValues.size(), 4u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);
    QCOMPARE(deserializedValues[3], 2.0);

    QVERIFY(compareContextFields(deserialized[1].context(), result2.context()));

    QCOMPARE(deserialized[2].filterId(), result3.filterId());
    QCOMPARE(deserialized[2].resultType(), precitec::interface::AnalysisOK);
    QCOMPARE(deserialized[2].nioType(), precitec::interface::AnalysisErrBadROI);
    QCOMPARE(deserialized[2].isNio(), false);
    QCOMPARE(deserialized[2].isValid(), true);
    QCOMPARE(deserialized[2].deviation<double>().start(), 1.0);
    QCOMPARE(deserialized[2].deviation<double>().end(), 3.5);
    deserializedValues = deserialized[2].value<double>();
    QCOMPARE(deserializedValues.size(), 5u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);
    QCOMPARE(deserializedValues[3], 2.0);
    QCOMPARE(deserializedValues[4], 3.0);

    QVERIFY(compareContextFields(deserialized[2].context(), result3.context()));
}

void TestResultsSerializer::testResultsWriterCommand()
{
    QTemporaryDir dir;

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    values.ref().getData().push_back(2.0);
    values.ref().getRank().push_back(255);
    ResultDoubleArray result2{Poco::UUIDGenerator().createRandom(), precitec::interface::AnalysisOK, precitec::interface::AnalysisErrBadROI, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 3.0), false};
    values.ref().getData().push_back(3.0);
    values.ref().getRank().push_back(255);
    ResultDoubleArray result3{Poco::UUIDGenerator().createRandom(), precitec::interface::AnalysisOK, precitec::interface::AnalysisErrBadROI, ImageContext{}, values, precitec::geo2d::TRange<double>(1.0, 3.5), false};

    ResultsWriterCommand writer{dir.path(), 1, std::list<ResultArgs>{result, result2, result3}};
    writer.execute();

    ResultsSerializer serializer;
    serializer.setDirectory(QDir(dir.path()));
    serializer.setFileName(QStringLiteral("1.result"));
    const auto deserialized = serializer.deserialize<std::vector>();
    QCOMPARE(deserialized.size(), 3u);
    QCOMPARE(deserialized[0].filterId(), result.filterId());
    QCOMPARE(deserialized[0].resultType(), precitec::interface::Missmatch);
    QCOMPARE(deserialized[0].nioType(), precitec::interface::XCoordOutOfLimits);
    QCOMPARE(deserialized[0].isNio(), true);
    QCOMPARE(deserialized[0].isValid(), true);
    QCOMPARE(deserialized[0].deviation<double>().start(), 0.0);
    QCOMPARE(deserialized[0].deviation<double>().end(), 2.0);
    auto deserializedValues = deserialized[0].value<double>();
    QCOMPARE(deserializedValues.size(), 3u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);

    QCOMPARE(deserialized[1].filterId(), result2.filterId());
    QCOMPARE(deserialized[1].resultType(), precitec::interface::AnalysisOK);
    QCOMPARE(deserialized[1].nioType(), precitec::interface::AnalysisErrBadROI);
    QCOMPARE(deserialized[1].isNio(), false);
    QCOMPARE(deserialized[1].isValid(), true);
    QCOMPARE(deserialized[1].deviation<double>().start(), 0.0);
    QCOMPARE(deserialized[1].deviation<double>().end(), 3.0);
    deserializedValues = deserialized[1].value<double>();
    QCOMPARE(deserializedValues.size(), 4u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);
    QCOMPARE(deserializedValues[3], 2.0);


    QCOMPARE(deserialized[2].filterId(), result3.filterId());
    QCOMPARE(deserialized[2].resultType(), precitec::interface::AnalysisOK);
    QCOMPARE(deserialized[2].nioType(), precitec::interface::AnalysisErrBadROI);
    QCOMPARE(deserialized[2].isNio(), false);
    QCOMPARE(deserialized[2].isValid(), true);
    QCOMPARE(deserialized[2].deviation<double>().start(), 1.0);
    QCOMPARE(deserialized[2].deviation<double>().end(), 3.5);
    deserializedValues = deserialized[2].value<double>();
    QCOMPARE(deserializedValues.size(), 5u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);
    QCOMPARE(deserializedValues[3], 2.0);
    QCOMPARE(deserializedValues[4], 3.0);
}

void TestResultsSerializer::testResultsDeserializeForOldFormatCompressLevel()
{
    const QString fileName = QFINDTESTDATA("testdata/compressed_result_files/test");
    QVERIFY(!fileName.isEmpty());
    auto const fileInfo = QFileInfo{fileName};
    auto dir = QDir(fileInfo.absoluteDir());
    QCOMPARE(dir.exists(), true);

    // decompress the file with old compress level 5
    ResultsSerializer serializer;
    serializer.setDirectory(dir);
    serializer.setFileName(fileInfo.fileName());
    const auto deserialized = serializer.deserialize<std::vector>();

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    values.ref().getData().push_back(2.0);
    values.ref().getRank().push_back(255);
    ResultDoubleArray result2{Poco::UUIDGenerator().createRandom(), precitec::interface::AnalysisOK, precitec::interface::AnalysisErrBadROI, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 3.0), false};
    values.ref().getData().push_back(3.0);
    values.ref().getRank().push_back(255);
    ResultDoubleArray result3{Poco::UUIDGenerator().createRandom(), precitec::interface::AnalysisOK, precitec::interface::AnalysisErrBadROI, ImageContext{}, values, precitec::geo2d::TRange<double>(1.0, 3.5), false};

    QCOMPARE(deserialized.size(), 3u);
    QCOMPARE(deserialized[0].resultType(), precitec::interface::Missmatch);
    QCOMPARE(deserialized[0].nioType(), precitec::interface::XCoordOutOfLimits);
    QCOMPARE(deserialized[0].isNio(), true);
    QCOMPARE(deserialized[0].isValid(), true);
    QCOMPARE(deserialized[0].deviation<double>().start(), 0.0);
    QCOMPARE(deserialized[0].deviation<double>().end(), 2.0);
    auto deserializedValues = deserialized[0].value<double>();
    QCOMPARE(deserializedValues.size(), 3u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);

    QCOMPARE(deserialized[1].resultType(), precitec::interface::AnalysisOK);
    QCOMPARE(deserialized[1].nioType(), precitec::interface::AnalysisErrBadROI);
    QCOMPARE(deserialized[1].isNio(), false);
    QCOMPARE(deserialized[1].isValid(), true);
    QCOMPARE(deserialized[1].deviation<double>().start(), 0.0);
    QCOMPARE(deserialized[1].deviation<double>().end(), 3.0);
    deserializedValues = deserialized[1].value<double>();
    QCOMPARE(deserializedValues.size(), 4u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);
    QCOMPARE(deserializedValues[3], 2.0);

    QCOMPARE(deserialized[2].resultType(), precitec::interface::AnalysisOK);
    QCOMPARE(deserialized[2].nioType(), precitec::interface::AnalysisErrBadROI);
    QCOMPARE(deserialized[2].isNio(), false);
    QCOMPARE(deserialized[2].isValid(), true);
    QCOMPARE(deserialized[2].deviation<double>().start(), 1.0);
    QCOMPARE(deserialized[2].deviation<double>().end(), 3.5);
    deserializedValues = deserialized[2].value<double>();
    QCOMPARE(deserializedValues.size(), 5u);
    QCOMPARE(deserializedValues[0], 0.0);
    QCOMPARE(deserializedValues[1], 0.1);
    QCOMPARE(deserializedValues[2], 1.0);
    QCOMPARE(deserializedValues[3], 2.0);
    QCOMPARE(deserializedValues[4], 3.0);
}

QTEST_GUILESS_MAIN(TestResultsSerializer)
#include "testResultsSerializer.moc"
