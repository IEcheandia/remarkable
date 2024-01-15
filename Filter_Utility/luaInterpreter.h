/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			FH
*	@date			2023
*	@brief			Lua scripting in editor
*/

#ifndef LuaInterpreter_H_
#define LuaInterpreter_H_

#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/SynchronePipe.h>
#include <filter/armStates.h>

#include <common/frame.h>
#include "lua/luaCanvas.h"
#include "lua/luaApi.h"

namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {

/**
 * Filter with lua scripting
 */
class FILTER_API LuaInterpreter : public fliplib::TransformFilter
{
public:
	LuaInterpreter();

	static const std::string m_rootFunction;
	static const std::string m_processFunction;
	static const std::string m_luaName;
	static const std::string m_oFilterName;
	static const std::string m_pPipeOutScalarSName;
	static const std::string m_pPipeOutScalarRName;
	static const std::string m_pPipeOutLineRName;
	static const std::string m_pPipeOutLineSName;
	static const std::string m_pPipeOutImageRName;
	static const std::string m_pPipeOutImageSName;
	virtual void setParameter() override;
	virtual void arm(const fliplib::ArmStateBase& p_rArmstate) override;
	virtual void dispose() override;

protected:
	virtual bool subscribe(fliplib::BasePipe& pipe, int group) override;
	virtual void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent ) override;
	virtual void paint() override;

private:
    const fliplib::SynchronePipe<GeoDoublearray>	*m_pPipeInScalarA;		//< in pins
    const fliplib::SynchronePipe<GeoDoublearray>	*m_pPipeInScalarB;
    const fliplib::SynchronePipe<GeoVecDoublearray>	*m_pPipeInLineA;
    const fliplib::SynchronePipe<GeoVecDoublearray>	*m_pPipeInLineB;
	const fliplib::SynchronePipe<ImageFrame>		*m_pPipeInImageA;
	const fliplib::SynchronePipe<ImageFrame>		*m_pPipeInImageB;

	fliplib::SynchronePipe<GeoDoublearray>			m_pPipeOutScalarR;		//< out pins
	fliplib::SynchronePipe<GeoDoublearray>			m_pPipeOutScalarS;
	fliplib::SynchronePipe<GeoVecDoublearray>		m_pPipeOutLineR;
	fliplib::SynchronePipe<GeoVecDoublearray>		m_pPipeOutLineS;
	fliplib::SynchronePipe<ImageFrame>				m_pPipeOutImageR;
	fliplib::SynchronePipe<ImageFrame>				m_pPipeOutImageS;

	std::string				m_luaText;		/// Lua code
	bool					m_resetOutput;	/// Whether the output values should be reset between frames

	std::string 			m_luaBinary;	/// Compiled lua binary chunk, compiled from m_luaText on setParameter
	bool					m_hasError;		/// Whether Lua code has encountered an error (prevents log spam), gets reset on arm
    interface::SmpTrafo 	m_trafo;		/// Default context trafo used for painting
	lua::LuaPaintCallQueue	m_paintQueue;	/// Queued up paint calls for the next paint operation
	lua_State* 				m_luaState;		/// Currently running LUA state
	bool					m_hasProcess;	/// Whether LUA defines a process function we're executing
	int						m_runCount;		/// Whether this has already run at least once (used for code analysis)
};

} // namespace filter
} // namespace precitec

#endif /*LuaInterpreter_H_*/
