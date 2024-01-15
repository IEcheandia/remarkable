/** @defgroup AnalyzerInterface AnalyzerInterface
 */
/** @defgroup Analyzer_Interface Analyzer_Interface
 */

#if !defined(ANALYZER_INTERFACE_API)
	#if defined(_WIN32)
		#if defined(ANALYZER_INTERFACE_EXPORTS)
			#define ANALYZER_INTERFACE_API __declspec(dllexport)
		#else
			#define ANALYZER_INTERFACE_API __declspec(dllimport)
		#endif
	#endif
#endif

#if !defined(ANALYZER_INTERFACE_API)
	#define ANALYZER_INTERFACE_API
#endif

