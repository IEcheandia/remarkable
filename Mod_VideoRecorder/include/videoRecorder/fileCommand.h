/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		File commands which can be executed by a command processor.
 */

#ifndef FILECOMMAND_H_20121030_INCLUDE
#define FILECOMMAND_H_20121030_INCLUDE

// local includes
#include "videoRecorder/baseCommand.h"
#include "videoRecorder/parameter.h"
#include "videoRecorder/literal.h"
#include "videoRecorder/types.h"
#include "message/grabberStatus.interface.h"
#include "event/schedulerEvents.proxy.h"
#include "event/videoRecorder.h"
// Poco includes
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/XMLConfiguration.h"
// stl includes
#include <string>
#include <atomic>


namespace precitec {
namespace vdr {

/**
 * @brief Functor class to add an entry to a cache file and delete old entries from the cache.
 * This functor is used by the ProcessRecordCmd
 **/
class CachePath
{
public:
    /**
     * @param cacheFilePath The path to the cache file
     * @param pathToAdd The path to add to the cache file
     * @param entriesToKeep Number of entries to keep in the cache file, if the cache becomes larger older entries are deleted
     * @param pathToDeleteMustContain Any entry in the cache must contain this substring to get deleted
     **/
    CachePath(const std::string &cacheFilePath, const std::string &pathToAdd, std::size_t entriesToKeep, const std::string &pathToDeleteMustContain);

    void operator()();

private:
    std::string m_oCacheFilePath;
    std::string m_pathToAdd;
    std::size_t m_oNbEntriesToKeep;
    std::string m_pathToDeleteMustContain;
};

/**
 * @brief	File command. Base class for all concrete commands that work on a file.
 */
class FileCommand {
public:
	/**
	 * @brief	CTOR
	 * @param	p_rFile					File path to be processed.
	 */
	FileCommand(const Poco::File& p_rFile) : m_oFile(p_rFile) {}
protected:
	virtual ~FileCommand() {} // prohibits instantiation out of child classes
	mutable Poco::File 			m_oFile;		///< command argument
}; // FileCommand



/**
 * @brief	Concrete command. Deletes files or folders recursively.
 */
class DeleteRecursivelyCmd : public BaseCommand, private FileCommand {
public:
	/**
	 * @brief	CTOR
	 * @param	p_rFile					File path to be processed.
	 */
	explicit DeleteRecursivelyCmd(const Poco::File& p_rFile) : FileCommand(p_rFile) {}
	/*virtual*/ void execute();
}; // DeleteRecursivelyCmd


/**
 * @brief	Concrete command. Creates file and writes configuration to file.
 */
class WriteConfCmd : public BaseCommand, public FileCommand {
public:
	/**
	 * @brief	CTOR
	 * @param	p_rFile					File path to be processed.
	 * @param	p_rProductInstData		Product data to be written into configuration.
	 * @param	p_rSeamData				Seam data to be written into configuration.
	 * @param	p_rRecordCounters		Recording counters, up to date at time of writing.
	 */
	WriteConfCmd(	const Poco::File& 					p_rFile,
					const interface::ProductInstData& 	p_rProductInstData,
					const interface::SeamData& 			p_rSeamData,
					const Counters& 					p_rRecordCounters)
		:
		FileCommand			( p_rFile ),
		m_oProductInstData	( p_rProductInstData ),
		m_oSeamData			( p_rSeamData ),
		m_rRecordCounters	( p_rRecordCounters ) {
	} // WriteConfCmd

	/*virtual*/ void execute();

private:	
	interface::ProductInstData 		m_oProductInstData;		///< command argument
	interface::SeamData 			m_oSeamData;			///< command argument
	const Counters& 				m_rRecordCounters;		///< command argument
}; // WriteConfCmd



/**
 * @brief	Concrete command. Creates a file, directory must exist.
 */
class CreateFileCmd : public BaseCommand, public FileCommand {
public:
	/**
	 * @brief	CTOR
	 * @param	p_rFile					File path to be processed.
	 */
	explicit CreateFileCmd(const Poco::File& p_rFile) : FileCommand(p_rFile) {}
	/*virtual*/ void execute();
}; // CreateDirsCmd



/**
 * @brief	Concrete command. Creates a directory and all parent directories if necessary.
 */
class CreateDirsCmd : public BaseCommand, public FileCommand {
public:
	/**
	 * @brief	CTOR
	 * @param	p_rFile					File path to be processed.
	 */
	explicit CreateDirsCmd(const Poco::File& p_rFile) : FileCommand(p_rFile) {}
	/*virtual*/ void execute();
}; // CreateDirsCmd



/**
 * @brief	Concrete command. Renames a file to the the given name. Only works with files, not with directories.
 */
class RenameCmd : public BaseCommand, public FileCommand {
public:
	/**
	 * @brief	CTOR
	 * @param	p_rNewFilePath				Name path the file is renamed to.
	 */
	explicit RenameCmd(const Poco::File& p_rFile, const Poco::Path& p_rNewFilePath) :
		FileCommand		( p_rFile ),
		m_oNewFilePath	( p_rNewFilePath )
	{}
	/*virtual*/ void execute();
private:
	const Poco::Path			m_oNewFilePath;		///< command argument
}; // RenameCmd



/**
 * @brief	Concrete command. Moves a directory / file to the given destination.
 * 			The destination folder and all subfolders are created if not existing.
 */
class MoveUnixCmd : public BaseCommand, public FileCommand {
public:
	/**
	 * @brief	CTOR
	 * @param	p_rDestination				Destination path the file is copied to.
	 */
	MoveUnixCmd(const Poco::File& p_rFile, const Poco::Path& p_rDestination) :
		FileCommand		( p_rFile ),
		m_oDestination	( p_rDestination )
	{}
	/*virtual*/ void execute();
private:
	const Poco::Path			m_oDestination;		///< command argument
}; // MoveUnixCmd



/**
 * @brief	Concrete command. Writes an image to disk. 
 * @details	Performs a disk usage check and asks the grabber if the image number is still valid in memory.
 */
class WriteImageCmd : public BaseCommand, public FileCommand {
public:
	typedef	interface::TGrabberStatus<interface::AbstractInterface>&	grabberStatusInterface_t;

	/**
	 * @brief	CTOR
	 * @param	p_rDestination				Destination path the file is copied to.
	 */
	WriteImageCmd(
		const Poco::File&			p_rFile, 
		const vdrImage_t&			m_oVdrImage,
		Parameter&					p_rParameter,
		grabberStatusInterface_t&	p_rGrabberStatusProxy,
		Counters&					p_rCounters,
		std::atomic<bool>&			p_rIsRecordInterrupted)
		:
		FileCommand				( p_rFile ),
		m_oVdrImage				( m_oVdrImage ),
		m_rParameter			( p_rParameter ),
		m_rGrabberStatusProxy	( p_rGrabberStatusProxy ),
		m_rCounters				( p_rCounters ),
		m_rIsRecordInterrupted	( p_rIsRecordInterrupted )
	{}
	/*virtual*/ void execute();
private:
	const vdrImage_t				m_oVdrImage;				///< command argument
	Parameter&						m_rParameter;				///< command argument
	grabberStatusInterface_t&		m_rGrabberStatusProxy;		///< command argument
	Counters&						m_rCounters;				///< command argument
	std::atomic<bool>&				m_rIsRecordInterrupted;		///< command argument
}; // WriteImageCmd



/**
 * @brief	Concrete command. Writes a sample to disk.
 */
class WriteSampleCmd : public BaseCommand, public FileCommand {
public:
	typedef	interface::TGrabberStatus<interface::AbstractInterface>&	grabberStatusInterface_t;

	/**
	 * @brief	CTOR
	 * @param	p_rDestination				Destination path the file is copied to.
	 */
	WriteSampleCmd(
		const Poco::File&			p_rFile,
		const vdrSample_t&			m_oVdrSample,
		Parameter&					p_rParameter,
		Counters&					p_rCounters,
		std::atomic<bool>&			p_rIsRecordInterrupted)
		:
		FileCommand				( p_rFile ),
		m_oVdrSample			( m_oVdrSample ),
		m_rParameter			( p_rParameter ),
		m_rCounters				( p_rCounters ),
		m_rIsRecordInterrupted	( p_rIsRecordInterrupted )
	{}
	/*virtual*/ void execute();
private:

	const vdrSample_t				m_oVdrSample;				///< command argument
	Parameter&						m_rParameter;				///< command argument
	Counters&						m_rCounters;				///< command argument
	std::atomic<bool>&				m_rIsRecordInterrupted;		///< command argument
}; // WriteSampleCmd



/**
 * @brief	Concrete command. Processes a recording cycle.
 * @details	Deletes precreated empty folders, moves live mode sequence, updates the auto delete cache.
 */
class ProcessRecordCmd : public BaseCommand, public FileCommand {
public:
	typedef	interface::TSchedulerEvents<interface::AbstractInterface>&	schedulerEventsInterface_t;

	/**
	 * @brief	CTOR
	 * @param	p_rFile					File path to be processed.
	 * @param	p_rIsLiveMode			If live or automatic cycle.
	 * @param	p_rProductDirLiveFinal	Final live mode directory.
	 * @param	p_rCacheFilePath		File path to cache file.
	 * @param	p_oNbEntriesToKeep		Number of cache entries to keep.
	 */
	ProcessRecordCmd(
		const Poco::File&			p_rFile,
		bool						p_oIsLiveMode,
		const std::string&			p_rProductDirLiveFinal,
		const std::string& 			p_rCacheFilePath,
		std::size_t 				p_oNbEntriesToKeep,
		schedulerEventsInterface_t& p_rSchedulerEventsProxy)
		:
		FileCommand				( p_rFile ),
		m_oIsLiveMode			( p_oIsLiveMode ),
		m_oProductDirLiveFinal	( p_rProductDirLiveFinal ),
		m_oCacheFilePath		( p_rCacheFilePath ),
		m_oNbEntriesToKeep		( p_oNbEntriesToKeep ),
		m_rSchedulerEventsProxy	( p_rSchedulerEventsProxy )
	{}
	/*virtual*/ void execute();

private:

	/**
	 * @brief	Deletes all empty seam folders. Also deletes series and product folder if all seam folders within empty.
	 * @return	bool				If all seam folders were deleted.
	 */
	bool deleteEmptyFolders() const;

	/**
	 * @brief	Moves live mode record to final product folder.
	 * @return	void
	 */
	void moveLiveModeRecord() const;

	/**
	 * @brief	Updates the cache file and performs cyclic auto deletion.
	 */
	void cachePath() const;


	const bool					m_oIsLiveMode;				///< command argument
	const std::string			m_oProductDirLive;			///< command argument
	const std::string			m_oProductDirLiveFinal;		///< command argument
	const std::string			m_oCacheFilePath;			///< command argument
	std::size_t					m_oNbEntriesToKeep;			///< command argument
	schedulerEventsInterface_t&	m_rSchedulerEventsProxy;	///< command argument
}; // ProcessRecordCmd



/**
 * @brief	Concrete command.
 * @details	Updates the cache file and performs cyclic auto deletion.
 */
class UpdateCacheCmd : public BaseCommand
{
public:
	/**
	 * @brief	CTOR
	 * @param	cacheFilePath			Path to cache file
	 * @param	p_oNbEntriesToKeep		Number of cache entries to keep.
     * @param pathMustContain The paths stored in cache file must contain this string to get deleted
	 */
	UpdateCacheCmd(const std::string &cacheFilePath, std::size_t p_oNbEntriesToKeep, const std::string &pathMustContain);

	/*virtual*/ void execute();

	const std::string			m_cacheFilePath;
	std::size_t					m_oNbEntriesToKeep;			///< command argument

private:
    std::string m_pathMustContain;
}; // UpdateCacheCmd

/**
 * Command deleting the product instance directory and removing it from cache.
 **/
class DeleteProductInstanceCmd : public BaseCommand
{
public:
    /**
     * @param instancePath The product instance to delete
     * @param cacheFilePath The path to the cache file
     * @param pathMustContain The paths stored in cache file must contain this string to get deleted
     **/
    DeleteProductInstanceCmd(const std::string &instancePath, const std::string &cacheFilePath, const std::string &pathMustContain);

    void execute() override;

private:
    std::string m_instancePath;
    std::string m_cacheFilePath;
    std::string m_pathMustContain;
};

/// free functions

/**
 * @brief	Deletes a recorded instance. Performs some safety checks first.
 * @param pathMustContain the path @p p_rInstanceToDelete must contain this parameter
 * @return	bool				If instance was deleted.
 */
bool checkedDeleteRecordedInstance(const Poco::Path& p_rInstanceToDelete, const std::string &pathMustContain);


} // namespace vdr
} // namespace precitec

#endif // FILECOMMAND_H_20121030_INCLUDE
