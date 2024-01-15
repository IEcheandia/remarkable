#include <QtTest/QtTest>

#include "../valueDisplay.h"

#include "fliplib/NullSourceFilter.h"
#include "fliplib/BaseFilter.h"
#include "overlay/overlayCanvas.h"

class TestValueDisplay : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
};

struct DummyInput
{
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pos_x_pipe;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pos_y_pipe;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_value_pipe;

    precitec::interface::GeoDoublearray m_pos_x;
    precitec::interface::GeoDoublearray m_pos_y;
    precitec::interface::GeoDoublearray m_value;

    DummyInput()
    : m_pos_x_pipe{ & m_sourceFilter, "PositionX"}
    , m_pos_y_pipe{ & m_sourceFilter, "PositionY"}
    , m_value_pipe{&m_sourceFilter, "Value"}
    {
        m_pos_x_pipe.setTag("pos_x");
        m_pos_y_pipe.setTag("pos_y");
        m_value_pipe.setTag("value");
    }

    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group = 1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&( m_pos_x_pipe ), group);
        ok &= pFilter->connectPipe(&( m_pos_y_pipe ), group);
        ok &= pFilter->connectPipe(&(m_value_pipe), group);
        return ok;
    }

    void fillData( int imageNumber, double pos_x, double pos_y, double value, precitec::geo2d::Point trafoOffset)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        SmpTrafo oTrafo{new LinearTrafo(trafoOffset)};
        ImageContext context (ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        m_pos_x = GeoDoublearray{context,
            precitec::geo2d::Doublearray{1, pos_x, precitec::filter::eRankMax},
            AnalysisOK, 1.0 };
        m_pos_y = GeoDoublearray{context,
            precitec::geo2d::Doublearray{1, pos_y, precitec::filter::eRankMax},
            AnalysisOK, 1.0 };
        m_value = GeoDoublearray{context,
            precitec::geo2d::Doublearray{1, value, precitec::filter::eRankMax},
            AnalysisOK, 1.0 };

    }

    void signal()
    {
        m_pos_x_pipe.signal(m_pos_x);
        m_pos_y_pipe.signal(m_pos_y);
        m_value_pipe.signal(m_value);
    }
};

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

void TestValueDisplay::testCtor()
{
    precitec::filter::ValueDisplay filter;
    QCOMPARE(filter.name(), std::string("ValueDisplay"));
    QVERIFY(!filter.findPipe("NotAValidPipe"));

    QVERIFY(filter.getParameters().exists("TextPattern"));
    QCOMPARE(filter.getParameters().findParameter("TextPattern").getType(), fliplib::Parameter::TYPE_string);
    QCOMPARE(filter.getParameters().findParameter("TextPattern").getValue().convert<std::string>(), "");
    QVERIFY(filter.getParameters().exists("Red"));
    QCOMPARE(filter.getParameters().findParameter("Red").getType(), fliplib::Parameter::TYPE_uint);
    QVERIFY(filter.getParameters().exists("Green"));
    QCOMPARE(filter.getParameters().findParameter("Green").getType(), fliplib::Parameter::TYPE_uint);
    QVERIFY(filter.getParameters().exists("Blue"));
    QCOMPARE(filter.getParameters().findParameter("Blue").getType(), fliplib::Parameter::TYPE_uint);

}

//TODO: implement test for proceed function

QTEST_GUILESS_MAIN(TestValueDisplay)
#include "testValueDisplay.moc"
