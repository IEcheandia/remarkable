/**
*
* @defgroup Fliplib
*
* @file
* @brief  This is a copy of Poco::Delegate with notify not being blocked when running in multiple threads.
* @copyright    Precitec GmbH & Co. KG
* @author GM
* @date   22.05.17
*
*/
#pragma once

#include <Poco/AbstractDelegate.h>
#include "fliplib/Fliplib.h"

using Poco::AbstractDelegate;
using Poco::Mutex;

namespace fliplib
{

template <class TObj, class TArgs, bool withSender = true> 
class FLIPLIB_API BaseDelegate: public AbstractDelegate<TArgs>
{
public:
	typedef void (TObj::*NotifyMethod)(const void*, TArgs&);

	BaseDelegate(TObj* obj, NotifyMethod method):
		_receiverObject(obj), 
		_receiverMethod(method)
	{
	}

	BaseDelegate(const BaseDelegate& delegate):
		AbstractDelegate<TArgs>(delegate),
		_receiverObject(delegate._receiverObject),
		_receiverMethod(delegate._receiverMethod)
	{
	}

	~BaseDelegate()
	{
	}
	
	BaseDelegate& operator = (const BaseDelegate& delegate)
	{
		if (&delegate != this)
		{
			this->_receiverObject = delegate._receiverObject;
			this->_receiverMethod = delegate._receiverMethod;
		}
		return *this;
	}

	bool notify(const void* sender, TArgs& arguments)
	{
        TObj *receiver = nullptr;
        {
            Mutex::ScopedLock lock(_mutex);
            receiver = _receiverObject;
        }
        if (receiver)
        {
            (receiver->*_receiverMethod)(sender, arguments);
            return true;
        } else
        {
            return false;
        }
	}

	bool equals(const AbstractDelegate<TArgs>& other) const
	{
		const BaseDelegate* pOtherDelegate = reinterpret_cast<const BaseDelegate*>(other.unwrap());
		return pOtherDelegate && _receiverObject == pOtherDelegate->_receiverObject && _receiverMethod == pOtherDelegate->_receiverMethod;
	}

	AbstractDelegate<TArgs>* clone() const
	{
		return new BaseDelegate(*this);
	}
	
	void disable()
	{
		Mutex::ScopedLock lock(_mutex);
		_receiverObject = 0;
	}

protected:
	TObj*        _receiverObject;
	NotifyMethod _receiverMethod;
	Mutex        _mutex;

private:
	BaseDelegate();
};

}
