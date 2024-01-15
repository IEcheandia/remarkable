#include <QtTest/QtTest>

#include "../luaInterpreter.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>

namespace {
	using Ranks_t = std::vector<int>;
	using Values_t = std::vector<double>;
	using ScalarData_t = std::tuple<Values_t, Ranks_t>;
	using LineData_t = std::vector<ScalarData_t>;
	template <typename T> using Scored_t = std::tuple<precitec::interface::ResultType, double, T>;
	template <typename T> using Result_t = std::tuple<precitec::interface::ResultType, T>;
	using ImageData_t = std::vector<std::vector<byte>>;
	#define INF std::numeric_limits<double>::infinity()
	#define OK precitec::interface::AnalysisOK
	#define BAD precitec::interface::LuaError
	#define ERR(n) ((precitec::interface::ResultType)n)
}

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(Scored_t<ScalarData_t>)
Q_DECLARE_METATYPE(Scored_t<LineData_t>)
Q_DECLARE_METATYPE(Result_t<ImageData_t>)

class TestLuaInterpreter : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void testCtor();
	void testProceed_data();
	void testProceed();
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

void TestLuaInterpreter::testCtor()
{
	precitec::filter::LuaInterpreter filter;

	QCOMPARE(filter.name(), std::string("LuaInterpreter"));
	QVERIFY(filter.findPipe("sR") != nullptr);
	QVERIFY(filter.findPipe("sS") != nullptr);
	QVERIFY(filter.findPipe("lnR") != nullptr);
	QVERIFY(filter.findPipe("lnS") != nullptr);
	QVERIFY(filter.findPipe("imgR") != nullptr);
	QVERIFY(filter.findPipe("imgS") != nullptr);
	QVERIFY(filter.getParameters().exists(std::string("LuaText")));
	QVERIFY(filter.getParameters().exists(std::string("ResetOutput")));
}

void TestLuaInterpreter::testProceed_data()
{

	QTest::addColumn< int > ("iterations");
	QTest::addColumn< bool > ("param_resetOutput");
	QTest::addColumn< std::string > ("param_luaText");
	QTest::addColumn< Scored_t<ScalarData_t> > ("input_scalar_a");
	QTest::addColumn< Scored_t<ScalarData_t> > ("input_scalar_b");
	QTest::addColumn< Scored_t<LineData_t> > ("input_line_a");
	QTest::addColumn< Scored_t<LineData_t> > ("input_line_b");
	QTest::addColumn< Result_t<ImageData_t> > ("input_image_a");
	QTest::addColumn< Result_t<ImageData_t> > ("input_image_b");
	QTest::addColumn< Scored_t<ScalarData_t> > ("output_scalar_r");
	QTest::addColumn< Scored_t<ScalarData_t> > ("output_scalar_s");
	QTest::addColumn< Scored_t<LineData_t> > ("output_line_r");
	QTest::addColumn< Scored_t<LineData_t> > ("output_line_s");
	QTest::addColumn< Result_t<ImageData_t> > ("output_image_r");
	QTest::addColumn< Result_t<ImageData_t> > ("output_image_s");

	QTest::newRow("Test Add/Subtract") // Add and subtract all values, minimize rank
			<< int (1)
			<< bool (true)
			<< std::string { R"(;
sR = sA + sB
sS = sA - sB
lnR = lnA + lnB
lnS = lnA - lnB
imgR = imgA + imgB
imgS = imgA - imgB
)" }
			<< Scored_t<ScalarData_t>	(OK, 1.0, ScalarData_t (Values_t { 2.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 1.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 2 }))
			<< Scored_t<LineData_t> 	(OK, 1.0, LineData_t { ScalarData_t(Values_t { 4.0, 5.0, 6.0 }, Ranks_t { 1, 2, 3 }) })
			<< Scored_t<LineData_t>		(OK, 1.0, LineData_t { ScalarData_t(Values_t { 1.0, 2.0, 3.0 }, Ranks_t { 3, 2, 1 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4, 5 }, { 6, 7 }, { 8, 9 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 1, 2 }, { 3, 4 }, { 5, 6 }}) }
			<< Scored_t<ScalarData_t>	(OK, 1.0, ScalarData_t (Values_t { 3.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 1.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 1 }))
			<< Scored_t<LineData_t> 	(OK, 1.0, LineData_t { ScalarData_t(Values_t { 5.0, 7.0, 9.0 }, Ranks_t { 1, 2, 1 }) })
			<< Scored_t<LineData_t>		(OK, 1.0, LineData_t { ScalarData_t(Values_t { 3.0, 3.0, 3.0 }, Ranks_t { 1, 2, 1 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 5, 7 }, { 9, 11 }, { 13, 15 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 3, 3 }, { 3, 3 }, { 3, 3 }}) };

	QTest::newRow("Test Mul/Div/Unm") // Multiply and divide all values, minimize rank
			<< int (1)
			<< bool (true)
			<< std::string { R"(;
sR = sA * sB
sS = sA / -sB
lnR = lnA * -lnB
lnS = lnA / lnB
imgR = imgA * imgB
imgS = imgA / imgB
)" }
			<< Scored_t<ScalarData_t>	(OK, 1.0, ScalarData_t (Values_t { 2.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 1.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 2 }))
			<< Scored_t<LineData_t> 	(OK, 1.0, LineData_t { ScalarData_t(Values_t { 4.2, 6.0, 1.0 }, Ranks_t { 1, 2, 3 }) })
			<< Scored_t<LineData_t>		(OK, 1.0, LineData_t { ScalarData_t(Values_t { 0.5, 2.0, 0.0 }, Ranks_t { 3, 2, 1 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4, 6 }, { 1, 2 }, { 5, 0 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 1, 2 }, { 0, 4 }, { 5, 6 }}) }
			<< Scored_t<ScalarData_t>	(OK, 1.0, ScalarData_t (Values_t { 2.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 1.0, ScalarData_t (Values_t { -2.0 }, Ranks_t { 1 }))
			<< Scored_t<LineData_t> 	(OK, 1.0, LineData_t { ScalarData_t(Values_t { -2.1, -12.0, -0.0 }, Ranks_t { 1, 2, 1 }) })
			<< Scored_t<LineData_t>		(OK, 1.0, LineData_t { ScalarData_t(Values_t { 8.4, 3.0, INF }, Ranks_t { 1, 2, 1 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4, 12 }, { 0, 8 }, { 25, 0 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4, 3 }, { 0, 0 }, { 1, 0 }}) };

	QTest::newRow("Test Mod/Pow") // Modulo and power all values, minimize rank
			<< int (1)
			<< bool (true)
			<< std::string { R"(;
sR = sA % sB
sS = sA ^ sB
lnR = lnA % lnB
lnS = lnA ^ lnB
imgR = imgA % imgB
imgS = imgA ^ imgB
)" }
			<< Scored_t<ScalarData_t>	(OK, 1.0, ScalarData_t (Values_t { 2.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 1.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 2 }))
			<< Scored_t<LineData_t> 	(OK, 1.0, LineData_t { ScalarData_t(Values_t { 1.44, 7.0, 1.0 }, Ranks_t { 1, 2, 3 }) })
			<< Scored_t<LineData_t>		(OK, 1.0, LineData_t { ScalarData_t(Values_t { 0.5, 2.0, 0.0 }, Ranks_t { 3, 2, 1 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4, 7 }, { 1, 2 }, { 5, 0 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 1, 2 }, { 0, 2 }, { 2, 6 }}) }
			<< Scored_t<ScalarData_t>	(OK, 1.0, ScalarData_t (Values_t { 0.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 1.0, ScalarData_t (Values_t { 2.0 }, Ranks_t { 1 }))
			<< Scored_t<LineData_t> 	(OK, 1.0, LineData_t { ScalarData_t(Values_t { 0.44, 1.0, NAN }, Ranks_t { 1, 2, 1 }) })
			<< Scored_t<LineData_t>		(OK, 1.0, LineData_t { ScalarData_t(Values_t { 1.2, 49.0, 1.0 }, Ranks_t { 1, 2, 1 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 0, 1 }, { 0, 0 }, { 1, 0 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4, 49 }, { 1, 4 }, { 25, 0 }}) };

	QTest::newRow("Test Get/Set Count/Size/Data/Rank/Score") // Get and set variable data
			<< int (1)
			<< bool (true)
			<< std::string { R"(;
-- Scalars
sR = Scalar.new(_C, _Scores.ok, #sA, 0.0, _Ranks.min)
sR[1] = sA[1] -- Index set
sR:score(sB:score())

sS = Scalar.new(sB:context(), _Scores.ok, #sB, 0.0, _Ranks.min)
sS[1]:value( sB[2]:value() ) -- Value set
sS[1]:rank( sB[2]:rank() )
sS[2]:value( sB[1]:value() )
sS[2]:rank( sB[1]:rank() )
sS:score( sA:score() )

-- Lines
lnR = Line.new(_C, _Scores.ok, #lnA, #lnA[1], 0.0, _Ranks.min)
lnR[1][1] = lnA[1][1] -- Index set
lnR[2] = lnA[2]
lnR:score(lnB:score())

lnS = Line.new(lnB:context(), _Scores.ok, #lnB, #lnB[2], 0.5, 5)
lnS[1][1]:value( lnB[2][1]:value() ) -- Value set
lnS[1][1]:rank( lnB[2][1]:rank() )
lnS[2][1]:value( lnB[1][1]:value() )
lnS[2][1]:rank( lnB[1][1]:rank() )
lnS:score( lnA:score() )

-- Images
imgR = Image.new(_C, 2, 1, 0)
imgR:set(1, 1, imgA:get(2, 1))
imgR:set(2, 1, imgA:get(1, 1))

imgS = Image.new(imgB:context(), 1, 2, 0)
imgS:set(1, 1, imgB:get(1, 2))
imgS:set(1, 2, imgB:get(1, 1))
)" }
			<< Scored_t<ScalarData_t>	(OK, 0.1, ScalarData_t (Values_t { 1.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 0.2, ScalarData_t (Values_t { 2.0, 3.0 }, Ranks_t { 2, 3 }))
			<< Scored_t<LineData_t> 	(OK, 0.3, LineData_t { ScalarData_t(Values_t { 1.0 }, Ranks_t { 1 }), ScalarData_t(Values_t { 5.0 }, Ranks_t { 5 }) })
			<< Scored_t<LineData_t>		(OK, 0.4, LineData_t { ScalarData_t(Values_t { 2.0 }, Ranks_t { 2 }), ScalarData_t(Values_t { 3.0, 4.0 }, Ranks_t { 3, 4 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 1, 2 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 3 }, { 4 }}) }
			<< Scored_t<ScalarData_t>	(OK, 0.2, ScalarData_t (Values_t { 1.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 0.1, ScalarData_t (Values_t { 3.0, 2.0 }, Ranks_t { 3, 2 }))
			<< Scored_t<LineData_t> 	(OK, 0.4, LineData_t { ScalarData_t(Values_t { 1.0 }, Ranks_t { 1 }), ScalarData_t(Values_t { 5.0 }, Ranks_t { 5 }) })
			<< Scored_t<LineData_t>		(OK, 0.3, LineData_t { ScalarData_t(Values_t { 3.0, 0.5 }, Ranks_t { 3, 5 }), ScalarData_t(Values_t { 2.0, 0.5 }, Ranks_t { 2, 5 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 2, 1 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4 }, { 3 }}) };

	QTest::newRow("Test Scale/Shift/Result") // Test results and functions between types
			<< int (1)
			<< bool (true)
			<< std::string { R"(;
sR = sA:scale(2):shift(-1)
sS = sB:scale(2):shift(-1)
sS:result(_Results.luaError)

lnR = lnA:scale(sB):scale(2):shift(sB):shift(-1)
lnS = lnB:scale(sB):scale(2):shift(sB):shift(-1)
lnS:result(lnA:result())

imgR = imgA:scale(sB):scale(2):shift(sB):shift(-1)
imgS = imgB:scale(sB):scale(2):shift(sB):shift(-1)
imgS:result(24)
)" }
			<< Scored_t<ScalarData_t>	(OK, 0.1, ScalarData_t (Values_t { 1 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 0.2, ScalarData_t (Values_t { 2, 3 }, Ranks_t { 2, 3 }))
			<< Scored_t<LineData_t> 	(ERR(13), 0.3, LineData_t { ScalarData_t(Values_t { 1.1, 2.2, 3.3 }, Ranks_t { 1, 2, 3 }) })
			<< Scored_t<LineData_t>		(OK, 0.4, LineData_t { ScalarData_t(Values_t { 4.4, 5.5, 6.6 }, Ranks_t { 4, 5, 6 }), ScalarData_t(Values_t { 7.7, 8.8, 9.9 }, Ranks_t { 7, 8, 9 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 1 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 2, 3 }, { 4, 5 }}) }
			<< Scored_t<ScalarData_t>	(OK, 0.1, ScalarData_t (Values_t { 1 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(BAD, 0.2, ScalarData_t (Values_t { 3, 5 }, Ranks_t { 2, 3 }))
			<< Scored_t<LineData_t> 	(OK, 0.2, LineData_t { ScalarData_t(Values_t { 5.4, 9.8, 14.2 }, Ranks_t { 1, 2, 2 }) })
			<< Scored_t<LineData_t>		(ERR(13), 0.2, LineData_t { ScalarData_t(Values_t { 18.6, 23.0, 27.4 }, Ranks_t { 2, 2, 2 }), ScalarData_t(Values_t { 48.2, 54.8, 61.4 }, Ranks_t { 3, 3, 3 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 5 }}) }
			<< Result_t<ImageData_t>	{ERR(24), ImageData_t({{ 9, 13 }, { 17, 21 }}) };

	// TODO: Add support for testing overlay shapes
	QTest::newRow("Test Draw/Default/Copy values") // Make sure drawing operations don´t crash the filter and whether all results return the right default values if not set
			<< int (1)
			<< bool (true)
			<< std::string { R"(;
_Draw:point(_Layers.position, 5, 16, _Colors.red)
_Draw:line(_Layers.line, sA, sB, 5, 10 , _Colors.green)
_Draw:cross(_Layers.position, sA[1]:value(), sA, 8, _Colors.blue)
_Draw:rect(_Layers.contour, 6, 2, sB, sB, 0xFFFFFF)
_Draw:text(_Layers.text, 0, 0, "My Name Is Methos", 16, 0x0000FF)
_Draw:circle(_Layers.contour, 16, 16, sB, 256)

sS = Scalar.new(sB) --Inherit bad input result
lnS = Line.new(lnB) --Inherit bad input result
imgS = Image.new(imgB) --Inherit bad input result
)" }
			<< Scored_t<ScalarData_t>	(BAD, 1.0, ScalarData_t (Values_t { 2.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(BAD, 1.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 2 }))
			<< Scored_t<LineData_t> 	(BAD, 1.0, LineData_t { ScalarData_t(Values_t { 4.0, 5.0, 6.0 }, Ranks_t { 1, 2, 3 }) })
			<< Scored_t<LineData_t>		(ERR(2), 1.0, LineData_t { ScalarData_t(Values_t { 1.0, 2.0, 3.0 }, Ranks_t { 3, 2, 1 }) })
			<< Result_t<ImageData_t>	{BAD, ImageData_t({{ 4, 5 }, { 6, 7 }, { 8, 9 }})}
			<< Result_t<ImageData_t>	{ERR(14), ImageData_t({{ 0, 1 }, { 2, 3 }})}
			<< Scored_t<ScalarData_t>	(OK, 0.0, ScalarData_t (Values_t { 0.0 }, Ranks_t { 0 }))
			<< Scored_t<ScalarData_t> 	(BAD, 1.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 2 }))
			<< Scored_t<LineData_t> 	(OK, 0.0, LineData_t { ScalarData_t(Values_t { }, Ranks_t { }) })
			<< Scored_t<LineData_t>		(ERR(2), 1.0, LineData_t { ScalarData_t(Values_t { 1.0, 2.0, 3.0 }, Ranks_t { 3, 2, 1 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({})}
			<< Result_t<ImageData_t>	{ERR(14), ImageData_t({{ 0, 1 }, { 2, 3 }})};

	// TODO: Add support for testing print output
	QTest::newRow("Test Print/Fail/Predicates") // Test predicates and whether outputs properly react to runtime failure
			<< int (1)
			<< bool (true)
			<< std::string { R"(;
print(_Math.pi, _Math.e)
print(sA, sB)
print(lnA, lnB)
print(imgA, imgB)

if sA < sB then
	sR = 1 -- Scalars support number primitve output
else
	sR = 2
end

if lnA < lnB then
	sS = 3
else
	sS = 4
end

fail("Failed", "successfully!")
)" }
			<< Scored_t<ScalarData_t>	(OK, 1.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 1 }))
			<< Scored_t<ScalarData_t> 	(OK, 1.0, ScalarData_t (Values_t { 2.0, 3.0, 4.0, 5.0, 6.0 }, Ranks_t { 2, 3, 4, 5, 6 }))
			<< Scored_t<LineData_t> 	(OK, 1.0, LineData_t { ScalarData_t(Values_t { 4.0, 5.0, 6.0 }, Ranks_t { 1, 2, 3 }) })
			<< Scored_t<LineData_t>		(OK, 1.0, LineData_t { ScalarData_t(Values_t { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 }, Ranks_t { 1, 2, 3, 4, 5, 6 }), ScalarData_t(Values_t { 7.0, 8.0, 9.0, 10.0, 11.0, 12.0 }, Ranks_t { 7, 8, 9, 10, 11, 12 }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 4, 5 }, { 6, 7 }, { 8, 9 }}) }
			<< Result_t<ImageData_t>	{OK, ImageData_t({{ 1 }}) }
			<< Scored_t<ScalarData_t>	(BAD, 0.0, ScalarData_t (Values_t { 1.0 }, Ranks_t { 0 }))
			<< Scored_t<ScalarData_t> 	(BAD, 0.0, ScalarData_t (Values_t { 4.0 }, Ranks_t { 0 }))
			<< Scored_t<LineData_t> 	(BAD, 0.0, LineData_t { ScalarData_t(Values_t { }, Ranks_t { }) })
			<< Scored_t<LineData_t>		(BAD, 0.0, LineData_t { ScalarData_t(Values_t { }, Ranks_t { }) })
			<< Result_t<ImageData_t>	{BAD, ImageData_t({})}
			<< Result_t<ImageData_t>	{BAD, ImageData_t({})};

QTest::newRow("Test Multiframe") // Test behaviour over multiple frames
			<< int (4)
			<< bool (false)
			<< std::string { R"(;
x = 0.0;

function process()
	x = x + 1.1
	sR[1]:value(x)
	sR[1]:rank( sR[1]:rank() + _N) -- sR shouldn't get reset each frame because ResetOutput is false
	print("frame", _N)
end
)" }
			<< Scored_t<ScalarData_t>	(OK, 0.0, ScalarData_t (Values_t { 0.0 }, Ranks_t { 0 }))
			<< Scored_t<ScalarData_t> 	(OK, 0.0, ScalarData_t (Values_t { 0.0 }, Ranks_t { 0 }))
			<< Scored_t<LineData_t> 	(OK, 0.0, LineData_t { ScalarData_t(Values_t { }, Ranks_t { }) })
			<< Scored_t<LineData_t>		(OK, 0.0, LineData_t { ScalarData_t(Values_t { }, Ranks_t { }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({})}
			<< Result_t<ImageData_t>	{OK, ImageData_t({})}
			<< Scored_t<ScalarData_t>	(OK, 0.0, ScalarData_t (Values_t { 4.4 }, Ranks_t { 10 }))
			<< Scored_t<ScalarData_t> 	(OK, 0.0, ScalarData_t (Values_t { 0.0 }, Ranks_t { 0 }))
			<< Scored_t<LineData_t> 	(OK, 0.0, LineData_t { ScalarData_t(Values_t { }, Ranks_t { }) })
			<< Scored_t<LineData_t>		(OK, 0.0, LineData_t { ScalarData_t(Values_t { }, Ranks_t { }) })
			<< Result_t<ImageData_t>	{OK, ImageData_t({})}
			<< Result_t<ImageData_t>	{OK, ImageData_t({})};
}

void TestLuaInterpreter::testProceed()
{
	precitec::filter::LuaInterpreter filter;

	// In-Pipe "input_scalar_a"
	fliplib::NullSourceFilter sourceFilterQualA;
	fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeScalarA{ &sourceFilterQualA, "sA"};
	pipeScalarA.setTag("input_scalar_a");
	QVERIFY(filter.connectPipe(&pipeScalarA, 1));

	// In-Pipe "input_scalar_b"
	fliplib::NullSourceFilter sourceFilterQualB;
	fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeScalarB{ &sourceFilterQualB, "sB"};
	pipeScalarB.setTag("input_scalar_b");
	QVERIFY(filter.connectPipe(&pipeScalarB, 1));

	// In-Pipe "input_line_a"
	fliplib::NullSourceFilter sourceFilterLineA;
	fliplib::SynchronePipe< precitec::interface::GeoVecDoublearray >  pipeLineA{ &sourceFilterLineA, "lnA"};
	pipeLineA.setTag("input_line_a");
	QVERIFY(filter.connectPipe(&pipeLineA, 1));

	// In-Pipe "input_line_b"
	fliplib::NullSourceFilter sourceFilterLineB;
	fliplib::SynchronePipe< precitec::interface::GeoVecDoublearray >  pipeLineB{ &sourceFilterLineB, "lnB"};
	pipeLineB.setTag("input_line_b");
	QVERIFY(filter.connectPipe(&pipeLineB, 1));

	// In-Pipe "input_line_a"
	fliplib::NullSourceFilter sourceFilterImageA;
	fliplib::SynchronePipe< precitec::interface::ImageFrame >  pipeImageA{ &sourceFilterImageA, "imgA"};
	pipeImageA.setTag("input_image_a");
	QVERIFY(filter.connectPipe(&pipeImageA, 1));

	// In-Pipe "input_line_b"
	fliplib::NullSourceFilter sourceFilterImageB;
	fliplib::SynchronePipe< precitec::interface::ImageFrame >  pipeImageB{ &sourceFilterImageB, "imgB"};
	pipeImageB.setTag("input_image_b");
	QVERIFY(filter.connectPipe(&pipeImageB, 1));

	// Create Out-Pipes and connect to conditional filter
	// --------------------------------------------------

	DummyFilter filterOutData;
	QVERIFY(filterOutData.connectPipe(filter.findPipe("sR"), 1));
	QVERIFY(filterOutData.connectPipe(filter.findPipe("sS"), 1));
	QVERIFY(filterOutData.connectPipe(filter.findPipe("lnR"), 1));
	QVERIFY(filterOutData.connectPipe(filter.findPipe("lnS"), 1));
	QVERIFY(filterOutData.connectPipe(filter.findPipe("imgR"), 1));
	QVERIFY(filterOutData.connectPipe(filter.findPipe("imgS"), 1));

	// dummy data

	int imageNumber  =   0;
	int position     = 300;

	precitec::interface::ImageContext context;
	context.setImageNumber(imageNumber);
	context.setPosition(position);

	// parse test data
	QFETCH(int								, iterations);
	QFETCH(std::string						, param_luaText);
	QFETCH(bool								, param_resetOutput);
	QFETCH(Scored_t<ScalarData_t>			, input_scalar_a);
	QFETCH(Scored_t<ScalarData_t>			, input_scalar_b);
	QFETCH(Scored_t<LineData_t> 			, input_line_a);
	QFETCH(Scored_t<LineData_t> 			, input_line_b);
	QFETCH(Result_t<ImageData_t> 			, input_image_a);
	QFETCH(Result_t<ImageData_t> 			, input_image_b);
	QFETCH(Scored_t<ScalarData_t>			, output_scalar_r);
	QFETCH(Scored_t<ScalarData_t>			, output_scalar_s);
	QFETCH(Scored_t<LineData_t> 			, output_line_r);
	QFETCH(Scored_t<LineData_t> 			, output_line_s);
	QFETCH(Result_t<ImageData_t> 			, output_image_r);
	QFETCH(Result_t<ImageData_t> 			, output_image_s);

    filter.getParameters().update(std::string("LuaText"), fliplib::Parameter::TYPE_text, param_luaText);
	filter.getParameters().update(std::string("ResetOutput"), fliplib::Parameter::TYPE_text, param_resetOutput);
    filter.setParameter();

	filter.arm(precitec::filter::ArmState::eSeamStart);

	// convert test params into appropriate data structure

	const precitec::interface::ResultType scalarResultA = std::get<0>(input_scalar_a);
	const double scalarScoreA = std::get<1>(input_scalar_a);
	const ScalarData_t& scalarDataA = std::get<2>(input_scalar_a);
	precitec::geo2d::Doublearray sA;
	sA.getData().insert(sA.getData().begin(), std::get<0>(scalarDataA).begin(), std::get<0>(scalarDataA).end());
	sA.getRank().insert(sA.getRank().begin(), std::get<1>(scalarDataA).begin(), std::get<1>(scalarDataA).end());

	const precitec::interface::ResultType scalarResultB = std::get<0>(input_scalar_b);
	const double scalarScoreB = std::get<1>(input_scalar_b);
	const ScalarData_t& scalarDataB= std::get<2>(input_scalar_b);
	precitec::geo2d::Doublearray sB;
	sB.getData().insert(sB.getData().begin(), std::get<0>(scalarDataB).begin(), std::get<0>(scalarDataB).end());
	sB.getRank().insert(sB.getRank().begin(), std::get<1>(scalarDataB).begin(), std::get<1>(scalarDataB).end());

	const precitec::interface::ResultType lineResultA = std::get<0>(input_line_a);
	const double lineScoreA = std::get<1>(input_line_a);
	const LineData_t& lineDataA = std::get<2>(input_line_a);
	precitec::geo2d::VecDoublearray lnA(lineDataA.size());
	for (size_t i = 0; i < lineDataA.size(); i++)
	{
		lnA[i].getData().insert(lnA[i].getData().begin(), std::get<0>(lineDataA[i]).begin(), std::get<0>(lineDataA[i]).end());
		lnA[i].getRank().insert(lnA[i].getRank().begin(), std::get<1>(lineDataA[i]).begin(), std::get<1>(lineDataA[i]).end());
	}

	const precitec::interface::ResultType lineResultB = std::get<0>(input_line_b);
	const double lineScoreB = std::get<1>(input_line_b);
	const LineData_t& lineDataB = std::get<2>(input_line_b);
	precitec::geo2d::VecDoublearray lnB(lineDataB.size());
	for (size_t i = 0; i < lineDataB.size(); i++)
	{
		lnB[i].getData().insert(lnB[i].getData().begin(), std::get<0>(lineDataB[i]).begin(), std::get<0>(lineDataB[i]).end());
		lnB[i].getRank().insert(lnB[i].getRank().begin(), std::get<1>(lineDataB[i]).begin(), std::get<1>(lineDataB[i]).end());
	}

	const precitec::interface::ResultType imageResultA = std::get<0>(input_image_a);
	const ImageData_t& imageDataA = std::get<1>(input_image_a);
	precitec::image::BImage imgA(precitec::geo2d::Size(imageDataA.size() > 0 ? imageDataA[0].size() : 0, imageDataA.size()));
	for (size_t y = 0; y < imageDataA.size(); y++)
	{
		for (size_t x = 0; x < imageDataA[y].size(); x++)
		{
			imgA.setValue(x, y, imageDataA[y][x]);
		}
	}

	const precitec::interface::ResultType imageResultB = std::get<0>(input_image_b);
	const ImageData_t& imageDataB = std::get<1>(input_image_b);
	precitec::image::BImage imgB(precitec::geo2d::Size(imageDataB.size() > 0 ? imageDataB[0].size() : 0, imageDataB.size()));
	for (size_t y = 0; y < imageDataB.size(); y++)
	{
		for (size_t x = 0; x < imageDataB[y].size(); x++)
		{
			imgB.setValue(x, y, imageDataB[y][x]);
		}
	}

	// This simulates multiple frames being processed with the same arguments
	for(int i = 0; i < iterations; i++)
	{
		// signal pipes
		pipeScalarA.signal(precitec::interface::GeoDoublearray(context, sA, scalarResultA, scalarScoreA));
		pipeScalarB.signal(precitec::interface::GeoDoublearray(context, sB, scalarResultB, scalarScoreB));
		pipeLineA.signal(precitec::interface::GeoVecDoublearray(context, lnA, lineResultA, lineScoreA));
		pipeLineB.signal(precitec::interface::GeoVecDoublearray(context, lnB, lineResultB, lineScoreB));
		pipeImageA.signal(precitec::interface::ImageFrame(context, imgA, imageResultA));
		pipeImageB.signal(precitec::interface::ImageFrame(context, imgB, imageResultB));

		// verify that the filter has run
		QVERIFY(filterOutData.isProceedCalled());

		filterOutData.resetProceedCalled();
	}

	filterOutData.dispose();

	// compare signaled data
	auto checkScalarOutput = [&](const char* pipe, const Scored_t<ScalarData_t>& desired){
		auto outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe(pipe));
		QVERIFY(outPipeData);

		const precitec::interface::GeoDoublearray& outScalar = outPipeData->read(context.imageNumber());
		QCOMPARE((int)outScalar.analysisResult(), (int)std::get<0>(desired));

		const double score = std::get<1>(desired);
		QCOMPARE(outScalar.rank(), score);

		const ScalarData_t& scalarData = std::get<2>(desired);

		QCOMPARE(outScalar.ref().size(), std::get<0>(scalarData).size());
		const size_t elements = std::min(outScalar.ref().size(), std::get<1>(scalarData).size());
		for (size_t i = 0; i < elements; i++) {
			QCOMPARE(std::get<precitec::filter::eData>(outScalar.ref()[i]), std::get<0>(scalarData)[i]);
			QCOMPARE(std::get<precitec::filter::eRank>(outScalar.ref()[i]), std::get<1>(scalarData)[i]);
		}
	};

	checkScalarOutput("sR", output_scalar_r);
	checkScalarOutput("sS", output_scalar_s);

	auto checkLineOutput = [&](const char* pipe, const Scored_t<LineData_t>& desired){
		auto outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecDoublearray>*>(filter.findPipe(pipe));
		QVERIFY(outPipeData);

		const precitec::interface::GeoVecDoublearray& outLine = outPipeData->read(context.imageNumber());
		QCOMPARE((int)outLine.analysisResult(), (int)std::get<0>(desired));

		const double score = std::get<1>(desired);
		QCOMPARE(outLine.rank(), score);

		const LineData_t& lineData = std::get<2>(desired);
		QCOMPARE(outLine.ref().size(), lineData.size());
		const size_t size = std::min(outLine.ref().size(), lineData.size());
		for (size_t i = 0; i < size; i++) {
			QCOMPARE(outLine.ref()[i].size(), std::get<0>(lineData[i]).size());
			const size_t elements = std::min(outLine.ref()[i].size(), std::get<0>(lineData[i]).size());
			for (size_t j = 0; j < elements; j++) {
				QCOMPARE(std::get<precitec::filter::eData>(outLine.ref()[i][j]), std::get<0>(lineData[i])[j]);
				QCOMPARE(std::get<precitec::filter::eRank>(outLine.ref()[i][j]), std::get<1>(lineData[i])[j]);
			}
		}
	};

	checkLineOutput("lnR", output_line_r);
	checkLineOutput("lnS", output_line_s);

	auto checkImageOutput = [&](const char* pipe, const Result_t<ImageData_t>& desired){
		auto outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(filter.findPipe(pipe));
		QVERIFY(outPipeData);

		const precitec::interface::ImageFrame& outImage = outPipeData->read(context.imageNumber());
		QCOMPARE((int)outImage.analysisResult(), (int)std::get<0>(desired));

		const ImageData_t& imageData = std::get<1>(desired);
		QCOMPARE(outImage.data().height(), (int)imageData.size() );
		const int height = std::min(outImage.data().height(), (int)imageData.size());
		for (int y = 0; y < height; y++)
		{
			QCOMPARE(outImage.data().width(), (int)imageData[y].size() );
			const int width = std::min(outImage.data().width(), (int)imageData[y].size());
			for (int x = 0; x < width; x++)
			{
				QCOMPARE(outImage.data().getValue(x, y), imageData[y][x]);
			}
		}
	};

	checkImageOutput("imgR", output_image_r);
	checkImageOutput("imgS", output_image_s);
}

QTEST_GUILESS_MAIN(TestLuaInterpreter)

#include "testLuaInterpreter.moc"
