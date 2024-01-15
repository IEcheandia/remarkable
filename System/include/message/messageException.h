#ifndef MESSAGEEXCEPTION_H_
#define MESSAGEEXCEPTION_H_

namespace precitec
{
namespace system
{
	namespace message
	{

		/**
		 * Basis Exception fuer Messages
		 * Ettliche Funktionen werfen diese eine Exception mit einem jeweils eigenen String.
		 */
		class MessageException : public std::exception
		{
		public:
			MessageException(const char* what) : what_(what) {}
			const char* what() const throw() { return what_; }
		private:
			const char* what_;
		};

		class TransferTerminatedException : public std::exception
		{
		public:
			TransferTerminatedException(const char* what) : what_(what) {}
			const char* what() const throw() { return what_; }
		private:
			const char* what_;
		};


	} // message
} // system
/*
namespace interface
{
	class HandlerException : public std::exception {
	public:
		virtual const char *what() const throw()  { return "couldnt find free handler"; }
	};
}
*/
} // precitec


#endif /*MESSAGEEXCEPTION_H_*/
