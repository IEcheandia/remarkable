///////////////////////////////////////////////////////////
//  NullPipeContent.h
//  Implementation of the Class NullPipeContent
//  Created on:      23-Okt-2015 
//  Original author: Duw
///////////////////////////////////////////////////////////

#ifndef _NULL_PIPE_CONTENT_H__
#define _NULL_PIPE_CONTENT_H__

class NullImageContext
{
public:
	NullImageContext(){};
	~NullImageContext(){};

	int imageNumber()const { return 0; }
};

class NullPipeContent
{
public:
	NullPipeContent(){};
	~NullPipeContent(){};

	NullImageContext context()const { return NullImageContext(); }
};

#endif