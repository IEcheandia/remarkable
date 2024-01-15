/**
 * @defgroup Workflow Workflow
 *
 * @mainpage The WeldMaster inspection system.
 *
 * The WeldMaster is a high performance inspection system and consists of the following <a href="modules.html">modules</a>:
 *
 * @ref Logger - Responsible for collecting the log messages from the QNX processes and sending them to the GUI.
 *
 * @ref Framegrabber - Configures the camera and the frame-grabber, retrieves the images and sends them to the analyzer.
 *
 * @ref VideoRecorder - Stores image sequences on the hard disk and sends them to GUI player.
 */

#if !defined(MOD_WORKFLOW_API)
	#if defined(_WIN32)
		#if defined(MOD_WORKFLOW_EXPORTS)
			#define MOD_WORKFLOW_API __declspec(dllexport)
		#else
			#define MOD_WORKFLOW_API __declspec(dllimport)
		#endif
	#endif
#endif // !defined(MOD_WORKFLOW_API)

#if !defined(MOD_WORKFLOW_API)
	#define MOD_WORKFLOW_API
#endif
