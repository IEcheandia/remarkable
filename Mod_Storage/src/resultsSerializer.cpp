#include "resultsSerializer.h"

#include "event/resultType.h"

#include <QFile>
#include <QVariant>

#include <Poco/UUID.h>

using precitec::interface::ImageContext;
using precitec::interface::GeoDoublearray;
using precitec::interface::ResultDoubleArray;
using precitec::interface::ResultType;

namespace precitec
{
namespace storage
{

/*
 * The magic version is generated through:
 * sha256 on "weldmaster" and taking the first four bytes.
 **/
static const quint32 s_magicHeaderNumber = 0xCE983826;
static const quint32 s_versionNumber = 4;

ResultsSerializer::ResultsSerializer() = default;
ResultsSerializer::~ResultsSerializer() = default;

void ResultsSerializer::writeHeader(QDataStream &out) const
{
    out.setVersion(QDataStream::Qt_5_10);
    out << s_magicHeaderNumber;
    out << s_versionNumber;
}

bool ResultsSerializer::verifyHeader(QDataStream &in) const
{
    quint32 magic;
    in >> magic;
    if (magic != s_magicHeaderNumber)
    {
        return false;
    }
    quint32 version;
    in >> version;
    if (version > s_versionNumber)
    {
        return false;
    }
    in.device()->setProperty("__wm_results_file_version", version);
    in.setVersion(QDataStream::Qt_5_10);
    return true;
}

bool ResultsSerializer::writeToFile(const QByteArray &data) const
{
    QFile file(m_directory.absoluteFilePath(m_fileName));
    if (file.exists())
    {
        // don't overwrite existing files
        return false;
    }
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }
    // qCompress is preserved only for backward compatability.
    // It does not give speed or size overhead with the following 0 level.
    // Please see https://doc.qt.io/qt-5/qbytearray.html#qCompress.
    const QByteArray compressedData = qCompress(data, 0);
    if (file.write(compressedData) != compressedData.size())
    {
        file.close();
        file.remove();
        return false;
    }
    return true;
}

QByteArray ResultsSerializer::readFile() const
{
    QFile file(m_directory.absoluteFilePath(m_fileName));
    if (!file.exists())
    {
        return QByteArray{};
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        return QByteArray{};
    }
    // qUncompress is preserved only for backward compatability.
    // In current version compression level is 0
    // and does not influence size and speed of compression.
    // Please see https://doc.qt.io/qt-5/qbytearray.html#qCompress.
    return qUncompress(file.readAll());
}

}
}

QDataStream &operator<<(QDataStream &out, const Poco::UUID &id)
{
    out << id.toString().c_str();
    return out;
}

QDataStream &operator>>(QDataStream &in, Poco::UUID &id)
{
    char *data;
    in >> data;
    id.parse(std::string(data));
    delete[] data;
    return in;
}

QDataStream &operator<<(QDataStream &out, const precitec::interface::ImageContext &context)
{
    out << context.SamplingX_;
    out << context.SamplingY_;
    out << context.measureTaskPosFlag();
    out << context.imageNumber();
    out << qint32(context.position());
    out << qint32(context.relativeTime());
    out << context.HW_ROI_x0;
    out << context.HW_ROI_y0;
    // TODO: trafo?
    // TODO: taskContext?
    out << context.taskContext().measureTask().get()->triggerDelta();
    return out;
}

QDataStream &operator>>(QDataStream &in, precitec::interface::ImageContext &context)
{
    in >> context.SamplingX_;
    in >> context.SamplingY_;
    qint32 measureTaskPosFlag;
    in >> measureTaskPosFlag;
    context.setMeasureTaskPositionFlag(precitec::interface::MeasureTaskPos(measureTaskPosFlag));
    qint32 imageNumber;
    in >> imageNumber;
    context.setImageNumber(imageNumber);
    qint32 position;
    in >> position;
    context.setPosition(position);
    qint32 time;
    in >> time;
    context.setTime(time);
    in >> context.HW_ROI_x0;
    in >> context.HW_ROI_y0;
    if (in.device()->property("__wm_results_file_version").value<quint32>() >= 2u)
    {
        qint32 delta;
        in >> delta;
        context.taskContext().measureTask().get()->setTriggerDelta(delta);
    }

    return in;
}

QDataStream &operator<<(QDataStream &out, const precitec::interface::ResultArgs &result)
{
    out << result.filterId();
    out << result.resultType();
    out << result.nioType();
    out << result.type();
    out << result.isNio();
    out << result.isValid();
    if (result.type() == precitec::interface::RegDoubleArray)
    {
        const auto values = result.value<double>();
        out << quint64(values.size());
        for (auto value : values)
        {
            out << value;
        }
        // deviation
        out << result.deviation<double>().start();
        out << result.deviation<double>().end();
    }
    else
    {
        out << quint64(0);
    }
    out << result.context();

    auto signalQualities = result.signalQuality();
    out << quint64(signalQualities.size());
    for (auto quality : signalQualities)
    {
        out << quality;
    }
    auto nioPercentages = result.nioPercentage();
    out << quint64(nioPercentages.size());
    for (auto percentage : nioPercentages)
    {
        out << percentage.x;
        out << percentage.y;
    }
    auto upperReference = result.upperReference();
    out << quint64(upperReference.size());
    for (auto currUpperRefValue : upperReference)
    {
        out << currUpperRefValue.x;
        out << currUpperRefValue.y;
    }
    auto lowerReference = result.lowerReference();
    out << quint64(lowerReference.size());
    for (auto currLowerRefValue : lowerReference)
    {
        out << currLowerRefValue.x;
        out << currLowerRefValue.y;
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, precitec::interface::ResultArgs &result)
{
    Poco::UUID filterId;
    in >> filterId;
    qint32 resultType;
    in >> resultType;
    qint32 nioType;
    in >> nioType;
    qint32 type;
    in >> type;
    bool isNio;
    in >> isNio;
    bool valid;
    in >> valid;

    GeoDoublearray data;
    quint64 valueSize;
    in >> valueSize;
    for (quint64 i = 0; i < valueSize; i++)
    {
        double value;
        in >> value;
        data.ref().getData().push_back(value);
        data.ref().getRank().push_back(precitec::filter::ValueRankType::eRankMax);
    }
    double start;
    in >> start;
    double end;
    in >> end;

    ImageContext context;
    in >> context;

    result = ResultDoubleArray{filterId, ResultType(resultType), ResultType(nioType), context, data, precitec::geo2d::TRange<double>{start, end}, isNio};
    result.setValid(valid);


    if (in.device()->property("__wm_results_file_version").value<quint32>() >= 2u)
    {
        std::vector<float> signalQuality{};
        quint64 signalQualitySize;
        in >> signalQualitySize;
        signalQuality.reserve(signalQualitySize);
        for (quint64 i = 0; i < signalQualitySize; i++)
        {
            float quality;
            in >> quality;
            signalQuality.push_back(quality);
        }

        precitec::geo2d::VecDPoint nioPercentage{};
        quint64 nioPercentageSize;
        in >> nioPercentageSize;
        nioPercentage.reserve(nioPercentageSize);
        for (quint64 i = 0; i < nioPercentageSize; i++)
        {
            double x,y;
            in >> x;
            in >> y;
            nioPercentage.push_back(precitec::geo2d::DPoint(x,y));
        }

        result.setQuality(signalQuality);
        result.setNioResult(nioPercentage);
    }
    if (in.device()->property("__wm_results_file_version").value<quint32>() == 3u)
    {
        precitec::geo2d::VecDPoint upperReference{};
        quint64 upperReferenceSize;
        in >> upperReferenceSize;
        upperReference.reserve(upperReferenceSize);
        const auto sampleDistance = (double) context.taskContext().measureTask().get()->triggerDelta() / valueSize;
        for (quint64 i = 0; i < upperReferenceSize; i++)
        {
            float currUpperRefValue;
            in >> currUpperRefValue;
            upperReference.emplace_back(0.001 * (context.position() + i * sampleDistance), currUpperRefValue);
        }
        precitec::geo2d::VecDPoint lowerReference{};
        quint64 lowerReferenceSize;
        in >> lowerReferenceSize;
        lowerReference.reserve(lowerReferenceSize);
        for (quint64 i = 0; i < lowerReferenceSize; i++)
        {
            float currLowerRefValue;
            in >> currLowerRefValue;
            lowerReference.emplace_back(0.001 * (context.position() + i * sampleDistance), currLowerRefValue);
        }
        result.setUpperReference(upperReference);
        result.setLowerReference(lowerReference);
    }
    if (in.device()->property("__wm_results_file_version").value<quint32>() > 3u)
    {
        precitec::geo2d::VecDPoint upperReference{};
        quint64 upperReferenceSize;
        in >> upperReferenceSize;
        upperReference.reserve(upperReferenceSize);
        for (quint64 i = 0; i < upperReferenceSize; i++)
        {
            double x,y;
            in >> x;
            in >> y;
            upperReference.emplace_back(x,y);
        }
        precitec::geo2d::VecDPoint lowerReference{};
        quint64 lowerReferenceSize;
        in >> lowerReferenceSize;
        lowerReference.reserve(lowerReferenceSize);
        for (quint64 i = 0; i < lowerReferenceSize; i++)
        {
            double x,y;
            in >> x;
            in >> y;
            lowerReference.emplace_back(x,y);
        }
        result.setUpperReference(upperReference);
        result.setLowerReference(lowerReference);
    }

    return in;
}
