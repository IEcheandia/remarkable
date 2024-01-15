///////////////////////////////////////////////////////////
//  PipeEventArgs.cpp
//  Implementation of the Class PipeEventArgs
//  Created on:      30-Okt-2007 14:29:47
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include "fliplib/PipeEventArgs.h"
#include "fliplib/PipeGroupEvent.h"

using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::BasePipe;

/////////////////////////////////////////////////////////////////////////////////
// PipeEventArgs
////////////////////////////////////////////////////////////////////////////////

PipeEventArgs::PipeEventArgs(BasePipe* pipe, int p_oImgNb) :
	pipe_       (pipe),
    m_oImgNb    (p_oImgNb)
{
}



PipeEventArgs::~PipeEventArgs()
{
	pipe_ = NULL;
}

//bool PipeEventArgs::isValid() const
//{		
//	return pipe_->isValid();
//}

fliplib::BasePipe* PipeEventArgs::pipe() const
{

	return pipe_;
}


/////////////////////////////////////////////////////////////////////////////////
// PipeGroupEventArgs
////////////////////////////////////////////////////////////////////////////////

fliplib::PipeGroupEventArgs::PipeGroupEventArgs(int count, PipeGroupEvent* parent, int p_oImgNb) :
	count_(count), parent_(parent), m_oImgNb(p_oImgNb)		
{
}

PipeGroupEventArgs::~PipeGroupEventArgs()
{
	count_ = 0;
	parent_ = NULL;
}

bool PipeGroupEventArgs::isValid() const
{		
	return true;
}

fliplib::BasePipe* PipeGroupEventArgs::pipe(const std::string& name)
{
	return parent_->pipe(name);
}

fliplib::BasePipe* PipeGroupEventArgs::pipe(const std::type_info& type)
{
	return parent_->pipe(type);
}

int PipeGroupEventArgs::count() const
{		
	return parent_->count();
}

fliplib::PipeGroupEvent* PipeGroupEventArgs::group() const
{		
	return parent_;
}
