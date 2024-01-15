/**
 * Analyzer_Interface::sharedMem.cpp
 *
 *  Created on: 01.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 */

#if defined __QNX__ || defined __linux__

#include <sys/mman.h> // Mem manager: mmap
#include <errno.h>
#include <fcntl.h>
#include <unistd.h> // lseek, ...

#include <iostream>
#include <string.h> // wg strerror

#include "system/types.h"
#include "system/templates.h"
#include "system/sharedMem.h"

#include "module/moduleLogger.h"

namespace precitec
{

namespace system
{
#ifndef NOFD
#define NOFD (-1)
#endif

SharedMemControl& SharedMemControl::instance()
{
	static SharedMemControl sharedMemControl;
	return sharedMemControl;
}

	/**
	 * mehr als Platzvorhalten in den Arrays ist nicht noetig
	 */
	SharedMemControl::SharedMemControl() {
		memoryList_.reserve(10);
	}

	int SharedMemControl::findHandle(PvString name) const {
        auto it = std::find_if(memoryList_.begin(), memoryList_.end(), [name] (const ShMemEntry &entry) { return entry.name == name; });
        if (it != memoryList_.end())
        {
            return it->handle;
        }
		return NoHandle;
	}

    PvString SharedMemControl::name(int handle) const
    {
        auto it = std::find_if(memoryList_.begin(), memoryList_.end(), [handle] (const ShMemEntry &entry) { return entry.handle == handle; });
        if (it != memoryList_.end())
        {
            return it->name;
        }
        return std::string{};
    }

//
//	bool SharedMemControl::registerMemory(int handle, int size, PhysMem &mem) {
//		if (isUsed(handle)) return false; // bereits registriert
//		memoryList_[handle] = ShMemEntry(handle, size, mem);
//		nameList_[handle] 	= PvString("/__") + toString(int(mem.ptr()));
//		return true;
//	}
	/**
	 * fuer selbstverwaltetes SharedMem die Server-Registrierung:
	 *  das ShMem ist angelegt alles ist bekannt
	 * @param handle benutzerdefiniert: muss ueber globale Header mit Client gesynct sein
	 * @param name   benutzerdefiniert: muss ueber globale Header mit Client gesynct sein
	 * @param size	 benutzerdefiniert: muss ueber globale Header mit Client gesynct sein
	 * @param mem		 lokal, hierauf greifen die ShMemPointer zu
	 * @return true falls ShMem noc nicht registriert wurde
	 */
	int SharedMemControl::registerMemory(int handle, PvString name, SharedMem &mem, int size) {
		if (isUsed(handle)) return handle; // bereits registriert
		if (findHandle(name)!= NoHandle) return handle; // NoHandle;
		memoryList_.emplace_back(handle, size, mem, name);

		wmLog( eDebug, "registered: %s with %i\n", name.c_str(), handle );
		return handle;
	}


	SharedMem& SharedMemControl::operator [] (int handle) const {
        auto it = std::find_if(memoryList_.begin(), memoryList_.end(), [handle] (const ShMemEntry &entry) { return entry.handle == handle; });
        // FIXME: safety check
        return *(it->memory);
	}

	bool SharedMemControl::isUsed(int handle) const {
        auto it = std::find_if(memoryList_.begin(), memoryList_.end(), [handle] (const ShMemEntry &entry) { return entry.handle == handle; });
        return it != memoryList_.end() && it->memory != nullptr;
	}










	/**
	 * nicht initialisiertes Shared Mem fuer Arrays etwa
	 */
	SharedMem::SharedMem()
		:	name_(),
			fd_(NOFD),
			handle_(SharedMemControl::NoHandle),
			basePtr_(NULL),
			size_(0) {
	}


	/**
	 * unverwaltetes Shared Mem mit Speicher
	 * Dies kann sinnvoll sein, wenn ein SharedMem nur in einer gekapselten
	 * Struktur ohne externen Zugriff erfolgt.
	 */
	SharedMem::SharedMem(PvString const& fileName, ShMemMode mode, int size)
		: name_("/"+fileName),
			fd_(open(name_, mode)),
			basePtr_(map(size, mode)),
			size_(size) {
		//std::cout << "SharedMem::SharedMem0(" << fileName <<", "<< mode << ", " <<size<<"):" << std::endl;
		if (fileName.empty()) {
			std::cout << "SharedMem::SharedMem0(fileName, mode, size): name is empty" << std::endl;
		}
    if( fd_ == -1 ) { handleError("ShMem openfailed: ", errno);   }
		if (long(basePtr_)<0) { handleError("memory mapping failed: ", errno); }
		handle_ = -1;
	}

	/**
	 * handverwaltetes Shared Mem mit Speicher
	 */
	SharedMem::SharedMem(int handle, PvString const& fileName, ShMemMode mode, int size)
		: name_("/"+fileName),
			fd_(open(name_, mode)),
			basePtr_(map(size, mode)),
			size_(size) {
		//std::cout << "SharedMem::SharedMem0(" << handle << ", " << fileName <<", "<< mode << ", " <<size<<"):" << std::endl;
		if (fileName.empty()) {
			std::cout << "SharedMem::SharedMem1(handle, fileName, mode, size): name is empty" << std::endl;
		}
    if( fd_ == -1 ) { handleError("ShMem openfailed: ", errno);   }
		if (long(basePtr_)<0) { handleError("memory mapping failed: ", errno); }
		handle_ = SharedMemControl::instance().registerMemory(handle, name_, *this, size);
	}

	/**
	 * handverwaltetes Shared Mem
	 *  wir registrieren, legen aber nicht an
	 */
	SharedMem::SharedMem(int handle, PvString const& fileName)
		: name_("/"+fileName),
			fd_(NOFD),
			basePtr_(NULL),
			size_(0) {
		if (fileName.empty()) {
			std::cout << "SharedMem::SharedMem2(handle, filename): name is empty" << std::endl;
		}
		// wir registrieren, legen aber nicht an
		handle_ = SharedMemControl::instance().registerMemory(handle, name_, *this, 0);
	}

	/**
	 * Verzoegertes Oeffnen eines breits registrierten ShMems.
	 * Dies erlaubt zweistufige Initialisierung. SharedMems koennen
	 * bereits angelegt, ShMemPtr daraus erzeugt werden, nur der
	 * Zugriff darauf ist verboten.
	 */
	void SharedMem::set(ShMemMode mode, int size) {
		size_ = size;
		if (SharedMemControl::instance().name(handle_).empty()) {
			std::cout << "SharedMem::set0(mode, size): name is empty" << std::endl;
		}
		fd_ = open(SharedMemControl::instance().name(handle_), mode);
    if( fd_ == -1 ) { handleError("ShMem openfailed: ", errno); return;  }
		basePtr_ = map(size, mode);
		if (long(basePtr_)<0) { handleError("memory mapping failed: ", errno); }
	}

	/// unverwaltetes ShMem; kann nicht mit ShMemPtr verwendet werden!!
	void SharedMem::set(PvString const& fileName, ShMemMode mode, int size) {
		if (fileName.empty()) {
			std::cout << "SharedMem::set1(fileName, mode, size): name is empty" << std::endl;
		}
		name_ = "/" + fileName;
		//wmLog( eDebug, "SharedMem::set: %s - %i\n", fileName.c_str(), size );
		size_ = size;
		fd_ = open(name_, mode);
    if( fd_ == -1 ) { handleError("ShMem openfailed: ", errno); return;  }
		basePtr_ = map(size, mode);
		if (long(basePtr_)<0) { handleError("memory mapping failed: ", errno); }
//		std::stringstream oSt; oSt << " ---> " << *this << std::endl;
//		wmLog( eDebug, oSt.str() );
	}

	/**
	 * Verzoegertes Oeffnen eines noch nicht registrierten ShMem
	 * @param index Hiermit wird das ShMem von ShMemPtr aus angesprochen
	 * @param fileName
	 * @param mode
	 * @param size
	 */
	void SharedMem::set(int handle, PvString const& fileName, ShMemMode mode, int size) {
		if (fileName.empty()) {
			std::cout << "SharedMem::set2(handle, fileName, mode, size): name is empty" << std::endl;
		}
		name_ = "/" + fileName;
		fd_ = open(name_, mode);
		if( fd_ == -1 ) { handleError("ShMem openfailed: ", errno); }
		basePtr_ = map(size, mode);
		size_ = size;
		if (long(basePtr_)<0) { handleError("memory mapping failed: ", errno); }
		handle_ = SharedMemControl::instance().registerMemory(handle, name_, *this, size);
	}

	/// \todo ueber das virtual muss man noch reden
	SharedMem::~SharedMem() {
		std::cout << "SharedMem::~SharedMem()   " << *this << std::endl;
		if (fd_!=NOFD)	close(fd_);
		std::cout << "SharedMem::~SharedMem()   closed file descriptor" << fd_ << std::endl;
		//shm_unlink(_name.c_str());
	}

	PVoid SharedMem::map(int size, ShMemMode mode) {
		void* address  		= 0; // we don't sugggest anything to the system
		off64_t offset 			= 0; // offset from start of file
		int protection;
		int createFlags ;
		if (size==0) {
			// cursor an's Ende der Datei
			lseek( fd_, 0,SEEK_END);
			// gibt die Position aus
#ifdef __QNX__
			size = tell(fd_);
#elif defined(__linux__)
			size = lseek(fd_, 0, SEEK_CUR);
#endif
		}
		if (ftruncate(fd_, size) == -1)	{
			//throw SystemException("Cannot resize shared memory object", _name);
		}
		if (fd_==NOFD) {
			// PhysMem--mapping
#if defined(__QNX__)
			protection	= PROT_READ | PROT_WRITE | PROT_NOCACHE;
			createFlags = MAP_PHYS
										| MAP_ANON 			// Unix-Extention: kein fd
										| MAP_NOINIT 		// keine 0-Initialisierung: klappt vermutich nicht
										| MAP_SHARED; 	// Zugriff von mehreren Prozessen
#elif defined(__linux__)
			protection	= PROT_READ | PROT_WRITE;
			createFlags = MAP_ANON 			// Unix-Extention: kein fd
							| MAP_SHARED; 	// Zugriff von mehreren Prozessen
#endif
		} else {
			// file-mapping
			protection	= PROT_READ | PROT_WRITE ;
			createFlags = MAP_SHARED; 		// Zugriff von mehreren Prozessen
			if (mode & Locked)
            {
                createFlags |= MAP_LOCKED;
            }
		}
		//return mmap( address,	size, protection, createFlags, fd_, offset);
		return mmap64( address,	size, protection, createFlags, fd_, offset);
	}

		/**
		 * \todo ggf. size_ ggf size aus fd (lseek) testen
		 * @param fileName
		 * @param mode
		 * @return
		 */
	int SharedMem::open(PvString const& fileName, ShMemMode mode) {
		//std::cout << "SharedMem::open: " << fileName.c_str() << std::endl;
		int openFlags = createNew(mode) ? O_RDWR  |  O_CREAT : O_RDWR;
		int accessFlags = writeAccess(mode) ? S_IWUSR | S_IRUSR : S_IRUSR;
		if (recreateOld(mode)) {
			// vorhandenes SharedMem loeschen
			if (shm_open( fileName.c_str(), openFlags, accessFlags) !=NOFD) {
				shm_unlink(fileName.c_str()); // ???? close ??
			}
		}
		return shm_open( fileName.c_str(), openFlags, accessFlags);
	}

	void SharedMem::handleError(PvString const& baseText, int error) const {
    std::cerr <<  baseText <<  strerror( error );
	}

} // namespace system
} // namespace precitec


#endif


