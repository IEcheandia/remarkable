#ifndef POLICIES_H_
#define POLICIES_H_


namespace precitec
{
namespace system
{

	/**
	 * Release-Klasse fuer Shared-Memory Smart-Pointer
	 */
	template <class T>
	class NoReleasePolicy	{
	public:
		static void release(T*) {}
	};

	/**
	 * Release-Klasse fuer Pointer auf Arrays
	 */
	template <class T>
	class ArrayReleasePolicy	{
	public:
		static void release(T* p) { delete []p;}
	};

	/**
	 * Release-Klasse fuer Shared-Memory Smart-Pointer auf Arrays
	 * die ist natuerlich gleich der normalen Shared-Mem. Policies, aber
	 * so kann man die resultierenden Pointer unterscheiden (wichtig fuer Serialisierung)
	 */
	template <class T>
	class ArrayNoReleasePolicy	{
	public:
		static void release(T*) {}
	};
}
}


#endif /*POLICIES_H_*/
