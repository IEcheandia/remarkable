/*
 * FilterLibrary.cpp
 *
 *  Created on: 07.12.2010
 *      Author: Administrator
 */

#include <string>
#include <iostream>
#include <sstream>

#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/SharedLibrary.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/ClassLoader.h"

#include "fliplib/Activator.h"
#include "fliplib/FilterLibrary.h"

using namespace Poco;
using namespace fliplib;

std::string FilterLibrary::collect (Poco::Path const& orgPath)
{
	Poco::Path path ( orgPath );
	if ( !path.isDirectory() )
		path.append ( std::string(1, Path::pathSeparator()) );

	DirectoryIterator dit ( path );
	DirectoryIterator end;
	ClassLoader<BaseFilterInterface> pluginLoader;	

	// Systemunabhaengiger Suffix ermitteln und Debug Info entfernen d.dll -> dll oder d.so wird so
	std::string suffix = Poco::SharedLibrary::suffix();
	std::string::size_type pos = suffix.rfind('.');
	if (pos != std::string::npos)
		suffix = suffix.substr(pos + 1);

	std::stringstream ss;

	while(dit != end)
	{
		// Nur oeffnen, wenn es eine Library ist
		if ( dit->isFile() && Path( dit->path() ).getExtension() == suffix)
		{
			try
			{
				// Manifest verbinden
				pluginLoader.loadLibrary( dit->path() );

				try
				{
					ClassLoader<BaseFilterInterface>::Iterator it( pluginLoader.begin());
					ClassLoader<BaseFilterInterface>::Iterator end( pluginLoader.end());

					for(; it != end; ++it)
					{
						ss << "<component assembly=\"" << it->first << "\" createDate=\""
						<< Poco::DateTimeFormatter::format(dit->created(), DateTimeFormat::ISO8601_FORMAT)
						<< "\">" ;

						std::cout << "libPath" << it->first << std::endl;
						Poco::Manifest<BaseFilterInterface>::Iterator itMan( it->second->begin() );
						Poco::Manifest<BaseFilterInterface>::Iterator endMan( it->second->end() );
						for(; itMan != endMan; ++itMan)
						{
							BaseFilterInterface* pInterface = pluginLoader.classFor( itMan->name() ).create();
							BaseFilter* pFilter = static_cast<BaseFilter*>(pInterface);
							ss << pFilter->toXml( itMan->name() ) << std::endl;
							pluginLoader.destroy( itMan->name(), pInterface );
						}
						ss << "</component>";
					}
				}
				catch(NotFoundException &ex)
				{
					std::cout << "[FilterLibrary::collect] " << ex.message() <<" path:" << dit->path() << std::endl;
				}

				pluginLoader.unloadLibrary( dit->path() );
			}
			catch(Poco::LibraryLoadException &ex)
			{
				std::cout << "[FilterLibrary::collect] cannot load library " << ex.message() << dit->path() << std::endl;
			}
		}

		++dit;
	}

	ss << std::endl;

	return ss.str();
}



