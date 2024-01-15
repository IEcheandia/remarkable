#ifndef INTERFACES_H_
#define INTERFACES_H_

#if !defined(INTERFACES_API)
	#if defined(_WIN32)
		#if defined(INTERFACES_EXPORTS)
			#define INTERFACES_API __declspec(dllexport)
		#else
			#define INTERFACES_API __declspec(dllimport)
		#endif
	#endif
#endif

#if !defined(INTERFACES_API)
	#define INTERFACES_API
#endif

#endif /*INTERFACES_H_*/
