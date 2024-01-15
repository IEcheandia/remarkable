#ifndef IPIMAGE_HPP_
#define IPIMAGE_HPP_

#include "memory/container.h"
#include "image/ipSignal.h"
#include "image/ipImage.h"
#include "image/ipLineImage.h"

namespace precitec
{
namespace image
{

template <class ValueT>
TImage<ValueT> * TImage<ValueT>::create(int type, system::message::MessageBuffer const& buffer )
{
	typedef TImage* (*TImageFactory)(system::message::MessageBuffer const&buffer);
	/// Achtung!!!! Performance-Bug??!!???? Liste wird erst bei erstem Aufruf erzeugt
	TImageFactory imageFactoryList[TNumTypes] = {
		//TBlockImage<ValueT>::create,
		TLineImage<ValueT>::create
	};
	return imageFactoryList[type](buffer);
}

} // namespace image
} // namespace precitec



#endif /*IPIMAGE_HPP_*/
