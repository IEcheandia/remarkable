#pragma once

#include <string>

namespace precitec
{
namespace system
{
/**
 * @brief Represents an in-memory file, created by memfd_create
 *
 * Creates a shared-memory segment which can be addressed by a file descriptor.
 */
class MemoryFile
{
public:
    /**
     * @brief Creates an object which does not refer to an in-memory file.
     */
    MemoryFile();

    /**
     * @brief Creates an object which refers to an in-memory file with the given name.
     *
     * The filename is just a label for debugging purposes, there is no such file in
     * the filesystem and a file descriptor must be used to address the memory.
     */
    MemoryFile(const std::string& filename);

    /**
     * @brief Closes the in-memory file.
     */
    ~MemoryFile();

    /**
     * @brief The class must not be copyable or moveable since it manages a unique resource.
     *
     * The memory segment is closed in the destructor, so allowing copy or move could cause
     * unwanted release of the memory.
     */
    MemoryFile(const MemoryFile&) = delete;
    MemoryFile& operator=(const MemoryFile&) = delete;
    MemoryFile(MemoryFile&&) = delete;
    MemoryFile& operator=(MemoryFile&&) = delete;

    /**
     * @brief Gets the file descriptor.
     * @returns an integer representing the file descriptor.
     */
    int descriptor() const
    {
        return m_fileDescriptor;
    }

    /**
     * @brief Checks if the object has a valid file descriptor.
     * @returns true if there is a file descriptor, false otherwise.
     */
    bool hasFileDescriptor() const
    {
        return m_fileDescriptor != -1;
    }

    /**
     * @brief Gets the path in the filesystem under which the filedescriptor is accessible.
     * @returns a path to the filedescriptor.
     */
    std::string filepath() const
    {
        return "/proc/self/fd/" + std::to_string(m_fileDescriptor);
    }

    int setSize(std::size_t size);
    int write(const std::string& line);
    std::pair<std::string, int> read(std::size_t size);
    long seekStart();

private:
    int m_fileDescriptor{-1};
};

inline bool operator==(const MemoryFile& first, const MemoryFile& second)
{
    return first.descriptor() == second.descriptor();
}

inline bool operator!=(const MemoryFile& first, const MemoryFile& second)
{
    return !(first == second);
}

}
}