/**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Simon Hilsenbeck (HS)
 *  @date		2011
 * 	@brief 		WIN32 DLL EXPORT MACRO
 */

/** @defgroup VideoRecorder VideoRecorder
 */

#if !defined(MOD_VIDEORECORDER_API)
	#if defined(_WIN32)
		#if defined(MOD_VIDEORECORDER_EXPORTS)
			#define MOD_VIDEORECORDER_API __declspec(dllexport)
		#else
			#define MOD_VIDEORECORDER_API __declspec(dllimport)
		#endif
	#endif
#endif // !defined(MOD_VIDEORECORDER_API)

#if !defined(MOD_VIDEORECORDER_API)
	#define MOD_VIDEORECORDER_API
#endif
