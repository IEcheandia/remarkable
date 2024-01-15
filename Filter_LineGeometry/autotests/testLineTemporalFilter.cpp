#include <QTest>
#include <fliplib/NullSourceFilter.h>
#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

#include "../lineTemporalFilter.h"
#include "filter/armStates.h"

class LineTemporalFilterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
};

using precitec::geo2d::Doublearray;
using precitec::geo2d::VecDoublearray;
using precitec::interface::GeoVecDoublearray;
Q_DECLARE_METATYPE(Doublearray)
Q_DECLARE_METATYPE(GeoVecDoublearray)

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter()
        : fliplib::BaseFilter("dummy")
    {
    }
    void proceed(const void* sender, fliplib::PipeEventArgs& event) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(event)
        preSignalAction();
        m_proceedCalled = true;
    }
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
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

std::vector<VecDoublearray> splitIntoBlocks(const VecDoublearray& array, int frameSize)
{
    std::vector<VecDoublearray> result;
    if (frameSize == 0)
    {
        return result;
    }
    int numFrames = array.size() / frameSize;
    int dataCounter = 0;
    for (int imgNumber = 0; imgNumber < numFrames; imgNumber++)
    {
        VecDoublearray frame(frameSize);
        for (int i = 0; i < frameSize; i++)
        {
            assert(dataCounter < int(array.size()));
            frame[i] = array[dataCounter];
            dataCounter++;
        }
        result.push_back(frame);
    }
    return result;
}

void LineTemporalFilterTest::testCtor()
{
    precitec::filter::LineTemporalFilter filter;

    QCOMPARE(filter.name(), std::string("LineTemporalFilter"));
    QVERIFY(filter.findPipe("Result"));
}

void LineTemporalFilterTest::testProceed_data()
{
    using namespace precitec::interface;
    using namespace precitec::geo2d;

    QTest::addColumn<int>("parameter_operation");
    QTest::addColumn<int>("parameter_numImages");
    QTest::addColumn<int>("parameter_startImage");
    QTest::addColumn<std::vector<GeoVecDoublearray>>("input");
    QTest::addColumn<std::vector<Doublearray>>("expectedResult");

    {
        Doublearray line(10, 50, 255);
        std::iota(line.getData().begin(), line.getData().end(), 0.0);
        VecDoublearray constantLines(5, line);
        std::vector<GeoVecDoublearray> input{{ImageContext{}, constantLines, ResultType::AnalysisOK, Limit}};
        std::vector<Doublearray> mean{line};
        std::vector<Doublearray> variance{Doublearray(10, 0, 255)};
        std::vector<Doublearray> validity{Doublearray(10, 1.0, 255)};
        QTest::addRow("constantInput_mean") << 0 << 1 << 1 << input << mean;
        QTest::addRow("constantInput_variance") << 1 << 1 << 1 << input << variance;
    }
    {
        Doublearray line(10, 50, 255);
        std::vector<GeoVecDoublearray> input;
        std::vector<Doublearray> mean;
        std::vector<Doublearray> variance;
        std::vector<Doublearray> validity;
        ImageContext context;
        for (auto imageNumber = 0; imageNumber < 3; imageNumber++)
        {
            context.setImageNumber(imageNumber);
            input.emplace_back(context, VecDoublearray(1, line), ResultType::AnalysisOK, Limit);
            mean.push_back(line);
            variance.push_back(Doublearray(10, 0, 0));
            validity.push_back(Doublearray(10, 1.0, 255));
        }
        QTest::addRow("singleInput_mean") << 0 << 1 << 1 << input << mean;
        QTest::addRow("singleInput_variance") << 1 << 1 << 1 << input << variance;
        QTest::addRow("singleInput_validity") << 2 << 1 << 1 << input << validity;
        // now process a line where one point has bad rank
        {
            auto lineWithBadRank = line;
            lineWithBadRank.getRank()[0] = 0;
            context.setImageNumber(3);
            input.emplace_back(context, VecDoublearray(1, lineWithBadRank), ResultType::AnalysisOK, Limit);
            auto meanLine = lineWithBadRank;
            meanLine.getData()[0] = 0.0;
            mean.push_back(meanLine);
            variance.push_back(Doublearray(10, 0, 0));
            auto validityLine = Doublearray(10, 1.0, 255);
            validityLine.getData()[0] = 0.0;
            validity.push_back(validityLine);
        }
        QTest::addRow("singleInputWithBadRank_mean") << 0 << 1 << 1 << input << mean;
        QTest::addRow("singleInputWithBadRank_variance") << 1 << 1 << 1 << input << variance;
        QTest::addRow("singleInputWithBadRank_validity") << 2 << 1 << 1 << input << validity;

        // try a window, now the mean it's againg just the line , but we see the effect of the single bad rank point on the validity
        mean[3] = line;
        for (int imageNumber = 1; imageNumber <= 3; imageNumber++)
        {
            // from the second image the variance is valid (but zero because the input is constant)
            variance[imageNumber].getRank().assign(10, 255);
        }
        validity[3].getData()[0] = 3.0 / 4.0;
        QTest::addRow("singleInputWithBadRank_mean5") << 0 << 4 << 1 << input << mean;
        QTest::addRow("singleInputWithBadRank_variance5") << 1 << 4 << 1 << input << variance;
        QTest::addRow("singleInputWithBadRank_validity5") << 2 << 4 << 1 << input << validity;
    }
    {
        int parameter_numImages = 3;
        const std::vector<double> valueTemporalSequence{0.0, 1.5, -4.5, 3.0, 4.5};
        const std::vector<double> meanTemporalSequence{0.0, 0.75, -1.0, 0.0, 1.0};
        const std::vector<double> validityTemporalSequence(5, 1.0);
        const std::vector<double> varianceTemporalSequence{
            0.0,
            (0.75 * 0.75 + 0.75 * 0.75) / 1.0,
            (1 * 1 + 2.5 * 2.5 + 3.5 * 3.5) / 2.0,
            (1.5 * 1.5 + 4.5 * 4.5 + 3 * 3) / 2.0,
            (5.5 * 5.5 + 2 * 2 + 3.5 * 3.5) / 2.0};

        {
            std::vector<GeoVecDoublearray> input;
            std::vector<Doublearray> mean;
            std::vector<Doublearray> variance;
            std::vector<Doublearray> validity;
            ImageContext context;
            for (auto imageNumber = 0; imageNumber < 5; imageNumber++)
            {
                context.setImageNumber(imageNumber);
                input.emplace_back(context, VecDoublearray(1, Doublearray(1, valueTemporalSequence[imageNumber], 255)), ResultType::AnalysisOK, Limit);
                mean.push_back(Doublearray(1, meanTemporalSequence[imageNumber], 255));
                variance.push_back(Doublearray(1, varianceTemporalSequence[imageNumber], imageNumber > 0 ? 255 : 0));
                validity.push_back(Doublearray(1, validityTemporalSequence[imageNumber], 255));
            }
            QTest::addRow("meanFromWindow") << 0 << parameter_numImages << 1 << input << mean;
            QTest::addRow("varianceFromWindow") << 1 << parameter_numImages << 1 << input << variance;
            QTest::addRow("validityFromWindow") << 2 << parameter_numImages << 1 << input << validity;

            //verify that the computations doesn't update when we move the profile line slider (image 3 comes twice)
            input.insert(input.begin() + 3, *(input.begin() + 2));
            mean.insert(mean.begin() + 3, *(mean.begin() + 2));
            variance.insert(variance.begin() + 3, *(variance.begin() + 2));
            validity.insert(validity.begin() + 3, *(validity.begin() + 2));
            QTest::addRow("meanFromWindowWithRepetition") << 0 << parameter_numImages << 1 << input << mean;
            QTest::addRow("varianceFromWindowWithRepetition") << 1 << parameter_numImages << 1 << input << variance;
            QTest::addRow("validityFromWindowWithRepetition") << 2 << parameter_numImages << 1 << input << validity;
        }
        {
            int numLines = 3; //the same as the window in the previous test case, to reuse the calculation
            int parameter_numImages = 1;
            std::vector<GeoVecDoublearray> input;
            std::vector<Doublearray> mean;
            std::vector<Doublearray> variance;
            std::vector<Doublearray> validity;
            ImageContext context;
            for (auto imageNumber = 0; imageNumber < 5 - numLines + 1; imageNumber++)
            {
                context.setImageNumber(imageNumber);
                VecDoublearray inputLines;
                for (int lineN = 0; lineN < numLines; lineN++)
                {
                    inputLines.push_back(Doublearray(1, valueTemporalSequence[imageNumber + lineN], 255));
                }
                input.emplace_back(context, inputLines, ResultType::AnalysisOK, Limit);
                mean.push_back(Doublearray(1, meanTemporalSequence[imageNumber + numLines - 1], 255));
                variance.push_back(Doublearray(1, varianceTemporalSequence[imageNumber + numLines - 1], 255));
                validity.push_back(Doublearray(1, validityTemporalSequence[imageNumber + numLines - 1], 255));
            }
            QTest::addRow("meanFromBlock") << 0 << parameter_numImages << 1 << input << mean;
            QTest::addRow("varianceFromBlock") << 1 << parameter_numImages << 1 << input << variance;
            QTest::addRow("validityFromBlock") << 2 << parameter_numImages << 1 << input << validity;
        }
        {
            //let's have an empty block in image 2
            int numLines = 3; //the same as the window in the previous test case, to reuse the calculation
            int parameter_numImages = 1;
            std::vector<GeoVecDoublearray> input;
            std::vector<Doublearray> mean;
            std::vector<Doublearray> variance;
            std::vector<Doublearray> validity;
            ImageContext context;
            int valueCounter = 0;
            for (auto imageNumber = 0; imageNumber < 6 - numLines + 1; imageNumber++)
            {
                context.setImageNumber(imageNumber);
                if (imageNumber == 2)
                {
                    input.emplace_back(context, VecDoublearray{}, ResultType::AnalysisOK, NotPresent);
                    mean.push_back(Doublearray(1, 0.0, 0));
                    variance.push_back(Doublearray(1, 0.0, 0));
                    validity.push_back(Doublearray(1, 0.0, 0));
                }
                else
                {
                    VecDoublearray inputLines;
                    for (int lineN = 0; lineN < numLines; lineN++)
                    {
                        inputLines.push_back(Doublearray(1, valueTemporalSequence[valueCounter + lineN], 255));
                    }
                    input.emplace_back(context, inputLines, ResultType::AnalysisOK, Limit);
                    mean.push_back(Doublearray(1, meanTemporalSequence[valueCounter + numLines - 1], 255));
                    variance.push_back(Doublearray(1, varianceTemporalSequence[valueCounter + numLines - 1], 255));
                    validity.push_back(Doublearray(1, validityTemporalSequence[valueCounter + numLines - 1], 255));
                    valueCounter++;
                }
            }
            QTest::addRow("meanFromBlock_1") << 0 << parameter_numImages << 1 << input << mean;
            QTest::addRow("varianceFromBlock_1") << 1 << parameter_numImages << 1 << input << variance;
            QTest::addRow("validityFromBlock_1") << 2 << parameter_numImages << 1 << input << validity;
        }
        //test start image: pass-through until start image, the first unplausible values don't effect the filtering
        {
            std::vector<GeoVecDoublearray> input;
            {
                int parameter_startImage = 5;
                std::vector<Doublearray> mean;
                std::vector<Doublearray> variance;
                std::vector<Doublearray> validity;
                ImageContext context;
                int imageNumber = 0;
                for (; imageNumber < 4; imageNumber++)
                {
                    context.setImageNumber(imageNumber);
                    input.emplace_back(context, VecDoublearray(1, Doublearray(1, 1000, 255)), ResultType::AnalysisOK, Limit);
                    mean.push_back(Doublearray(1, 1000, 255));
                    variance.push_back(Doublearray(1, 1000, 255));
                    validity.push_back(Doublearray(1, 1000, 255));
                }
                for (unsigned int i = 0; i < valueTemporalSequence.size(); i++)
                {
                    context.setImageNumber(imageNumber);
                    input.emplace_back(context, VecDoublearray(1, Doublearray(1, valueTemporalSequence[i], 255)), ResultType::AnalysisOK, Limit);
                    mean.push_back(Doublearray(1, meanTemporalSequence[i], 255));
                    variance.push_back(Doublearray(1, varianceTemporalSequence[i], i > 0 ? 255 : 0));
                    validity.push_back(Doublearray(1, validityTemporalSequence[i], 255));
                    imageNumber++;
                }

                QTest::addRow("mean_firstBad") << 0 << parameter_numImages << parameter_startImage << input << mean;
                QTest::addRow("variance_firstBad") << 1 << parameter_numImages << parameter_startImage << input << variance;
                QTest::addRow("validity_firstBad") << 2 << parameter_numImages << parameter_startImage << input << validity;
            }
            //just passthrough: start image=0
            {
                int parameter_startImage = 0;
                std::vector<Doublearray> mean;
                std::vector<Doublearray> variance;
                std::vector<Doublearray> validity;

                for (auto inputLine : input)
                {
                    auto firstValue = inputLine.ref().front().getData().front();
                    mean.push_back(Doublearray(1, firstValue, 255));
                    variance.push_back(Doublearray(1, firstValue, 255));
                    validity.push_back(Doublearray(1, firstValue, 255));
                }

                QTest::addRow("mean_passthrough") << 0 << parameter_numImages << parameter_startImage << input << mean;
                QTest::addRow("variance_passthrough") << 1 << parameter_numImages << parameter_startImage << input << variance;
                QTest::addRow("validity_passthrough") << 2 << parameter_numImages << parameter_startImage << input << validity;
            }
        }


    }
    {
        //test start image: pass-through until start image
        int parameter_numImages = 3;
        const std::vector<double> valueTemporalSequence{0.0, 1.5, -4.5, 3.0, 4.5};
        const std::vector<double> meanTemporalSequence{0.0, 0.75, -1.0, 0.0, 1.0};
        const std::vector<double> validityTemporalSequence(5, 1.0);
        const std::vector<double> varianceTemporalSequence{
            0.0,
            (0.75 * 0.75 + 0.75 * 0.75) / 1.0,
            (1 * 1 + 2.5 * 2.5 + 3.5 * 3.5) / 2.0,
            (1.5 * 1.5 + 4.5 * 4.5 + 3 * 3) / 2.0,
            (5.5 * 5.5 + 2 * 2 + 3.5 * 3.5) / 2.0};

        {
            std::vector<GeoVecDoublearray> input;
            std::vector<Doublearray> mean;
            std::vector<Doublearray> variance;
            std::vector<Doublearray> validity;
            ImageContext context;
            for (auto imageNumber = 0; imageNumber < 5; imageNumber++)
            {
                context.setImageNumber(imageNumber);
                input.emplace_back(context, VecDoublearray(1, Doublearray(1, valueTemporalSequence[imageNumber], 255)), ResultType::AnalysisOK, Limit);
                mean.push_back(Doublearray(1, meanTemporalSequence[imageNumber], 255));
                variance.push_back(Doublearray(1, varianceTemporalSequence[imageNumber], imageNumber > 0 ? 255 : 0));
                validity.push_back(Doublearray(1, validityTemporalSequence[imageNumber], 255));
            }
            QTest::addRow("meanFromWindow") << 0 << parameter_numImages << 1 << input << mean;
            QTest::addRow("varianceFromWindow") << 1 << parameter_numImages << 1 << input << variance;
            QTest::addRow("validityFromWindow") << 2 << parameter_numImages << 1 << input << validity;

            //verify that the computations doesn't update when we move the profile line slider (image 3 comes twice)
            input.insert(input.begin() + 3, *(input.begin() + 2));
            mean.insert(mean.begin() + 3, *(mean.begin() + 2));
            variance.insert(variance.begin() + 3, *(variance.begin() + 2));
            validity.insert(validity.begin() + 3, *(validity.begin() + 2));
            QTest::addRow("meanFromWindowWithRepetition") << 0 << parameter_numImages << 1 << input << mean;
            QTest::addRow("varianceFromWindowWithRepetition") << 1 << parameter_numImages << 1 << input << variance;
            QTest::addRow("validityFromWindowWithRepetition") << 2 << parameter_numImages << 1 << input << validity;
        }
    }
}

void LineTemporalFilterTest::testProceed()
{
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas{new precitec::image::OverlayCanvas};
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<GeoVecDoublearray> inputPipeProfiles{&sourceFilter, "Profiles"};

    precitec::filter::LineTemporalFilter filter;
    filter.setCanvas(pcanvas.get());
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<GeoVecDoublearray>*>(filter.findPipe("Result"));
    QVERIFY(outPipe);
    QVERIFY(filter.connectPipe(&inputPipeProfiles, 1));
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipe, 1));

    QFETCH(int, parameter_operation);
    QFETCH(int, parameter_numImages);
    QFETCH(int, parameter_startImage);
    filter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, parameter_operation);
    filter.getParameters().update(std::string("NumImages"), fliplib::Parameter::TYPE_int, parameter_numImages);
    filter.getParameters().update(std::string("StartImage"), fliplib::Parameter::TYPE_int, parameter_startImage);
    filter.setParameter();

    QFETCH(std::vector<GeoVecDoublearray>, input);
    QFETCH(std::vector<Doublearray>, expectedResult);
    auto numImages = input.size();
    for (auto counter = 0u; counter < numImages; counter++)
    {
        QVERIFY(!dummyFilter.isProceedCalled());
        inputPipeProfiles.signal(input[counter]);
        QVERIFY(dummyFilter.isProceedCalled());
        dummyFilter.resetProceedCalled();

        auto& geoResult = outPipe->read(counter);
        auto& result = geoResult.ref();

        QCOMPARE(result.size(), 1);
        if (!result.front().empty())
        {
            // compare also the first point, because the comparisn message is more readable with numbers
            //qDebug() << "Entry " << counter << " Image Number " << input[counter].context().imageNumber();
            QCOMPARE(result.front().getData()[0], expectedResult[counter].getData()[0]);
            QCOMPARE(result.front().getRank()[0], expectedResult[counter].getRank()[0]);
        }

        QCOMPARE(result.front().getData(), expectedResult[counter].getData());
        QCOMPARE(result.front().getRank(), expectedResult[counter].getRank());
        QCOMPARE(geoResult.rank(), 1.0);
    }
}

QTEST_GUILESS_MAIN(LineTemporalFilterTest)

#include "testLineTemporalFilter.moc"
