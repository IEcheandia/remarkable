
#include "luaInterpreter.h"

#include "filter/algoStl.h"
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>

#include "lua/luaConstants.h"
#include "lua/luaContext.h"
#include "lua/luaData.h"
#include "lua/luaScalar.h"
#include "lua/luaLine.h"
#include "lua/luaImage.h"

#include "image/image.h"
#include "common/bitmap.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LuaInterpreter::m_rootFunction =			std::string("root");
const std::string LuaInterpreter::m_processFunction =		std::string("process");
const std::string LuaInterpreter::m_luaName =				std::string("program");
const std::string LuaInterpreter::m_oFilterName =			std::string("LuaInterpreter");
const std::string LuaInterpreter::m_pPipeOutScalarRName =	std::string("sR");
const std::string LuaInterpreter::m_pPipeOutScalarSName =	std::string("sS");
const std::string LuaInterpreter::m_pPipeOutLineRName =		std::string("lnR");
const std::string LuaInterpreter::m_pPipeOutLineSName =		std::string("lnS");
const std::string LuaInterpreter::m_pPipeOutImageRName =	std::string("imgR");
const std::string LuaInterpreter::m_pPipeOutImageSName =	std::string("imgS");


LuaInterpreter::LuaInterpreter() :
	TransformFilter		( LuaInterpreter::m_oFilterName, Poco::UUID{"07efee2d-bfaa-4f1d-b8d3-f5c2697ce8c3"} ),
	m_pPipeInScalarA	( nullptr ),
	m_pPipeInScalarB	( nullptr ),
	m_pPipeInLineA		( nullptr ),
	m_pPipeInLineB		( nullptr ),
	m_pPipeInImageA		( nullptr ),
	m_pPipeInImageB		( nullptr ),
	m_pPipeOutScalarR	( this, LuaInterpreter::m_pPipeOutScalarRName ),
	m_pPipeOutScalarS	( this, LuaInterpreter::m_pPipeOutScalarSName ),
	m_pPipeOutLineR		( this, LuaInterpreter::m_pPipeOutLineRName ),
	m_pPipeOutLineS		( this, LuaInterpreter::m_pPipeOutLineSName ),
	m_pPipeOutImageR	( this, LuaInterpreter::m_pPipeOutImageRName ),
	m_pPipeOutImageS	( this, LuaInterpreter::m_pPipeOutImageSName ),
	m_luaText			(""),
	m_resetOutput		(true),
	m_luaBinary			(""),
	m_hasError			(false),
	m_luaState			(nullptr),
	m_hasProcess		(false),
	m_runCount			(0)
{
	parameters_.add("LuaText", fliplib::Parameter::TYPE_text, m_luaText);
	parameters_.add("ResetOutput", fliplib::Parameter::TYPE_bool, m_resetOutput);

	setInPipeConnectors({
		{Poco::UUID("a5d23b6c-27a1-4b3a-9949-2686c36dfb8d"), m_pPipeInScalarA, "sA", 1, "input_scalar_a", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("d74842d1-17b6-4b7f-bb3b-82de0446b3bd"), m_pPipeInScalarB, "sB", 1, "input_scalar_b", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("0b31a1ff-8ce0-4a1b-9df9-933993305b54"), m_pPipeInLineA, "lnA", 1, "input_line_a", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("5aa265ef-7b4a-4910-aca8-82beefea609f"), m_pPipeInLineB, "lnB", 1, "input_line_b", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("3dd85b82-51be-4132-83ea-e8eeba220b87"), m_pPipeInImageA, "imgA", 1, "input_image_a", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("7710f0fe-a769-4e73-9dae-eff27fe4d126"), m_pPipeInImageB, "imgB", 1, "input_image_b", fliplib::PipeConnector::ConnectionType::Optional}
		});
	setOutPipeConnectors({
		{Poco::UUID("355cd2fb-c944-492a-b021-7c937eb5c406"), &m_pPipeOutScalarR, m_pPipeOutScalarRName, 0, "", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("f2d8519e-ad49-4803-8005-bcf3b08b983f"), &m_pPipeOutScalarS, m_pPipeOutScalarSName, 0, "", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("a71a1ced-24b8-4efd-8e24-8542592a91f5"), &m_pPipeOutLineR, m_pPipeOutLineRName, 0, "", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("7a3b3f24-071b-44ea-8d60-294dd350332f"), &m_pPipeOutLineS, m_pPipeOutLineSName, 0, "", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("7b578d2b-1927-455e-9bdc-ed09f0e57f50"), &m_pPipeOutImageR, m_pPipeOutImageRName, 0, "", fliplib::PipeConnector::ConnectionType::Optional},
		{Poco::UUID("d113e1e8-a936-46b0-b85c-dc07af47570b"), &m_pPipeOutImageS, m_pPipeOutImageSName, 0, "", fliplib::PipeConnector::ConnectionType::Optional}
		});
	setVariantID(Poco::UUID("c575e417-1835-49ae-ab14-5dfd9465e9d2"));
}

void LuaInterpreter::setParameter()
{
	TransformFilter::setParameter();

	m_luaText = parameters_.getParameter("LuaText").convert<std::string>();
	m_resetOutput = parameters_.getParameter("ResetOutput").convert<bool>();
	m_luaBinary = lua::lua_compile(m_luaText);
}

bool LuaInterpreter::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "input_scalar_a" )
	{
		m_pPipeInScalarA = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
	}
	else if ( p_rPipe.tag() == "input_scalar_b" )
	{
		m_pPipeInScalarB = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
	}
	else if ( p_rPipe.tag() == "input_line_a" )
	{
		m_pPipeInLineA = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&p_rPipe);
	}
	else if ( p_rPipe.tag() == "input_line_b" )
	{
		m_pPipeInLineB = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&p_rPipe);
	}
   	else if ( p_rPipe.tag() == "input_image_a" )
	{
		m_pPipeInImageA = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&p_rPipe);
	}
	else if ( p_rPipe.tag() == "input_image_b" )
	{
		m_pPipeInImageB = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}


void LuaInterpreter::arm(const fliplib::ArmStateBase& p_rArmstate)
{
	// Make sure we only report errors once per run
	m_hasError = false;

	if(m_luaBinary.size() > 0)
	{
		m_luaState = lua::lua_create();

		lua::LuaIntegerConstants::registerMetatable(m_luaState);
		lua::LuaNumberConstants::registerMetatable(m_luaState);
		lua::LuaCanvas::registerMetatable(m_luaState);
		lua::LuaContext::registerMetatable(m_luaState);
		lua::LuaScalar::registerMetatable(m_luaState);
		lua::LuaLine::registerMetatable(m_luaState);
		lua::LuaImage::registerMetatable(m_luaState);

		lua::lua_load(m_luaState, m_luaBinary, m_luaName);

		// It is possible to call the same state multiple times.
		// pcall pops the main function from the stack, but since we set it to a global variable we can reset it.
		lua_setglobal(m_luaState, m_rootFunction.c_str());

		// Set global constants

		lua::LuaIntegerConstants::registerConstants(m_luaState, "_Results", {
			{"analysisOk", 	interface::ResultType::AnalysisOK},
			{"luaError", 	interface::ResultType::LuaError}
		});

		lua::LuaNumberConstants::registerConstants(m_luaState, "_Scores", {
			{"minimum", 	interface::Minimum},
			{"marginal", 	interface::Marginal},
			{"bad", 		interface::Bad},
			{"doubtful", 	interface::Doubtful},
			{"notGood", 	interface::NotGood},
			{"ok", 			interface::Ok},
			{"good", 		interface::Good},
			{"perfect", 	interface::Perfect},
			{"limit", 		interface::Limit}
		});

		lua::LuaIntegerConstants::registerConstants(m_luaState, "_Ranks", {
			{"min", 	filter::eRankMin},
			{"max", 	filter::eRankMax}
		});

		lua::lua_registerMathConstants(m_luaState);

		m_hasError = false;
		m_hasProcess = false;
		m_runCount = 0;
	}

	return TransformFilter::arm( p_rArmstate );
}


void LuaInterpreter::dispose()
{
	if(m_luaState)
	{
		lua_close(m_luaState);
		m_luaState = nullptr;
	}
	return TransformFilter::dispose();
}

void LuaInterpreter::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
	// Default context is set to the latest available input
	const interface::ImageContext* defaultContext = nullptr;

	// Read Scalar input pipes if available
	lua::LuaScalar::Type inScalarA;
	if (m_pPipeInScalarA != nullptr)
	{
		inScalarA = m_pPipeInScalarA->read(m_oCounter);
		defaultContext = &inScalarA.context();
	}

	lua::LuaScalar::Type inScalarB;
	if (m_pPipeInScalarB != nullptr)
	{
		inScalarB = m_pPipeInScalarB->read(m_oCounter);
		defaultContext = &inScalarB.context();
	}

	// Read Line input pipes if available
	lua::LuaLine::Type inLineA;
	if (m_pPipeInLineA != nullptr)
	{
		inLineA = m_pPipeInLineA->read(m_oCounter);
		defaultContext = &inLineA.context();
	}

	lua::LuaLine::Type inLineB;
	if (m_pPipeInLineB != nullptr)
	{
		inLineB = m_pPipeInLineB->read(m_oCounter);
		defaultContext = &inLineB.context();
	}

	// Read Image pipe if available
	lua::LuaImage::Type inImageA;
	if (m_pPipeInImageA != nullptr)
	{
		inImageA = m_pPipeInImageA->read(m_oCounter);
		defaultContext = &inImageA.context();
	}

	lua::LuaImage::Type inImageB;
	if (m_pPipeInImageB != nullptr)
	{
		inImageB = m_pPipeInImageB->read(m_oCounter);
		defaultContext = &inImageB.context();
	}

	// If proceedGroup was called we can be certain that at least one pin with valid context is connected
	poco_assert_dbg(defaultContext != nullptr);

	// Default trafo is used for painting
	m_trafo = defaultContext->trafo();

	// Make sure all output values are initially valid
	lua::LuaScalar::Type outScalarR = lua::lua_makeEmptyScalar(*defaultContext);
	lua::LuaScalar::Type outScalarS = lua::lua_makeEmptyScalar(*defaultContext);
	lua::LuaLine::Type outLineR = lua::lua_makeEmptyLine(*defaultContext);
	lua::LuaLine::Type outLineS = lua::lua_makeEmptyLine(*defaultContext);
	lua::LuaImage::Type outImageR = lua::lua_makeEmptyImage(*defaultContext);
	lua::LuaImage::Type outImageS = lua::lua_makeEmptyImage(*defaultContext);

	// Stop execution if there was an error in our LUA code (or compilation failed)
	if (!m_hasError && m_luaBinary.size() > 0)
	{
		// Frame count starts at 1
		lua_pushinteger(m_luaState, m_runCount + 1);
		lua_setglobal(m_luaState, "_N");

		lua::lua_createContext(m_luaState, *defaultContext);
		lua_setglobal(m_luaState, "_C");

		// Reset drawcalls before each execution
		m_paintQueue.clear();
		lua::lua_createCanvas(m_luaState, m_paintQueue);
		lua_setglobal(m_luaState, "_Draw");

		// Set inputs to pipe values
		lua::lua_createObject<lua::LuaScalar>(m_luaState, inScalarA);
		lua_setglobal(m_luaState, "sA");

		lua::lua_createObject<lua::LuaScalar>(m_luaState, inScalarB);
		lua_setglobal(m_luaState, "sB");

		lua::lua_createObject<lua::LuaLine>(m_luaState, inLineA);
		lua_setglobal(m_luaState, "lnA");

		lua::lua_createObject<lua::LuaLine>(m_luaState, inLineB);
		lua_setglobal(m_luaState, "lnB");

		lua::lua_createObject<lua::LuaImage>(m_luaState, inImageA);
		lua_setglobal(m_luaState, "imgA");

		lua::lua_createObject<lua::LuaImage>(m_luaState, inImageB);
		lua_setglobal(m_luaState, "imgB");

		if (m_resetOutput || m_runCount == 0)
		{
			// Initialize outputs to empty
			lua::lua_createObject<lua::LuaScalar>(m_luaState, outScalarR);
			lua_setglobal(m_luaState, "sR");

			lua::lua_createObject<lua::LuaScalar>(m_luaState, outScalarS);
			lua_setglobal(m_luaState, "sS");

			lua::lua_createObject<lua::LuaLine>(m_luaState, outLineR);
			lua_setglobal(m_luaState, "lnR");

			lua::lua_createObject<lua::LuaLine>(m_luaState, outLineS);
			lua_setglobal(m_luaState, "lnS");

			lua::lua_createObject<lua::LuaImage>(m_luaState, outImageR);
			lua_setglobal(m_luaState, "imgR");

			lua::lua_createObject<lua::LuaImage>(m_luaState, outImageS);
			lua_setglobal(m_luaState, "imgS");
		}

		if (m_runCount == 0)
		{
			// If the user defines a process function we want to run the whole script only once to initialise all global variables,
			// then we start calling the process function each frame.
			lua_getglobal(m_luaState, m_rootFunction.c_str());
			lua::lua_run(m_luaState, m_hasError);

			// In case there is a process function we run again
			const int mainType = lua_getglobal(m_luaState, m_processFunction.c_str());
			if(mainType == LUA_TFUNCTION)
			{
				m_hasProcess = true;
				lua::lua_run(m_luaState, m_hasError);
			}
		}
		else
		{
			// We assume here that process won't get undefined during LUA execution
			lua_getglobal(m_luaState, m_hasProcess ? m_processFunction.c_str() : m_rootFunction.c_str());
			lua::lua_run(m_luaState, m_hasError);
		}
		m_runCount++;

		// Retreive scalar results
		lua::lua_retreiveScalar(m_luaState, "sR", outScalarR, m_hasError);
		lua::lua_retreiveScalar(m_luaState, "sS", outScalarS, m_hasError);

		// Retreive line results
		lua::lua_retreiveLine(m_luaState, "lnR", outLineR, m_hasError);
		lua::lua_retreiveLine(m_luaState, "lnS", outLineS, m_hasError);

		// Retreive image results
		lua::lua_retreiveImage(m_luaState, "imgR", outImageR, m_hasError);
		lua::lua_retreiveImage(m_luaState, "imgS", outImageS, m_hasError);
	}

	// Make sure all outputs report errors if LUA execution failed
	if (m_hasError)
	{
		outScalarR.setAnalysisResult(interface::LuaError);
		outScalarS.setAnalysisResult(interface::LuaError);
		outLineR.setAnalysisResult(interface::LuaError);
		outLineS.setAnalysisResult(interface::LuaError);
		outImageR.setAnalysisResult(interface::LuaError);
		outImageS.setAnalysisResult(interface::LuaError);
	}

	// Output
	preSignalAction();
	m_pPipeOutScalarR.signal(outScalarR);
	m_pPipeOutScalarS.signal(outScalarS);
	m_pPipeOutLineR.signal(outLineR);
	m_pPipeOutLineS.signal(outLineS);
	m_pPipeOutImageR.signal(outImageR);
	m_pPipeOutImageS.signal(outImageS);
}

void LuaInterpreter::paint()
{
	if (m_oVerbosity == eNone || m_trafo.isNull() || m_paintQueue.empty() || !hasCanvas())  // filter should not paint anything on verbosity eNone
	{
		return;
	}

	const interface::Trafo &trafo (*m_trafo);

	image::OverlayCanvas &overlayCanvas (canvas<image::OverlayCanvas>(m_oCounter));
	for (const std::unique_ptr<lua::LuaPaintCall>& paintCall : m_paintQueue)
	{
		paintCall->paint(overlayCanvas, trafo);
	}
}

} // namespace filter
} // namespace precitec

