#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <stdexcept>
#include <string>

namespace precitec
{
namespace system
{

	/**
	 * The exception that is thrown when a requested method or operation is not implemented.
	 */
	class NotImplementedException : public std::runtime_error
	{
	public:
		NotImplementedException(const std::string& what) : std::runtime_error(what) {}
		NotImplementedException() : std::runtime_error("The exception that is thrown when a requested method or operation is not implemented.") {}
	};

	/**
	 * The exception that is thrown when an invoked method is not supported, or when there is an attempt to read,
	 * seek, or write to a stream that does not support the invoked functionality.
	 */
	class NotSupportedException  : public std::runtime_error
	{
	public:
		NotSupportedException (const std::string& what) : std::runtime_error(what) {}
		NotSupportedException () : std::runtime_error("The exception that is thrown when an invoked method is not supported, or when there is an attempt to read, seek, or write to a stream that does not support the invoked functionality.") {}
	};

/*
 * Exception for missing parameters for measureTasks
 */
	class MissingParameterException : public std::runtime_error
	{
	public:
		MissingParameterException(const std::string& what) : std::runtime_error(what) {}
		MissingParameterException() : std::runtime_error("MissingParameter exception thrown: At least one measure task lacks necessary parameters.") {}
	};

	/**
	 * The exception that is thrown for invalid casting or explicit conversion.
	 */
	class InvalidCastException  : public std::runtime_error
	{
	public:
		InvalidCastException (const std::string& what) : std::runtime_error(what) {}
		InvalidCastException () : std::runtime_error("The exception that is thrown for invalid casting or explicit conversion.") {}
	};

	/**
	 * The exception that is thrown for when an index is out of range.
	 */
	class OutOfRangeException  : public std::runtime_error
	{
	public:
		OutOfRangeException (const std::string& what) : std::runtime_error(std::runtime_error(what)) {}
		OutOfRangeException () : std::runtime_error("The exception that is thrown for when an index is out of range.") {}
	};

	/**
	 * The exception that is thrown when the time allotted for a process or operation has expired.
	 */
	class TimeoutException : public std::runtime_error
	{
	public:
		TimeoutException(const std::string& what) : std::runtime_error(what) {}
		TimeoutException() : std::runtime_error("The exception that is thrown when the time allotted for a process or operation has expired.") {}
	};

    /**
     * The exception is thrown by the ShMemRingAllocator in case of no free element.
     **/
    class AllocFailedException : public std::runtime_error
    {
    public:
        AllocFailedException(const std::string &interfaceName) : std::runtime_error(std::string{ "allocBlock() no free element for interface  " } + interfaceName + std::string{".\n"}) {}
    };

} // namespace system
} // namespace precitec

#endif /*EXCEPTION_H_*/
