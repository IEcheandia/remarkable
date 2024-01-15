#ifndef IMAGE_SHMEM_H_
#define IMAGE_SHMEM_H_
#pragma once

/**
 * Interfaces::imageShMem.h
 *
 *  Created on: 15.07.2010
 *      Author: WoX
 *   Copyright: Precitec Vision KG
 */

#include "common/systemConfiguration.h"
#include "system/types.h"
#include <functional>

namespace precitec
{
namespace greyImage
{
	// die Bild-Konstanten, die fuer die Groesse des SharedMem wesentlich sind stehen hier.
	// Diese Loesung ist weder ideal noch zwingend.
	// Die Groesse des ShMEm kann ueber den File-Descriptor eremittelt werden.
	// Die bisherigen Konstanten koennen dann in einem Header-Teil des ShMem untergebracht werden.
	// der Siso-Bibliothek darf dann natuerlich nur der Speicher ohne Header uebergeben werden.
	// ggf. muss der Header wg DMA Page-Groesse haben
	const PvString	ImageShMemName  	= "ImageShMem";
	const int	    ImageShMemHandle	= 3;
	const int	  	NumImageBuffers		= 512;
	const int	  	ImageWidth			= 512;
	const int	  	ImageHeight			= 512;
	const int	  	ImageSize			= ((ImageWidth * ImageHeight + 4095)) & 0xfffff000; // auf Pagegroesse aufrunden
	const int		ImageShMemSize		= NumImageBuffers * ImageSize;
    const int FactorGige = 10;

    /**
     * @returns name for the shared memory segment including the station name
     **/
    inline std::string sharedMemoryName()
    {
        char *stationName = getenv("WM_STATION_NAME");
        return ImageShMemName + (stationName ? std::string(stationName) : std::string());
    }

    inline int sharedMemoryHandle()
    {
        char *stationName = getenv("WM_STATION_NAME");
        return ImageShMemHandle + (stationName ? std::hash<std::string>{}(std::string(stationName)) : 0);
    }

    inline int sharedMemorySize(bool simulation = false)
    {
        if (simulation ||
            interface::SystemConfiguration::instance().getInt("CameraInterfaceType", 0) == 0 ||
            !interface::SystemConfiguration::instance().getBool("HasCamera", true))
        {
            return ImageShMemSize;
        }
        return ImageShMemSize * FactorGige;
    }

} // namespace test
} // namespace precitec

#endif // IMAGE_SHMEM_H_
