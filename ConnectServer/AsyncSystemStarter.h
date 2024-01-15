/*
 * AsyncSystemStarter.h
 *
 *  Created on: 19.09.2012
 *      Author: admin
 */

#ifndef ASYNCSYSTEMSTARTER_H_
#define ASYNCSYSTEMSTARTER_H_

#include "Poco/Runnable.h"
#include <stdlib.h>

#include <vector>
#include <memory>
#include <mutex>

namespace Precitec {
	namespace Service {
		namespace Discovery {

			class AsyncSystemStarter: public Poco::Runnable {
			public:
				AsyncSystemStarter();
				virtual ~AsyncSystemStarter();

                void setConfigFileName(const std::string &name)
                {
                    m_configFileName = name;
                }

                void setShuttingDown();

			protected:
				/**
				 * Defines the information for starting a weldmaster application
				 * during startup phase.
				 */
				struct Process {
					/**
					 * The absolute path to the (GUI) terminal application
					 * for running the process in.
					 */
					std::string terminalPath;
					/**
					 * The name of the application to start.
					 * If @p weldMasterApplication is @c true, it is considered
					 * be an application in the $WM_BASE_DIR. In case @p batch is
					 * @c true, it is looked for in "${WM_BASE_DIR}/batch/, in case
					 * @p batch is @c false it is looked for in "${WM_BASE_DIR}/bin/.
					 *
					 * Otherwise it needs to be either an absolute path to a binary
					 * or name resolvable through $PATH.
					 */
					std::string processName;
					/**
					 * List of arguments to pass to the (GUI) terminal application.
					 */
					std::vector<std::string> terminalArguments;
					/**
					 * List of arguments to pass to the application to start.
					 */
					std::vector<std::string> arguments;
					/**
					 * If @c true, processName will be started with the "system" call.
					 * If @c false, processName will be started as a child process of
					 * a (GUI) terminal.
					 */
					bool batch;
					/**
					 * Whether the processName is part of the WM_BASE_DIR distribution
					 * or a system application.
					 */
					bool weldMasterApplication;
                    /**
                     * Whether the weldmasterApplication should be started in a terminal application
                     **/
                    bool useTerminal;
                    /**
                     * Whether this Process is enabled. A not enabled process won't be started
                     **/
                    bool enabled;
                    /**
                     * Whether this Process should be automatically restarted in case of a crash
                     **/
                    bool autoRestart;
                    /**
                     * Path to give to LD_PRELOAD. LD_PRELOAD is set for weldMasterApplication if not empty.
                     **/
                    std::string ldPreload;
				};

				/**
				 * Processes the startup configuration and returns a list of Processes to start.
				 */
				std::vector<Process> loadConfiguration();

				/**
				 * Start the given @p process.
				 */
				void startProcess(const Process &process);

				/**
				 * Starts the @p process as a weldmaster process.
				 * Passes @p pipePath as additional argument argument.
                 * In case @p restarted is set to @c true, the environment variable WM_RESTARTED is exported
				 */
				void startWeldmasterProcess(const Process &process, const std::string &pipePath, bool restarted = false);

				/**
				 * Creates a named pipe at @p pipePath and opens it.
				 *
				 * @returns a file descriptor to the pipe or @c -1 on error.
				 */

				int createPipe(const std::string &pipePath);

				/**
				 * Starts the given @p process as a batch through "system" call.
				 */
				void startBatch(const Process &process);

				/**
				 * Waits for a Process to be started by reading the
				 * @p readFileDescriptor.
				 *
				 * This blocks the thread till the started Process wrote into the
				 * pipe or till a timeout is reached.
				 */
				void waitForProcessStarted(int readFileDescriptor, std::string processName, int timeOut = 60000);

				/**
				 * @returns the arguments to start a @p process with the named @p pipePath
				 *
				 * Caller needs to delete[] the elements of the vector!
				 */
				std::vector<char*> createProcessArguments(const Process &process, const std::string &pipePath);

			private:
				void run();
                
                std::vector<std::unique_ptr<Poco::Runnable>> m_waitPidThreads;
                std::string m_configFileName;
                std::mutex m_shutDownMutex;
                bool m_shuttingDown = false;
			};

		}

	}

}

#endif /* ASYNCSYSTEMSTARTER_H_ */
