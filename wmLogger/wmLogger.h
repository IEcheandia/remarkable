/**
* \file wmLogger.h
*
* \author Andreas Beschorner
*
* \date Created on: Dec 17, 2010
*
* \brief Implements classes for log messages ( wmLogMessage ), loggers ( baseclass wmLoggerBase ) and,  nested in the latter, a string formatter ( wmLogFormatter ).
*
* <b style="color:#cc0000">Dependencies: </b> wmMPSCQueue, wmAtomics (indirect), wmLogStreams
*
* <b style="color:#22bb00">Note:</b><br />
* <i>The idea behind the <b>tMessage</b>-enumeration is, to allow messages to be of several types by bitwise ORing.
* I.o.w: You can choose a message to be both of informational type and a debug notification at the same time by creating a message
* create(message, senderId, precitec::utils::msgInfo | precitec::utils::msgDebug)</i>.
*
*/
#ifndef __precitec__wmLogger_h__
#define __precitec__wmLogger_h__

// clib includes
#include <inttypes.h>				//< for standard Width int types (also called strong types)
#include <string>
#include <memory>
#include <sstream>
#include <ostream>
#include <iostream>
#include <iomanip> 					//< for std::setw(...)
#include <fstream> 					//< for ofstream*
#include <map>
#include <ctime>
// Poco includes
#include "Poco/Activity.h"
#include "Poco/Thread.h"
#include "Poco/Condition.h"
#include "Poco/RegularExpression.h"
#include "Poco/String.h"
#include "Poco/Timer.h"
// WM includes
#include <wmAtomics.h>
#include <sys/time.h>
#include <wmLogStreams.h>
#include <wmMPSCQueue.h>
#include <wmLogStreams.h>
#include <wmLogMessage.h>

namespace precitec {
	namespace utils {

		/// Type definition for the log queue.
		typedef wmMPSCQueue<wmLogMessage> wmLogQueue;

		/// File storage strategy enum. Indicates the interval the logger's target file will be overwritten. To be considered wisely!<br /><i>Default: fileHour</i>
		enum tFileStorageStrategy {
			_fileFiveSec = 1, _fileTenSec,_fileMinute, _fileQuarter, _fileHour, _fileHour12,
			_fileDay, _fileWeek, /*_fileMonth, _fileMonth6, _fileYear,*/ _fileMax
		};
		/// Default file handling strategy (replace logfile hourly).
		const tFileStorageStrategy _defaultFileStorageStrategy = _fileHour;

		/// Memory dump strategy. Indicates, at what rate the logger's internal consumer consumes messages.<br /><i>Default: dumpBalanced</i>
		enum tDumpStrategy {
			/* As the queue is not a ringbuffer, we can not check for a certain percentage of the rinbuffer to be filled but
			* instead translate the strategy into a number x, represented by the value of the dump stragety.
			* Whenever the queue contains greater or equal than x elements, the consumer can pull.<br />
			*/
			_dumpImmediate = 1, _dumpOften = 10, _dumpAgile=25, _dumpBalanced=50, _dumpLazy=75, _dumpTardy=500
		};
		/// Default dump strategy.
		const tDumpStrategy _defaultDumpStrategy = _dumpBalanced;

		/// Map <threadId, stringstream>. Allows usage of << stream operator amongst multiple threads by assigning each thread it's own stream.
		typedef std::map<long int, std::stringstream*> tStringstreamMap;


		// ******************************************************************************


		// ==============================================================================
		// ---------------------------- class wmLoggerBase ------------------------------
		/** \class wmLoggerBase
		*
		* \brief Abstract base class for the specialized template logger classes.
		*
		* To avoid multiple quasi identical implementations (specialized loggers have basically identical functionality),
		* the specializations will all be derived from wmLoggerBase. The general <i>int</i>-template variant is also derived
		* from this class but left abstract, so both the latter and this class cannot be instantiated.<br />
		*
		* A detailed introduction including examples as well as warnings concerning pathological problems due to incorrect handling are
		* given in the documentation of the unit test file doc, wmLoggerTest.cc.
		*
		* The wmLoggerBase class and its derivatives/ template specializations characterize and determine the behaviour of a logger.
		* The logging itself is performed on instantiations of the refLogger class, which guarantees thread and process safe logging and streaming.
		*
		*/
		class wmLoggerBase
		{
			friend class refLogger;

		public:
			class wmLogFormatter;

			/// Manipulators are redefined as enums to prevent potentially unneccessary code generation due to calling/ generating tab/endl functions.
			enum m_manipulator { tab = '\t', endl = '\n' };

			/// Explicit constructor(streamType, streamName). The name is used for instance by filestreams.
			explicit wmLoggerBase(tStream streamType, std::string streamName)
			{
				// streamName will be filestream name for both additional filestreams and original filestreams
				this->init(streamType, streamName);
			}

			/// Virtual destructor.
			virtual ~wmLoggerBase()
			{
				if (m_FileStream != NULL)
				{
					if (m_FileStream->is_open())
						m_FileStream->close();
					delete m_FileStream;
				}

				if (m_OntopFilestream != NULL)
				{
					if (m_OntopFilestream->is_open())
						m_OntopFilestream->close();
					delete m_OntopFilestream;
				}

				if (m_actCons != NULL)
				{
					// this should not happen!
					if (m_actCons->isRunning())
					{
						//	throw("ERR <wmLoggerBase::~wmLoggerBase>: Consumer thread still running!");
						m_actCons->stop();
						m_actCons->wait();
					}
					delete m_actCons;
				}

				if (m_actProd != NULL)
				{
					// this should not happen!
					if (m_actProd->isRunning())
					{
						//throw("ERR <wmLoggerBase::~wmLoggerBase>: Producer thread still running!");
						m_actProd->stop();
						m_actProd->wait();
					}
					delete m_actProd;
				}

				//    m_oTimerLogfile.stop();
				delete m_pCallback;
				if (m_wmStream != NULL)
				{
					delete m_wmStream;
				}
			}

			/// Virtual function. Returns a string identifying the logger by a meaningful name. Can not be changed/ chosen by programmer.
			virtual std::string identify()
			{
				return std::string("");
			}

			/// Set logger name. The name can be chosen by the programmer in contrast to the identifier (returned by the identify() method).
			inline void setName(std::string s)
			{
				m_Name.assign(s);
			}

			/// Returns logger name.
			inline std::string name() {
				return m_Name;
			}

			//-------------------------------------------------------------------------------------------------------------
			// Setters and getters...
			/// Sets the verbosity level [0, 9].
			inline void setVerbosity(unsigned int v)
			{
				m_Verbosity = v % 10;
			}

			/// Returns type of logger.
			inline tLogtype type()
			{
				return m_LoggerType;
			}

			/// Adds type of logger.
			inline void addLogtype(tLogtype p_oLogtype)
			{
				m_LoggerType = tLogtype( int( m_LoggerType ) | int( p_oLogtype ) );
			}


			/// Returns stream type
			inline tStream streamType()
			{
				return m_wmStream->type();
			}

			/// Returns logger verbosity level.
			inline unsigned int verbosity()
			{
				return m_Verbosity;
			}

			/// Returns dump strategy
			inline void setDumpStrategy(unsigned int ds)
			{
				// We do not check whether ds is a valid value; this allows some (though not typesafe) freedom.
				if (!m_Running)
					m_DumpThreshold = (int)ds;
			}

			/// Sets dump strategy
			inline int getDumpStrategy()
			{
				return (int)m_DumpThreshold;
			}

			/// Sets file storage interval. Logger must be stopped to perform this action, not active or paused.
			void setFilestreamInterval(tFileStorageStrategy p_oStrategy)
			{
				if (!m_Running)
				{
					if (p_oStrategy >= _fileMax)
					{
						p_oStrategy = (tFileStorageStrategy)(_fileMax - 1);
					}
					m_oStorageStrategy = p_oStrategy;
					unsigned long int ms = storageStrategyToMilliseconds();
					m_oTimerLogfile.setPeriodicInterval(ms);
					m_oTimerLogfile.setStartInterval(ms);
					// m_oTimerLogfile.restart(ms);
				}
				else
				{
					std::cerr << "WARNING: Logfile renew interval can only be changed when logger is stopped!\n";
				}
			}

			/// Returns file storage interval
			inline tFileStorageStrategy getFilestreamInterval()
			{
				return m_oStorageStrategy;
			}

			//-------------------------------------------------------------------------------------------------------------

			/// Enables/ Disables queue dumping when pausing logger.
			inline void setDumpOnPause(bool b)
			{
				m_DumpOnPause = b;
			}

			/// Enable further streaming (store release), forwarded to stream class.
			inline volInt32 safeStreamReleased()
			{
				return !( m_wmStream->safeStreamLocked() );
			}
			/// Return whether streaming is locked (load acquire), forwarded to stream class.
			inline volInt32 safeStreamLocked()
			{
				return m_wmStream->safeStreamLocked();
			}
			/// Return whether streaming is enabled/ unlocked.
			inline volInt32 streamReleased()
			{
				return !(m_wmStream->streamLocked());
			}
			/// Return whether streaming is disabled/ locked.
			inline volInt32 streamLocked()
			{
				return m_wmStream->streamLocked();
			}
			/// Lock/ disable streaming (store release).
			inline void safeLockStream()
			{
				m_wmStream->safeLockStream();
			}
			/// Unlock/ enable streaming (store release).
			inline void safeReleaseStream()
			{
				m_wmStream->safeReleaseStream();
			}

			/// Returns whether logger is running.
			inline bool isRunning()
			{
				return m_Running;
			}

			/// Starts logger. <br /><b>Throws </b> char* exception on error.
			int start()
			{	// returns -1 if logger is already running, 1 if an error occurs when starting the consumer thread, 0 if all is OK.
				if ( m_Running | m_Stopping )
					return -1;

				m_Running = true;

				try
				{
					m_actCons->start();
				}
				catch (...)
				{
					throw "ERR <wmLogger::start> : Cannot create logger consumer thread.";
					if (m_actCons != NULL)
						delete m_actCons;
					return 1;
				}
				try
				{
					m_actProd->start();
				}
				catch (...)
				{
					throw "ERR <wmLogger::start> : Cannot create logger producer thread.";
					if (m_actCons != NULL)
						delete m_actCons;
					return 2;
				}
				//m_actCons->wait(); // wait for thread to start

				// Write logger start message and dump it right away
				m_oTimerLogfile.start( *m_pCallback );
				/*
				wmLogMessage *startMsg = wmLogMessage::create(std::string(" -- Logger started -- \n"),
				std::string("<wmBaseLogger::start>"), m_LoggerType, m_Verbosity);
				std::string theTime("");
				generateTimestamp(theTime);
				startMsg->setTimestamp(theTime);
				m_oLocalFormatter.setFormat("%d %n - %s: %m");
				commitMessage(startMsg, &m_oLocalFormatter, m_OntopFilestream);
				delete startMsg;
				*/
				return 0;	// all OK
			}

			///	Stops logger. <br /><b>Throws </b> char* exception on error.
			int stop()
			{	// returns -1 if logger is not running, 1 if an error occurs when stopping the consumer thread, 0 if all is OK.
				if ( !m_Running || m_Stopping )
					return -1;

				safeLockStream();                                                      // avoid further logging
				m_oTimerLogfile.stop();
				m_Running = false;
				m_Stopping = true;

				m_DumpThreshold = loadAcquireInt32(m_Queue.size());   // dump strategy: dump what is left in queue
				safeReleaseStream();                                  // necessary so we do not get stuck within a possible concurrent log
				pushConditionWait();                                  // signal cond var about new dump strategy

				while (loadAcquireInt32(&m_Dumping) == wmBaseStream::m_One)
					Poco::Thread::sleep(5);

				try
				{
					m_actProd->stop();
				}
				catch (...)
				{
					safeReleaseStream();
					if (m_actProd != NULL)
						delete m_actProd;  // todo: try stopping consumer thread
					throw "ERR <wmLogger::stop> : Cannot stop logger producer thread. Enforcing deletion.";
					return 2;
				}
				m_actProd->wait();

				try
				{
					m_actCons->stop();
				}
				catch (...)
				{
					safeReleaseStream();
					if (m_actCons != NULL)
						delete m_actCons;
					throw "ERR <wmLogger::stop> : Cannot stop logger consumer thread. Enforcing deletion.";
					return 1;
				}
				m_actCons->wait(); // wait for thread to stop

				while (m_Queue.size() > 0)
				{
					wmLogMessage *msg = m_Queue.popSpinlock();                           // pop message from queue
					if (msg != NULL)
					{
						commitMessage(msg, &m_Formatter, m_OntopFilestream);
					}
				}

				// Write logger stop message and dump it right away
				wmLogMessage *stopMsg = wmLogMessage::create(std::string(" -- Logger stopped -- \n"),
					std::string("<wmBaseLogger::stop>"), m_LoggerType, m_Verbosity);
				std::string theTime("");
				generateTimestamp(theTime);
				stopMsg->setTimestamp(theTime);
				m_oLocalFormatter.setFormat("%d %n - %s: %m");
				commitMessage(stopMsg, &m_oLocalFormatter, m_OntopFilestream);
				delete stopMsg;

				m_Stopping = false;
				return 0; // all OK
			}

			/// pauses the logger
			void pause()
			{
				if ( !m_Running || m_Stopping || m_Pause)
					return;

				safeLockStream();                                                        // avoid further logging
				m_Pause = true;

				m_oTimerLogfile.restart(0);
				if (m_DumpOnPause)
				{
					m_DumpThreshold = loadAcquireInt32(m_Queue.size());                  // dump strategy: dump what is left in queue
					pushConditionWait();                                                 // signal cond var about new dump strategy

					safeReleaseStream();                                                 // necessary so we do not get stuck within a possible concurrent log

					while (loadAcquireInt32(&m_Dumping) == wmBaseStream::m_One)
						Poco::Thread::sleep(5);

					while (m_Queue.size() > 0)
					{
						wmLogMessage *msg = m_Queue.popSpinlock();                           // pop remaining messages from queue
						if (msg != NULL)
						{
							commitMessage(msg, &m_Formatter, m_OntopFilestream);
						}
					}
				} else
				{
					safeReleaseStream();                                                  // necessary so we do not get stuck within a possible concurrent log
				}

				// Write logger stop message and dump it right away
				wmLogMessage *stopMsg = wmLogMessage::create(std::string(" > Logger paused < \n"),
					std::string("<wmBaseLogger::pause>"), m_LoggerType, m_Verbosity);
				std::string theTime("");
				generateTimestamp(theTime);
				stopMsg->setTimestamp(theTime);
				m_oLocalFormatter.setFormat("%d %n - %s: %m");
				commitMessage(stopMsg, &m_oLocalFormatter, m_OntopFilestream);
				delete stopMsg;
			}

			/// resumes logger from pause state
			inline void resume()
			{
				if ( !m_Running || m_Stopping || !m_Pause)
					return;
				m_oTimerLogfile.restart();
				// Write logger stop message and dump it right away
				wmLogMessage *stopMsg = wmLogMessage::create(std::string(" > Logger resumed < \n"),
					std::string("<wmBaseLogger::resume>"), m_LoggerType, m_Verbosity);
				std::string theTime("");
				generateTimestamp(theTime);
				stopMsg->setTimestamp(theTime);
				m_oLocalFormatter.setFormat("%d %n - %s: %m");
				commitMessage(stopMsg, &m_oLocalFormatter, m_OntopFilestream);
				delete stopMsg;

				m_Pause = false;
			}

			/// Create time stamp or not when generating message. For reasons of consistency, this is a "global" static member variable.<br /><b>Default: true.</b>
			static bool generateTimestampOnCreation;

			// --------- Region internal mesage formatter ---------
			/// Lets the programmer define the format of the messages (see precitec::utils::wmLogFormatter for details).
			bool setMessageFormat(std::string fmt)
			{
				// see class wmLogFormatter for details
				return m_Formatter.setFormat(fmt);
			}

			// ===================================================================
			// ------------------ nested class wmLogFormatter --------------------
			/**
			* \class wmLogFormatter
			*
			* \brief Small helper class for formatting message including it's logger context, nested in class wmLoggerBase.
			*
			* The logger allows a message to be composed of an arbitraty combination of letters, digits, punctuations (including the underscore character),
			* whitespaces and the six placeholders given below. At least one placeholder must be used. The format can be chosen using
			* the method <b>setFormat(patternstring)</b>.<br />
			*
			* <ul>
			* <li>	\%d - date/ timestamp</li>
			* <li>	\%i - logger type (identifier)</li>
			* <li>	\%n - logger name (not yet implemented)</li>
			* <li>	\%m - message</li>
			* <li>	\%t - message type</li>
			* <li>	\%s - message sender/ creater label</li>
			* </ul>
			*
			* Here the regular expression for valid formats (perl regexp format):
			* \code
			* ([[:word:][:blank:][:punct:]]*%(n|i|m|d|t|s))+
			* \endcode
			*
			* <i>Example: </i>"-> %d %n, %t (%s): %m"<br />
			* <i>Default: </i>"%d %t %s  %m"
			**/
			class wmLogFormatter
			{
			public:
				/// Default constructor.
				wmLogFormatter()
				{
					m_RegexPattern = "([[:word:][:blank:][:punct:]]*%(n|i|m|d|t|s))+";
					m_Format = new Poco::RegularExpression(m_RegexPattern);
					setFormat(std::string("%d %t %s  %m"));
				}
				/// Default destructor.
				~wmLogFormatter()
				{
					delete m_Format;
				}

				/// Sets the format string if it is a valid pattern w.r.t. the formatter's regular expression. Returns <i>true</i> if ok, <i>false</i> otherwise.<br /><b>Throws </b> char* exception on error.
				bool setFormat(std::string s)
				{
					if (s.length() < 2)
						return false;

					bool isOk = m_Format->match(s);
					if (!isOk)
					{
						std::string stmp("WARN <wmLogFormatter::setFormat>: Invalid format pattern \'" + s + "\'.");
						throw (stmp.c_str());
					}
					else
					{
						m_Pattern.assign(s);
					}
					return isOk;
				}

				/// Returns the format (pattern) string.
				std::string format()
				{
					return m_Pattern;
				}

				/// Builds the string according to the current pattern.
				void buildMessage(wmLoggerBase *logger, wmLogMessage *msg, std::string& result)
				{
					result = m_Pattern;
					std::string txtLogtype("");
					logtypeToString(msg->msgType(), txtLogtype);
					Poco::replaceInPlace(result, std::string("%n"), logger->name());
					Poco::replaceInPlace(result, std::string("%i"), logger->identify());
					Poco::replaceInPlace(result, std::string("%m"), msg->msg());
					Poco::replaceInPlace(result, std::string("%d"), msg->timestamp());
					Poco::replaceInPlace(result, std::string("%t"), txtLogtype );
					Poco::replaceInPlace(result, std::string("%s"), msg->sender());
				}

			protected:
				/// Internal converter logtype -> std::string. <br /><b>Throws </b> char* exception on error.
				void logtypeToString(tLogtype lt,  std::string &result)
				{
					// undefinde NULL-logger. Should not happen
					if (!lt) {
						throw ("ERR <wmLogFormatter::msgToString>: Invalid message type (null)!");
					}

					result = "";

					if (lt & _logInfo)
						result += "|Info";
					if (lt & _logWarning)
						result += "|Warning";
					if (lt & _logError)
						result += "|Error";
					if (lt & _logFatal)
						result += "|Fatal";
					if (lt & _logDebug)
						result += "|Debug";
					if (lt & _logNIO)
						result += "|NIO";
					if (lt & _logVerbose)
						result += "|Verbose";

					result += "|";
				}

				/*
				* The variables m_Format and m_RegexPattern represent the regular expression itself, where the first is
				* ad Poco variable representing the expression being initialized with it (the latter).
				* m_RegexPattern is thus -- as the name suggests -- a regular expression.
				* m_Pattern is the string representing the user defined output, for instance "%d %n  %m".
				*/
				/// The regular erpression variable.
				Poco::RegularExpression *m_Format;
				/// The RegEx-Term fo the variable
				std::string m_RegexPattern;
				/// The user string, which will be substituted
				std::string m_Pattern;
			};

			/// Opens additional filestream and enables additional output to file for non base-filestream loggers.
			inline int enableFilestream()
			{
				if (m_Running)
				{
					return 1;
				}

				if (m_wmStream->type() != _streamFile)
				{
					if (m_oStreamType != _streamNull)
						setOntopFileStream(&m_OntopFilestream, m_oLogfile);

					if ( m_OntopFilestream->good() )
					{
						m_WriteToFile = true;
						return 0;
					}
					else
					{
						m_WriteToFile = false;
						throw ("ERR <wmStreamContext::enableOntopFile> : Invalid or closed file stream.");
						return 1;
					}
				}
				else
				{
					m_WriteToFile = false;
					return 0; // or 1? it is not really an error though...
				}
			}

			/// Closes additional filestream and disables additional output to file for non base-filestream loggers.
			inline void disableFilestream()
			{
				m_WriteToFile = false;
				if (m_OntopFilestream != NULL)
				{
					if (m_OntopFilestream->is_open() )
					{
						m_OntopFilestream->close();
					}
				}
			}

			/// Returns state of context, which equals that of its stream.
			inline bool good(void)
			{
				return m_wmStream->good();
			}

			/// Sets commit flush strategy for ontop FILES. TRUE might slow down the process a little.
			inline void setFlushImmediately(bool m_pFlush)
			{
				m_oFlushImmediately = m_pFlush;
			}

			/// Returns commit flush strategy for ontop FILES.
			inline bool flushImmediately()
			{
				return m_oFlushImmediately;
			}

		protected:
			/// Basic Initializations for all loggers.
			void init(tStream st, std::string sn)
			{
				// default: Nullstream
				m_FileStream = NULL;
				m_Name.assign("");                                   // name of logger
				m_oFlushImmediately = false;
				m_Pause = false;
				m_DumpOnPause = true;
				m_LoggerType = precitec::utils::_defaultLogtype;
				m_Verbosity = precitec::utils::_defaultVerbosity;
				m_Running = false;
				m_Stopping = false;
				m_DumpBlocked = wmBaseStream::m_Zero;
				m_Dumping = wmBaseStream::m_Zero;
				m_Filechange = wmBaseStream::m_Zero;
				m_DumpThreshold = _defaultDumpStrategy;
				m_oStorageStrategy = _defaultFileStorageStrategy;
				_isPushingOrDumping = 0;

				m_WriteToFile = false;
				m_OntopName.assign("");
				m_OntopFilestream = NULL;

				m_BaseFilename.assign(sn);                           // set base filename and...
				generateNewFilename(m_oLogfile);            // generate appropriate name according to file storage strategy

				m_oStreamType = st;
				switch (st)
				{
				case _streamFile: initStream(m_FileStream, st, m_oLogfile, NULL); break;
				case _streamConsoleOut: initStream(NULL, st, m_oLogfile, &m_OntopFilestream); break;
				case _streamConsoleErr: initStream(NULL, st, m_oLogfile, &m_OntopFilestream); break;
				case _streamNull: default: initStream(NULL, st, m_oLogfile, NULL); break;
				}

				unsigned long int period = storageStrategyToMilliseconds();

				m_oTimerLogfile.setStartInterval(period);
				m_oTimerLogfile.setPeriodicInterval(period);
				m_pCallback = new Poco::TimerCallback<wmLoggerBase>(*this, &wmLoggerBase::changeTargetfile);
				// Consumer activity variable initialization.
				m_actCons = new Poco::Activity<wmLoggerBase>(this, &wmLoggerBase::run);
				// Producer activity variable initialization.
				m_actProd = new Poco::Activity<wmLoggerBase>(this, &wmLoggerBase::pushConditionWait);

				// Signal variable for dump action (consumer run method).
				m_DumpOK = 0;
			}

			/// Additional, logger type specific inits for verbosity and logger type
			inline void tinit(tLogtype loggerType, unsigned int verbosity)
			{
				m_LoggerType = loggerType;
				m_Verbosity = verbosity;
			}


			//------------------------------------------------------
			//* ------------ start of streaming section ------------

			/// Explicit constructor. No context without stream, streamtype and filename (either for filestream or for additional ontop filestream).
			void initStream(wmBaseStream *s, tStream st, std::string fn, std::ofstream **ontopHdl)
			{
				m_wmStream = NULL;
				if ( (st == _streamFile) && (ontopHdl != NULL) )
				{
					wmFileStream *fs = dynamic_cast<wmFileStream*>(s); // wmFileStream is created...
					setFileStream(&fs, fn);                            // ...in this function!
				}
				else
				{
					switch(st)
					{
					case _streamConsoleOut: m_wmStream = new wmConsoleStream(false); break;
					case _streamConsoleErr: m_wmStream = new wmConsoleStream(true); break;
					case _streamNull: default: m_wmStream = new wmNullStream(); break;
					}
				}
			}

			/// Set filestream/ change to filestream. <br /><b>Throws </b> char* exception on error.
			int setFileStream(wmFileStream** fs, std::string filename)
			{
				/* returns 0 if OK, a positive integer on error */

				// Filestream not yet opened? -> create/ open it right now and throw exception on failure
				if ( (*fs) == NULL )
				{
					// create new filestream
					(*fs) = new wmFileStream(filename);
					if ( ( (*fs) != NULL) && (*fs)->is_open() )
						m_wmStream = (*fs);
					else
					{
						std::string errStr = "ERR <wStreamContext::setFileStream> : Cannot create logfile " + filename + ".";
						throw errStr.c_str();
						return 2;
					}
				}

				// JIC. This should not happen, but there might be a programmer who tries to set different filenames to one handle...
				if (filename != (*fs)->filename() )
				{
					std::string errStr = "ERR <wStreamContext::setFileStream> : Filehandle already in use ( " + (*fs)->filename() + " ).";
					throw errStr.c_str();
					return 1;
				}

				if (!(*fs)->is_open())
				{
					std::string errStr = "ERR <wStreamContext::setFileStream> : Filehandle of logfile " + filename + " has become invalid. Closing file and deleting handle.";
					delete (*fs);
					(*fs) = NULL;
					throw errStr.c_str();
					return 3;
				}

				m_WriteToFile = false;
				m_wmStream = (*fs);
				return 0;
			}

			/// Initialize and open additional filestream (for non null- and non filestream loggers). File remains open.<br /><b>Throws </b> char* exception on error.
			int setOntopFileStream( std::ofstream ** fs, std::string filename)
			{
				/* returns 0 if OK, a positive integer on error */
				if ( (m_wmStream != NULL) )
				{
					if ( (m_wmStream->type() == _streamFile) || (m_wmStream->type() == _streamNull) )
					{
						m_WriteToFile = false;
						throw ("ERR <wStreamContext::setOntopFileStream> : File- and nullstreams have no additional filestream.");
						return 1;
					}
				}
				else // really should not happen
				{
					throw ("ERR <wStreamContext::setOntopFileStream> : Invalid streamhandle.");
					return 2;
				}

				// Filestream not yet opened? -> create/ open it right now and throw exception on failure
				if ( ((*fs) == NULL) && (m_OntopFilestream == NULL) )
				{
					// create new filestream
					(*fs) = new std::ofstream(filename.c_str());
					if ( ( (*fs) != NULL) && (*fs)->is_open() )
					{
						m_OntopFilestream = (*fs);
						m_OntopName = filename;
						return 0;
					}
					else
					{
						m_WriteToFile = false;
						std::string errStr = "ERR <wStreamContext::setOntopFileStream> : Cannot create additional logfile " + filename + ".";
						throw errStr.c_str();
						return 3;
					}
				}
				else // should NOT happen
					throw ("ERR <wStreamContext::setOntopFileStream> : Additional logfile already initialized.");
			}

			void nop(Poco::Timer &p_rTimer)
			{
				Poco::Thread::sleep(20);
			}

			/// Changes the filename following the file storage strategy. This is a callback function for poco::timer. If the base stream is a filestream, this function effects exactly that stream. Otherwise, if it is NOT a nullstream, the additional filestream will be changed.
			void changeTargetfile(Poco::Timer &p_rTimer)
			{
				if (!m_WriteToFile || m_Pause || !m_Running || m_Filechange)
				{
					return;
				}

				// wait for active push to be finished
				while ( safeStreamLocked() )
				{
					Poco::Thread::sleep(3);
				}

				// lock pushing
				safeLockStream();
				// empty queue in old file
				int m_DumpTmp = m_DumpThreshold;
				std::string oldLogfile(m_oLogfile);
				storeReleaseInt32(&m_Filechange, wmBaseStream::m_One);
				m_DumpThreshold = loadAcquireInt32(m_Queue.size());
				generateNewFilename(m_oLogfile);
				// allow pushing again
				safeReleaseStream();



				// wait until a potential dump ends
				while (loadAcquireInt32(&m_Dumping) != wmBaseStream::m_Zero)
				{
					Poco::Thread::sleep(3);
				}
				storeReleaseInt32(&m_DumpBlocked, wmBaseStream::m_Zero);
				pushConditionWait(); // before the while loop?
				Poco::Thread::sleep(5);
				//   storeReleaseInt32(&m_DumpBlocked, wmBaseStream::m_Zero);
				// wait until a potential dump ends
				while (loadAcquireInt32(&m_Dumping) != wmBaseStream::m_Zero)
				{
					Poco::Thread::sleep(3);
				}
				std::string theMsg(" -> Continued in file ");
				theMsg = theMsg + m_oLogfile + "\n";
				wmLogMessage *filechgMsg = wmLogMessage::create(theMsg,
					std::string("<wmBaseLogger::changeTargetfile>"), m_LoggerType, m_Verbosity);
				// m_oLocalFormatter.setFormat("%d %n - %s: %m");
				commitMessage(filechgMsg, &m_oLocalFormatter, m_OntopFilestream);
				delete filechgMsg;

				// No message dumping allowed during target file changes!!!
				storeReleaseInt32(&m_DumpBlocked, wmBaseStream::m_One);

				wmConsoleStream *wcout(nullptr);
				wmConsoleStream *wcerr(nullptr);
				wmFileStream *wfile;
				int error;

				switch (m_wmStream->type())
				{
				case _streamConsoleOut:
					wcout = dynamic_cast<wmConsoleStream*>(m_wmStream);
					error = wcout->changeFilestream(m_oLogfile, &m_OntopFilestream, &m_OntopName);
					break;
				case _streamConsoleErr:
					wcerr = dynamic_cast<wmConsoleStream*>(m_wmStream);
					error = wcerr->changeFilestream(m_oLogfile, &m_OntopFilestream, &m_OntopName);
					break;
				case _streamFile:
					wfile = dynamic_cast<wmFileStream*>(m_wmStream);
					error = wfile->changeFilestream(m_oLogfile, NULL, NULL);
					break;
				case _streamNull: default: error = 0; break; // nothing to be done
				}

				m_DumpThreshold = loadAcquireInt32(m_DumpTmp);
				theMsg = " <- Continued from file ";
				theMsg = theMsg + oldLogfile + "\n";
				filechgMsg = wmLogMessage::create(theMsg,
					std::string("<wmBaseLogger::changeTargetfile>"), m_LoggerType, m_Verbosity);
				// m_oLocalFormatter.setFormat("%d %n - %s: %m");
				commitMessage(filechgMsg, &m_oLocalFormatter, m_OntopFilestream);
				delete filechgMsg;

				// dump strategy: dump what is left in queue

				if (error)
				{
					std::cerr << "[BaseStream::changeTargetfile] ERROR: Cannot create file " << m_oLogfile << ". Disk full?\n";
				}
				storeReleaseInt32(&m_Filechange, wmBaseStream::m_Zero);
				storeReleaseInt32(&m_DumpBlocked, wmBaseStream::m_Zero);
			}


			//------------------------------------------------------
			//* ------------- end of streaming section -------------

			int  g_oLogStreamAccess; ///< Synchronize access to main buffer.

			/// Directs message to output stream(s)
			void commitMessage(wmLogMessage *msg, wmLogFormatter *fmt, std::ofstream* sFile)
			{
				std::string theMsg;
				fmt->buildMessage(this, msg, theMsg);

				// flush the message into the shared memory
				m_wmStream->flush(theMsg);

				// If we are additionally writing to a file...
				if ( m_OntopFilestream && m_WriteToFile && m_OntopFilestream->good() )
				{
					(*m_OntopFilestream) << theMsg;
					if (m_oFlushImmediately)
					{
						m_OntopFilestream->flush();
					}
				}
			}

			/// Generates a new (date and time dependent) filename in line with the currently active fileStorageStrategy
			void generateNewFilename(std::string &ret)
			{
				ret = m_BaseFilename;
				std::string date("");
				generateFilestreamTimestamp(date);
				ret = ret + "_log_" + date + ".txt";;
			}

			/// Internally used within consumer/producer dump wait.
			void pushConditionWait()
			{
				//m_DumpOK = m_Queue.size();
				//	while (m_DumpOK )
				if (loadAcquireInt32(&m_DumpBlocked) == wmBaseStream::m_Zero)
				{
					m_QueueDumpMutex.lock();
					m_DumpOK = m_Queue.size();
					m_QueueDumpCondvar.signal();
					m_QueueDumpMutex.unlock();
				}
			}

			/// Type of the logger. 0 for this baseclass.
			tLogtype m_LoggerType;
			/// Verbosity level [0, 9].
			unsigned int m_Verbosity;

			/// Additional filestream enabled?
			bool m_WriteToFile;
			/// Additional filestream's name.
			std::string m_OntopName;
			/// Additional non base filestream.
			std::ofstream *m_OntopFilestream;
			/// File storage strategy (=interval)
			tFileStorageStrategy m_oStorageStrategy;
			/// Callback for the filestorage timer
			Poco::TimerCallback<wmLoggerBase> *m_pCallback;

			// Static and non static stream members. (Nullstream, Consolestreams) will be shared by all loggers
			/// Base stream
			wmBaseStream *m_wmStream;
			/// Base Filestream; will only be created if necessary
			wmFileStream *m_FileStream;
			/// stream type
			tStream m_oStreamType;

			/// The MPSC-Queue
			wmLogQueue m_Queue;

			// Queue consumer methods and logger state variables.
			/// Indicates whether logger is running.
			bool m_Running;
			/// Indicates whether logger is paused.
			bool m_Pause;
			/// Indicates whether logger is in stopping process ( queue is emptied here ).
			bool m_Stopping;
			/// Dumps queue when true and logger is paused.
			bool m_DumpOnPause;
			/// Indicates whether message dumping from queue is blocked, for instance due to a change of the target storage file.
			volInt32 m_DumpBlocked;
			/// Indicates that we are in the process of dumping from queue. This is necessary for instance when changing the target file name, as we should not dump while doing so.
			volInt32 m_Dumping;
			/// Indicates whether the filestream (filename) is changed and blocks concurrent changes.
			volInt32 m_Filechange;
			///ged Locks out concurrent dumping and pushing
			volInt32 _isPushingOrDumping;


			/// Poco Activity for the consumer method (run/pop).
			Poco::Activity<wmLoggerBase> *m_actCons;
			/// Poco Activity for the producer method (push)
			Poco::Activity<wmLoggerBase> *m_actProd;

			/// Mutex associated with queue dump conditional variable.
			Poco::FastMutex m_QueueDumpMutex;
			/// Conditional variablethe for queue dump: dump according to strategy only if queue has certain number of elements
			Poco::Condition m_QueueDumpCondvar;
			/// Activation variable for the condition variable m_QueueDumpCondvar. This activation variable mirrors the queue's size.
			volInt32 m_DumpOK;
			/// Size of queue necessary for dumping logs to the chosen stream and potential additional file.
			volInt32 m_DumpThreshold;

			/// Poco-Timer for filestream handling. Callback time interval due to storage strategy.
			Poco::Timer m_oTimerLogfile;
			std::string m_oLogfile;
			wmLogFormatter m_oLocalFormatter;
			/// flush immediately or after dump. The latter saves time butmight result in message loss when something goes awry during dump... but this is rare and thus default.
			bool m_oFlushImmediately;

			// todo: wirklich innerhalb des Threads thrown benutzen? Dann MUSS der Nutzer auf jeden fall try-catch verwenden....
			/// Function object for thread: This is the consumer thread. <br /><b>Throws </b> char* exception on error.
			void run()
			{
				// This is the consumer function for the queue and is governed by a thread which is created in the sta-method.
				while ( m_Running | m_Stopping )
				{
					// Lock fast mutex assicoated with conditional variable and dump if queue size reaches dump threshold.
					m_QueueDumpMutex.lock();
					m_DumpOK = m_Queue.size();
					// wait for internal variable and potential threshold change
					while ( (m_DumpOK < m_DumpThreshold) || (loadAcquireInt32(&m_DumpBlocked) == wmBaseStream::m_One) )
					{
						//std::cout << "waiting... " << m_DumpOK << " of " << m_DumpThreshold << "\n";
						m_DumpOK = m_Queue.size();
						m_QueueDumpCondvar.wait(m_QueueDumpMutex);
					}

					storeReleaseInt32(&m_Dumping, wmBaseStream::m_One);

					// adjust variable, send conditional var signal and unlock mutex
					m_DumpOK = 0;
					m_QueueDumpCondvar.signal();
					m_QueueDumpMutex.unlock();

					// dump ( = consume = pop ) m_DumpThreshold messages

					int toDump = m_DumpThreshold;

					//ged Wait until a potential push ends
					volInt32 *pLock = &_isPushingOrDumping;
					while (atomicXchgInt32(pLock, 1))
						usleep(50);

					while (toDump)
					{
						wmLogMessage *msg = m_Queue.popSpinlock();                           // pop message from queue
						if (msg == NULL)
						{
							atomicXchgInt32(pLock, 0);
							storeReleaseInt32(&m_Dumping, wmBaseStream::m_Zero);
							toDump = 0;
							throw ("ERR <wmLoggerBase::run>: received message nullpointer.");
						}
						else
						{
							commitMessage(msg, &m_Formatter, m_OntopFilestream);
						}
						toDump--;
						delete msg;
					}
					// todo: lock m_OntopFilestream for if and flush
					if ( *m_OntopFilestream && m_OntopFilestream->good() )
					{
						m_OntopFilestream->flush();
					}
					// if we are stopping, we need to make sure that after the final dump we do not get stuck in the big while loop.
					if (m_Stopping)
						m_Stopping = false;
					atomicXchgInt32(pLock, 0);
					storeReleaseInt32(&m_Dumping, wmBaseStream::m_Zero);
				} // while (m_Running)
			}

			/// Generates and sets time stamp for logfiles/ filestreams. Separate function for performance reasons of generateTimestamp.
			void generateFilestreamTimestamp(std::string &sTime)
			{              
				std::stringstream ss;
				std::time_t t = std::time(nullptr);
				std::tm tm = *std::localtime(&t);
				const std::string fmt {"%Y_%m_%d_%H_%M_%S"};
				ss << std::put_time(&tm, fmt.c_str());
				sTime = ss.str();
			}

			/// Generates and sets the time stamp for the log message.
			void generateTimestamp(std::string &sTime)
			{
				std::stringstream ssTime;

				// get date
				char dateString[32];
				time_t m_TheTime = time(0);
				strftime(dateString, 30, "%Y-%m-%d_%H:%M:%S", localtime(&m_TheTime));
				ssTime << dateString;

				// get time
				timeval tv;
				gettimeofday(&tv, 0);                                                    // get #seconds passed since 01.01.1970, BIOS CLOCK reference, not OS!!!
				long d = (tv.tv_sec % 86400);                                            // get hour
				d  = (tv.tv_usec / 1000);                                             // get milliseconds and finish string composition
				ssTime << "." << std::setfill('0') << std::setw(3) << d;
				sTime =  ssTime.str();
			}

			/// Name of the logger.
			std::string m_Name;

			/// Transforms storage strategy into milliseconds for use in poco::utils::timer
			unsigned long storageStrategyToMilliseconds()
			{
				unsigned long perHour=3600000;                                         // milliseconds per hour (1000*3600)
				unsigned long perDay=perHour*24;
				switch(m_oStorageStrategy)
				{
				case _fileFiveSec: return 5000;
				case _fileTenSec:  return 10000;
				case _fileMinute:  return 60000;
				case _fileQuarter: return 60000*15;                                    // 15 minutes
				case _fileHour:    return perHour;                                     // one hour
				case _fileHour12:  return perHour*12;                                  // 12 hours
				case _fileDay:     return perDay;                                      // one day
				case _fileWeek: case _fileMax:   return perDay*7;                                    // one week
					/* poco::timer crashes on higher values...
					case _fileMonth:   return perDay*30;                                   // 30 days
					case _fileMonth6:  return perDay*30*6;                                 // 180 days
					case _fileYear: case _fileMax:    return perDay*365;                   // 365 days
					*/
				default:           return perDay;
				}
				return perDay;
			}

			// ----------------------- Region internal message formatter ----------------------
			/// Formatter object for formatting logs.
			wmLogFormatter m_Formatter;

			/// Base filename <b>prefix</b> for filestream or additional ontop filename
			std::string m_BaseFilename;
		};

		/*
		wmNullStream* getNullStream()
		{
		return new wmNullStream;
		}
		*/
		// ============================================================================
		// ----------------------- Instantiable logger classes ------------------------
		/** @param Base <i>int</i>-template logger class.
		*
		* \brief Abstract template base class for the various logger classes. Check class wmLoggerBase for more details.
		*
		* This is not instantiable and a dummy necessary for deriving the specialiyzed
		* implementations from wmLoggerBase, thus avoiding a lot code repetition.
		*/
		template<int n>
		class wmLogger : public wmLoggerBase
		{
		public:
			/// Explicit constructor.
			wmLogger(tStream stream, std::string sname):wmLoggerBase(stream, sname){};
			/// Virtual standard destructor.
			virtual ~wmLogger(){};
		};

		//----------------------------------------------------------------
		/** @param Logger type <i>_logInfo</i> for specialization
		*
		*	\brief Implements an info logger as a specialized template. Check class wmLoggerBase for more details.
		*/
		template<>
		class wmLogger<_logInfo> : public wmLoggerBase
		{
		public:
			/// Constructor (streamType, streamName="").
			explicit wmLogger(tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logInfo, _defaultVerbosity);
			}
			/// Constructor (verbosity, streamType, streamName="")
			explicit wmLogger(unsigned int vb, tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logInfo, vb%10);
			};
			~wmLogger<_logInfo>(){};

			std::string identify()
			{
				return ("[info] ");
			}
		};
		/// Simplified info logger definition.
		typedef wmLogger<_logInfo> wmLoggerInfo;

		//----------------------------------------------------------------
		/** @param Logger type <i>_logEverything</i> for specialization
		*
		*  \brief Implements an info logger as a specialized template. Check class wmLoggerBase for more details.
		*/
		template<>
		class wmLogger<_logEverything> : public wmLoggerBase
		{
		public:
			/// Constructor (streamType, streamName="").
			explicit wmLogger<_logEverything>(tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logEverything, _defaultVerbosity);
			}
			/// Constructor (verbosity, streamType, streamName="")
			explicit wmLogger(unsigned int vb, tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logEverything, vb%10);
			};
			~wmLogger<_logEverything>(){};

			std::string identify()
			{
				return ("[everything] ");
			}
		};
		/// Simplified info logger definition.
		typedef wmLogger<_logEverything> wmLoggerEverything;

		/*
		class loggerInfo : public wmLoggerInfo
		{
		public:
		explicit loggerInfo(wmLoggerInfo* p_pLogger, long int p_oThreadID)
		{
		m_oThreadID = p_oThreadID;
		m_pLogger = p_pLogger;
		}
		~loggerInfo()
		{
		};

		wmLoggerInfo* m_pLogger;
		private:
		long int m_oThreadID;
		};
		*/

		//----------------------------------------------------------------
		/** @param Logger type <i>_logWarning</i> for specialization
		*
		*	\brief Implements a warning logger as a specialized template. Check class wmLoggerBase for more details.
		*/
		template<>
		class wmLogger<_logWarning> : public wmLoggerBase
		{
		public:
			/// Constructor (streamType, streamName="").
			explicit wmLogger(tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logWarning, _defaultVerbosity);
			}
			/// Constructor (verbosity, streamType, streamName="")
			explicit wmLogger(unsigned int vb, tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logWarning, vb%10);
			};
			~wmLogger(){};

			std::string identify()
			{
				return ("[warning] ");
			}
		};
		/// Simplified warning logger definition.
		typedef wmLogger<_logWarning> wmLoggerWarning;

		//----------------------------------------------------------------
		/** @param Logger type <i>_logError</i> for specialization
		*
		*	\brief Implements an error logger as a specialized template. Check class wmLoggerBase for more details.
		*/
		template<>
		class wmLogger<_logError> : public wmLoggerBase
		{
		public:
			/// Constructor (streamType, streamName="").
			explicit wmLogger(tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logError, _defaultVerbosity);
			}
			/// Constructor (verbosity, streamType, streamName="")
			explicit wmLogger(unsigned int vb, tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logError, vb%10);
			};
			~wmLogger(){};

			std::string identify()
			{
				return ("[error] ");
			}
		};
		/// Simplified error logger definition.
		typedef wmLogger<_logError> wmLoggerError;

		//----------------------------------------------------------------
		/** @param Logger type <i>_logFatal</i> for specialization.
		*
		*	\brief Implements a logger for fatal/ critical errors as a specialized template. Check class wmLoggerBase for more details.
		*/
		template<>
		class wmLogger<_logFatal> : public wmLoggerBase
		{
		public:
			/// Constructor (streamType, streamName="").
			explicit wmLogger(tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				init(stream, sname);
				tinit(_logFatal, _defaultVerbosity);
			}
			/// Constructor (verbosity, streamType, streamName="")
			explicit wmLogger(unsigned int vb, tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logFatal, vb%10);
			};
			~wmLogger(){};

			std::string identify()
			{
				return ("[fatal] ");
			}
		};
		/// Simplified critical logger definition.
		typedef wmLogger<_logFatal> wmLoggerFatal;

		//----------------------------------------------------------------
		/** @param Logger type <i>_logDebug</i> for specialization
		*
		*	\brief Implements a debug logger as a specialized template. Check class wmLoggerBase for more details.
		*/
		template<>
		class wmLogger<_logDebug> : public wmLoggerBase
		{
		public:
			/// Constructor (streamType, streamName="").
			explicit wmLogger(tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logDebug, _defaultVerbosity);
			}
			/// Constructor (verbosity, streamType, streamName="")
			explicit wmLogger(unsigned int vb, tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logDebug, vb%10);
			};
			~wmLogger(){};

			std::string identify()
			{
				return ("[debug] ");
			}
		};
		/// Simplified debug logger definition.
		typedef wmLogger<_logDebug> wmLoggerDebug;


		//----------------------------------------------------------------
		/** @param Logger type <i>_logNIO</i> for specialization
		*
		*	\brief Implements a NIO logger as a specialized template. Check class wmLoggerBase for more details.
		*/
		template<>
		class wmLogger<_logNIO> : public wmLoggerBase
		{
		public:
			/// Constructor (streamType, streamName="").
			explicit wmLogger(tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logNIO, _defaultVerbosity);
			}
			/// Constructor (verbosity, streamType, streamName="")
			explicit wmLogger(unsigned int vb, tStream stream, std::string sname):wmLoggerBase(stream, sname)
			{
				tinit(_logNIO, vb%10);
			};
			~wmLogger(){};

			std::string identify()
			{
				return ("[nio] ");
			}
		};
		/// Simplified NIO logger definition.
		typedef wmLogger<_logNIO> wmLoggerNIO;


		/**
		* \class refLogger
		*
		* \brief This is the logger streaming class interface.
		*
		* Each thread/ process has to create its own refLogger object with a base logger as a parameter.
		*
		* Logging can be done by directly logging messages or by sequentially streaming them, see the respective methods.
		*
		**/
		class refLogger
		{
		public:
			/// Explicit constructor with base logger and threadID as parameters, the latter not yet used (2011/04)
			explicit refLogger(wmLoggerBase* p_pLogger, long int p_oThreadID)
			{
				m_pTheLogger = p_pLogger;
				m_oThreadID = p_oThreadID;
			}
			explicit refLogger()
			{
				m_pTheLogger = NULL;
				m_oThreadID = 0;
			};

			void setMainLogger(wmLoggerBase* p_pLogger, long int p_oThreadID)
			{
				m_pTheLogger = p_pLogger;
				m_oThreadID = p_oThreadID;
			}

			wmLoggerBase* getMainLogger()
			{
				return m_pTheLogger;
			}

			//-------------------------------------------------------------------------------------------------------------
			///  streaming operator << will be passed through to wmStream. This allows for easy use.
			template<typename T>
			refLogger& operator<<(const T& val)
			{
				if ( (m_pTheLogger->streamType() != _streamNull) ||
					(m_pTheLogger->m_Pause | m_pTheLogger->m_Stopping | !m_pTheLogger->m_Running) )
					m_pLocalBuffer << val;

				return *this;
			}

			/// Overloaded shift operator << for standard ostreams.
			refLogger& operator<<(std::ostream& (*val)(std::ostream&))
			{
				if ( (m_pTheLogger->streamType() != _streamNull) ||
					(m_pTheLogger->m_Pause | m_pTheLogger->m_Stopping | !m_pTheLogger->m_Running) )
					m_pLocalBuffer << val;

				return *this;
			}

			/// Overloaded shift operator << for standard ios-streams.
			refLogger& operator<<(std::ios& (*val)(std::ios&))
			{
				if ( (m_pTheLogger->streamType() != _streamNull) ||
					(m_pTheLogger->m_Pause | m_pTheLogger->m_Stopping | !m_pTheLogger->m_Running) )
					m_pLocalBuffer << val;

				return *this;
			}

			/// Overloaded shift operator << for the ios base package.
			refLogger& operator<<(std::ios_base& (*val)(std::ios_base&))
			{
				if ( (m_pTheLogger->streamType() != _streamNull) ||
					(m_pTheLogger->m_Pause | m_pTheLogger->m_Stopping | !m_pTheLogger->m_Running) )
					m_pLocalBuffer << val;

				return *this;
			}

			//-------------------------------------------------------------------------------------------------------------
			// Producer interfaces
			/** \fn pushLog(std::string)
			* <b>[Producer Interface]</b> Creates message from streams (including timestamp) and logs it
			* if (MessageVerbosity >= LoggerVerbosity) and ((message type bit || logger bit)==1) (warning, info, ...)
			* Finalizes <<-log accumulation by flushing and clearing message buffer if necessary.<br />
			* <b>Defatults: Logtype of message = info; verbosity = 9.</b><br />
			*/
			void pushLog(std::string senderID)
			{
				pushLog(senderID, precitec::utils::_defaultLogtype, precitec::utils::_defaultVerbosity);
			}
			/// <b>[Producer Interface]</b>. Dito. Arguments: senderID, logtype (warning, info, ...).<br /><b>Defatults: message verbosity = 9.</b>
			void pushLog(std::string senderID, tLogtype lt)
			{
				pushLog(senderID, lt, precitec::utils::_defaultVerbosity);
			}
			/// <b>[Producer Interface]</b> Dito. Arguments: senderID, message verbosity ([0, 9]).<br /><b>Defatults: Logtype of message = info.</b>
			void pushLog(std::string senderID, unsigned int vb)
			{
				pushLog(senderID, precitec::utils::_defaultLogtype, vb%10);
			}
			/// <b>[Producer Interface]</b>. Dito. Arguments: senderID, logtype (warning, info, ...), message verbosity ([0, 9])
			void pushLog(std::string senderID, tLogtype lt, unsigned int vb, std::string msg="")
			{
				//ged Wait until a potential dump ends
				volInt32 *pLock = &m_pTheLogger->_isPushingOrDumping;
				while (atomicXchgInt32(pLock, 1))
					usleep(3);

				// no overlapping pushes from different threads, block asap
				while ( m_pTheLogger->safeStreamLocked() )
					Poco::Thread::sleep(5);

				// block pushes and streaming
				m_pTheLogger->safeLockStream();

				// only continue if logger is active
				if (m_pTheLogger->m_Pause | m_pTheLogger->m_Stopping | !m_pTheLogger->m_Running)
				{
					if (msg=="")
						clearBuffer();
					atomicXchgInt32(pLock, 0);
					m_pTheLogger->safeReleaseStream();
					return;
				}

				vb = vb % 10;

				// If not called via directLog, push message from stream (context)
				if (msg=="")
					msg = m_pLocalBuffer.str();

				// Push message if we are not streaming to NULL and verbosity level and logMessage type match that of the logger.
				if ( (m_pTheLogger->streamType() != _streamNull) &&
					(vb >= m_pTheLogger->verbosity()) && (lt & m_pTheLogger->type() ) )
				{
					wmLogMessage *wlm = wmLogMessage::create(msg, senderID, lt, vb);
					if (m_pTheLogger->generateTimestampOnCreation)
					{
						std::string theTime("");
						m_pTheLogger->generateTimestamp(theTime);
						wlm->setTimestamp(theTime);
					}
					clearBuffer();
					m_pTheLogger->m_Queue.push(wlm);
					// safely get current queue size. m_Queue.size() internally uses loadAcquire, so this should be safe.
					storeReleaseInt32( &m_pTheLogger->m_DumpOK, m_pTheLogger->m_Queue.size() );
				}
				else
				{
					/* The clearing of the stream is left to the stream context, as management of the streams within this class here
					* would imply another ripping apart of the responsibilities/ management of the streams, which is the context's job.
					*/
					clearBuffer();
				}
				// announce the new object we are about to push
				m_pTheLogger->pushConditionWait();

				atomicXchgInt32(pLock, 0);
				// allow pushes and streaming again
				m_pTheLogger->safeReleaseStream();
			}

			/// <b>[Producer Interface]</b>. Dito. Arguments: senderID, logtype (warning, info, ...), message verbosity ([0, 9])
			void pushLog(wmLogMessage *wlm)
			{
				//ged Wait until a potential dump ends
				volInt32 *pLock = &m_pTheLogger->_isPushingOrDumping;
				while (atomicXchgInt32(pLock, 1))
					usleep(3);

				// no overlapping pushes from different threads, block asap
				while ( m_pTheLogger->safeStreamLocked() )
					Poco::Thread::sleep(5);

				// block pushes and streaming
				m_pTheLogger->safeLockStream();

				// only continue if logger is active
				if (m_pTheLogger->m_Pause | m_pTheLogger->m_Stopping | !m_pTheLogger->m_Running)
				{
					atomicXchgInt32(pLock, 0);
					m_pTheLogger->safeReleaseStream();
					return;
				}

				// Push message if we are not streaming to NULL and verbosity level and logMessage type match that of the logger.
				if ( (m_pTheLogger->streamType() != _streamNull) &&
					(wlm->verbosity() >= m_pTheLogger->verbosity()) && (wlm->msgType() & m_pTheLogger->type() ) )
				{
					if (m_pTheLogger->generateTimestampOnCreation)
					{
						std::string theTime("");
						m_pTheLogger->generateTimestamp(theTime);
						wlm->setTimestamp(theTime);
					}
					clearBuffer();
					m_pTheLogger->m_Queue.push(wlm);
					// safely get current queue size. m_Queue.size() internally uses loadAcquire, so this should be safe.
					storeReleaseInt32( &m_pTheLogger->m_DumpOK, m_pTheLogger->m_Queue.size() );
				}
				else
				{
					/* The clearing of the stream is left to the stream context, as management of the streams within this class here
					* would imply another ripping apart of the responsibilities/ management of the streams, which is the context's job.
					*/
					clearBuffer();
				}
				// announce the new object we are about to push
				m_pTheLogger->pushConditionWait();

				// allow pushes and streaming again
				atomicXchgInt32(pLock, 0);
				m_pTheLogger->safeReleaseStream();
			}

			void clearBuffer()
			{
				m_pLocalBuffer.str("");
				m_pLocalBuffer.clear();
			}

			/// Direct log (sender, msg), does not stream the message but directly logs it. Any streamed logs will be ignored and not included.
			void directLog(std::string senderID, std::string msg)
			{
				pushLog(senderID, precitec::utils::_defaultLogtype, precitec::utils::_defaultVerbosity, msg);
			}
			/// Direct log (sender, logtype, msg), dito.
			void directLog(std::string senderID, tLogtype lt, std::string msg)
			{
				pushLog(senderID, lt, precitec::utils::_defaultVerbosity, msg);
			}
			/// Direct log (sender, logmsg), dito.
			void directLog(std::string senderID, unsigned int vb, std::string msg)
			{
				pushLog(senderID, precitec::utils::_defaultLogtype, vb % 10, msg);
			}
			/// Direct log (sender, logmsg), dito.
			void directLog(std::string senderID, tLogtype lt, unsigned int vb, std::string msg)
			{
				pushLog(senderID, lt, vb % 10, msg);
			}

		protected:
			wmLoggerBase* m_pTheLogger;
			long int m_oThreadID;
			std::stringstream m_pLocalBuffer;
		};
	} // namespace utils
} // namespace precitec

#endif /*__precitec__wmLogger_h__ */
