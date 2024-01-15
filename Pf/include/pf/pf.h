#ifndef PF_H
#define PF_H

#include <list>
#include <vector>
#include "camProperty.h"

namespace precitec
{
	enum PfCommand  { List, Get, Set, Execute, Nothing};
	
	typedef std::list<ip::pf::Handle>  	PropList;
	typedef std::vector<ip::pf::Handle>  PropArray;
	typedef PropList::const_iterator 	PropCIter;
	typedef PropList::iterator 			 	PropIter;
}

#endif // PF_H
