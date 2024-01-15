/** @defgroup Calibration Calibration
 */

#if !defined(MOD_CALIBRATION_API)
	#if defined(_WIN32)
		#if defined(MOD_CALIBRATION_EXPORTS)
			#define MOD_CALIBRATION_API __declspec(dllexport)
		#else
			#define MOD_CALIBRATION_API __declspec(dllimport)
		#endif
	#endif
#endif // !defined(MOD_CALIBRATION_API)

#if !defined(MOD_CALIBRATION_API)
	#define MOD_CALIBRATION_API
#endif
