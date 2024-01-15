/**
 * \file wmLogStreams.h
 *
 * \author Andreas Beschorner
 *
 * \date Created on: Jan 04, 2011
 *
 * \brief Implements classes streaming classes ( forwards to console std::cout and std::cerr, a nullstream and a filestream )
 *
 * The stream context is implemented as a modified strategy pattern, the modification stemming upon the fact that some of the streams must allow for
 * an additional filestream switched on and off during runtime. In future, the destination stream might also be changeable during runtime, thus the
 * documentation already includes this potential feature.
 *
 */
#ifndef WMLOGSTREAMS_h__
#define WMLOGSTREAMS_h__

// clib includes
#include <ostream>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream> 			//< for ofstream*
#include <pthread.h>
#include <unistd.h>			//< for usleep()
#include <wmAtomics.h>
// Poco includes
#include <Poco/Timestamp.h>
// WM includes
#include <wmLogMessage.h>

//#define wmDebug__

namespace precitec {
namespace utils {

	/// Target stream enum.
enum tStream {
	_streamNone=0, _streamFile = 1, _streamConsoleOut = 2, _streamConsoleErr = 4,
	_streamNull = 256
};
/// Default stream type.
const tStream _defaultStream = _streamNull;

/**
 *	\file wmLogStreams.h
 *	\author Andreas Beschorner
 *	\date created 2011/01/13
 *
 */

// I do not use a template class for ofstreams, as the stream also might be something NOT base on ostream/ streambuf.

/** \class wmBaseStream
 *
 * \brief (Abstract) baseclass for various derived streaming classes.
 */
class wmBaseStream
{
public:
	/// Standard constructor
	wmBaseStream()
	{
		m_ostream =  NULL;
		m_streamType = precitec::utils::_defaultStream;
		m_StreamBlocked = m_Zero;
	}

	/// Virtual destructor, base class not instanciable
	virtual ~wmBaseStream(void)
	{
	};

	/// Changes filestream. Used in the context of the storage strategy when the target file changes.
	virtual int changeFilestream(const std::string, std::ofstream **, std::string *){return 0;};

	/// Manipulators are redefined as enums to prevent potentially unnecessary code generation due to calling/ generating tab/endl functions.
	enum m_manipulator { tab = '\t', endl = '\n' };

	/// Virtual function. Returns a string identifying the stream by a meaningful name.
	virtual std::string identify()
	{
		return std::string("");
	}

	/// Returns tStream type identifier.
	inline tStream type()
	{
		return m_streamType;
	}

	/// Returns condition of stream.
	inline bool good()
	{
		if (m_ostream == NULL)
			return false;
		return m_ostream->good();
	}

	/// Flushes the stream and clears the buffer. Should be overwritten for specific streams such as for instance netstreams.
	virtual void flush(const std::string &p_pBuffer)
	{
#ifdef _flushDebug  // The real output is part of the consumer (for instance logger)
		(*m_ostream) << p_pbuffer->str();
		m_ostream->flush();
#endif
		while (loadAcquireInt32(&m_StreamBlocked) != m_Zero)
			usleep(3);
		storeReleaseInt32(&m_StreamBlocked, m_One);
		(*m_ostream) << p_pBuffer;
		m_ostream->flush(); // todo: remove this line???
		storeReleaseInt32(&m_StreamBlocked, m_Zero);
	}

	/// Clears the internal stream buffer.
	inline virtual void clear(std::stringstream* p_pBuffer)
	{
		p_pBuffer->str("");
		p_pBuffer->clear();
	}

	/// Internal state variable for m_StreamBlocked
	static volInt32 m_Zero;
	/// Internal state variable for m_StreamBlocked
	static volInt32 m_One;

	/// Returns whether a streaming is locked.
	inline volInt32 streamLocked()
	{
		return (m_StreamBlocked == m_One);
	}

	/// Returns whether a streaming is locked via memory fence read (loadAcquire).
	inline volInt32 safeStreamLocked()
	{
		return ( loadAcquireInt32(&m_StreamBlocked) == m_One );
	}

	/// Locks streaming with memory fence (store release).
	inline void safeLockStream()
	{
		storeReleaseInt32(&m_StreamBlocked, m_One);
	}

	/// Unlocks streaming with memory fence (store release).
	inline void safeReleaseStream()
	{
		storeReleaseInt32(&m_StreamBlocked, m_Zero);
	}


#ifdef wmDebug__
	void print(void);
#endif

protected:
	/// Out-stream member variable. Protected so derived streaming classes can define/allocate/use it according to their strategy.
	std::ostream *m_ostream;
	/// Kind of stream (console, null, ...)
	tStream m_streamType;
	/// Internal variable, blocks stream while being pushed
	volInt32 m_StreamBlocked;

	/// (Re)opens filestream. This functions handles both real filestreams and ontop/ additional filestreams.
	int openFilestream(std::ofstream** hdl, const std::string filename, std::string *newFilename, tStream st)
	{ // returns 1 on error, 0 if everything is fine
		// handle already exists => close before reopening
		if ( (*hdl) != NULL )
		{
			if ( (*hdl)->is_open() )
				(*hdl)->close();

			delete (*hdl);
		}

		(*hdl) = new std::ofstream(filename.c_str());
		if ( ( (*hdl) != NULL) && (*hdl)->good() )
		{
			*newFilename = filename;
			m_streamType = st;
			return 0;
		}
		else
		{
			if ( (*hdl) != NULL)
				delete (*hdl);
			(*hdl) = NULL;
			return 1; // error
		}
	}
};


// -------------------------------------------------------------------------


/** \class wmFileStream
 *
 * \brief Implements a file stream. Check class wmBaseStream for more details.
 */
class wmFileStream : public wmBaseStream
{
public:
	/// Private explicit constructor, filename givan as const char*.
	explicit wmFileStream(const char* filename)
	{
		std::ofstream *tmpO = dynamic_cast<std::ofstream*>(m_ostream);
		openFilestream(&tmpO, std::string(filename), &m_filename, _streamFile);
		m_ostream = tmpO;
        m_StreamBlocked = m_Zero;
	}

	/// Private explicit constructor, filename given as const string.
	explicit wmFileStream(const std::string filename)
	{
		std::ofstream *tmpO = dynamic_cast<std::ofstream*>(m_ostream);
		openFilestream(&tmpO, filename, &m_filename, _streamFile);
		m_ostream = tmpO;
        m_StreamBlocked = m_Zero;
	}

	/// Destructor.
	~wmFileStream(void)
	{
		std::ofstream *tmpO = dynamic_cast<std::ofstream*>(m_ostream);
		if ( (tmpO != NULL) && tmpO->is_open() )
			tmpO->close();

		if (m_ostream != NULL)
			delete m_ostream;
	}

	/// Changes filestream itself.
	int changeFilestream(const std::string filename, std::ofstream **hdl=NULL, std::string *filenameBuffer=NULL)
	{// here hdl is m_ostream and the filename is m_filename
		std::ofstream *tmpO = dynamic_cast<std::ofstream*>(m_ostream);
		return openFilestream(&tmpO, filename, &m_filename, _streamFile);
	}

	/// Implementation of the virtual identify()-method. Returns a string identifying the stream by a meaningful name.
	std::string identify()
	{
		std::string ret="{file:" + m_filename + "}";
		return ret;
	}

	/// Returns filename.
	inline std::string filename()
	{
		return m_filename;
	}

	/// Returns whether file stream if open or not.
	bool is_open()
	{
		std::ofstream *tmpO = dynamic_cast<std::ofstream*>(m_ostream);

		if (tmpO == NULL)
			return false;

		return (tmpO->is_open());
	}

	/// Closes the internal stream.
	void close()
	{
		std::ofstream *tmpO = dynamic_cast<std::ofstream*>(m_ostream);

		if (tmpO == NULL)
			return;

		if (tmpO->is_open())
			tmpO->close();
	}
private:
	std::string m_filename;
};


// -------------------------------------------------------------------------


/** \class wmConsoleStream
 *
 * \brief Implements console streams std::cout and std::cerr. Check class wmBaseStream for more details.
 */
class wmConsoleStream : public wmBaseStream
{
public:
	/// Private explicit constructor. Writes to std::cout (toErr=false) or std::cerr (toErr=true).
	explicit wmConsoleStream(bool toErr)
	{
		if (toErr)
		{
			m_ostream = &std::cerr;
			m_streamType = precitec::utils::_streamConsoleErr;
		}
		else
		{
			m_ostream = &std::cout;
			m_streamType = _streamConsoleOut;
		}
		m_toErrorConsole = toErr;
	}

	/// Destructor.
	~wmConsoleStream(){};

	/// Changes filestream of ontop file.
	int changeFilestream(const std::string filename, std::ofstream **hdl, std::string *filenameBuffer)
	{
		if (m_toErrorConsole)
			return openFilestream(hdl, filename, filenameBuffer, _streamConsoleErr);

		return openFilestream(hdl, filename, filenameBuffer, _streamConsoleOut);
	}

	/// Implementation of the virtual identify()-method. Returns a string identifying the stream by a meaningful name.
	std::string identify()
	{
		std::string ret;
		if (m_toErrorConsole)
			ret = "{::std::cerr}";
		else
			ret = "{::std::cout}";
		return ret;
	}

private:
	bool m_toErrorConsole;                                                   /// Variable representing the stream target: True -> std::cerr, False -> std::cout
};


// -------------------------------------------------------------------------


/** \class wmNullStream
 *
 * \brief Implements a null stream, which ignores incoming data. Check class wmBaseStream for more details.
 */
class wmNullStream : public wmBaseStream
{
public:
	// Standard constructor.
	wmNullStream()
	{
		m_ostream = NULL;
		m_streamType = _streamNull;
        m_StreamBlocked = m_Zero;
	}
	// Standard destructor.
	~wmNullStream(){};

	/// Returns condition of stream. Always true for nullstream.
	bool good(void)
	{
		return true;
	}

	/// Implementation of the virtual identify()-method. Returns a string identifying the stream by a meaningful name.
	std::string identify()
	{
		std::string ret="{0}";
		return ret;
	}

	int changeFilestream(const std::string filename, std::ofstream **hdl, std::string *filenameBuffer)
	{
		return 0;
	}
};

} // namespace utils
} // namespace precitec

#endif // WMLOGSTREAMS_h__
