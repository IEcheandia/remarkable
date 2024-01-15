/**
*
* @defgroup testing
*
* @file
* @brief  Header file for some unit test helping
* @copyright    Precitec GmbH & Co. KG
* @author GM
*/
#pragma once
// cppunit includes needed in all tests
#include "common/bitmap.h"
#include "image/image.h"
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <Poco/File.h>
#include <Poco/Path.h>

#include <cstring>
#include <string>

#define TEST_MAIN( CLASSNAME ) \
int main(int /*argc*/, char **/*argv*/) \
{ \
    CppUnit::TextUi::TestRunner runner; \
    runner.addTest(CLASSNAME::suite()); \
    return !runner.run(); \
}


/**
 * RAII to create a unique directory and delete on deconstruction
 **/
class TemporaryDirectory
{
public:
    TemporaryDirectory(const std::string &pattern);
    ~TemporaryDirectory();
    
    std::string path() const;
    
    auto createDirectory(const std::string &name)
    {
        Poco::Path dir(path(), name);
        return Poco::File(dir).createDirectory();
    }

    auto createFile(const std::string &name)
    {
        Poco::Path dir(path(), name);
        return Poco::File(dir).createFile();
    }

    auto createBitmap(int width, int height, int seed, const std::string& name, const std::vector<unsigned char>& additionalData = std::vector<unsigned char>())
    {
        bool success = createFile(name);
        Poco::Path dir(path(), name);
        Poco::File file(dir);
        auto image = precitec::image::genModuloPattern({width, height}, seed);
        fileio::Bitmap bitmap(file.path(), width, height);
        success &= bitmap.save(image.begin(), additionalData);
        return success;        
    };
private:
    Poco::Path m_directory;
};

inline TemporaryDirectory::TemporaryDirectory(const std::string &pattern)
{
    Poco::Path tmpDir{Poco::Path::temp()};
    tmpDir.pushDirectory(pattern + std::string("XXXXXX"));
    std::string unixPath = tmpDir.toString(Poco::Path::PATH_UNIX);
    // generated with trailing /
    // need to remove it
    unixPath.pop_back();
    // copy into a cString
    auto cPath = std::make_unique<char[]>(std::strlen(unixPath.c_str()) + 1);
    std::strcpy(cPath.get(), unixPath.c_str());
    m_directory = Poco::Path(std::string(mkdtemp(cPath.get())));
}

inline TemporaryDirectory::~TemporaryDirectory()
{
    Poco::File(m_directory).remove(true);
}

inline std::string TemporaryDirectory::path() const
{
    return m_directory.toString(Poco::Path::PATH_UNIX);
}