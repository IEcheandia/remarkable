/** @defgroup Analyzer Analyzer
 */

#if !defined(MOD_ANALYZER_API)
	#if defined(_WIN32)
		#if defined(MOD_ANALYZER_EXPORTS)
			#define MOD_ANALYZER_API __declspec(dllexport)
		#else
			#define MOD_ANALYZER_API __declspec(dllimport)
		#endif
	#endif
#endif // !defined(MOD_ANALYZER_API)

#if !defined(MOD_ANALYZER_API)
	#define MOD_ANALYZER_API
#endif
