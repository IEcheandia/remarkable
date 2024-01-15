#include "Poco/Exception.h"
#include "fliplib/Exception.h"
#include <typeinfo>

using Poco::Exception;

namespace fliplib {
	
#define FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(CLS, NAME) FLIPLIB_IMPLEMENT_EXCEPTION(CLS, Poco::CLS, NAME)
	
FLIPLIB_IMPLEMENT_EXCEPTION(GraphBuilderException, DataException, "FilterGraph builder exception")
FLIPLIB_IMPLEMENT_EXCEPTION(ParameterException, LogicException, "Filter parameter exception")

FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(LogicException, "Logic exception")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(AssertionViolationException, "Assertion violation")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(NullPointerException, "Null pointer")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(BugcheckException, "Bugcheck")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(InvalidArgumentException, "Invalid argument")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(NotImplementedException,"Not implemented")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(RangeException,"Out of range")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(IllegalStateException,"Illegal state")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(InvalidAccessException,"Invalid access")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(SignalException,"Signal received")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(UnhandledException,"Signal received")

FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(RuntimeException,"Runtime exception")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(NotFoundException,"Not found")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(ExistsException,"Exists")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(TimeoutException,"Timeout")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(SystemException,"System exception")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(RegularExpressionException,"Error in regular expression")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(LibraryLoadException,"Cannot load library")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(LibraryAlreadyLoadedException, "Library already loaded")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(NoThreadAvailableException,"No thread available")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(PropertyNotSupportedException,"Property not supported")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(PoolOverflowException,"Pool overflow")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(NoPermissionException,"No permission")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(OutOfMemoryException,"Out of memory")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(DataException,"Data error")

FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(DataFormatException, "Bad data format")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(SyntaxException, "Syntax error")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(CircularReferenceException, "Circular reference")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(PathSyntaxException, "Bad path syntax")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(IOException,"I/O error")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(FileException,  "File access error")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(FileExistsException,"File exists")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(FileNotFoundException,"File not found")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(PathNotFoundException,"Path not found")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(FileReadOnlyException,"File is read-only")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(FileAccessDeniedException,"Access to file denied")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(CreateFileException,"Cannot create file")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(OpenFileException,"Cannot open file")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(WriteFileException,"Cannot write file")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(ReadFileException,"Cannot read file")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(UnknownURISchemeException,"Unknown URI scheme")


FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(ApplicationException,"Application exception")
FLIPLIB_SUBLCASSING_IMPLEMENT_EXCEPTION(BadCastException,"Bad cast exception")

} // namespace Poco
