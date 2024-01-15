/*
 * laserlineResults.cpp
 *
 *  Created on: 10.09.2010
 *      Author: KiH, HS
 */


#include <utility>
#include <stdlib.h>
#include <iostream>



#include "laserlineResults.h"



LaserlineResultT::LaserlineResultT()
:m_oNumberAllocated(0),
 m_oNumberCalled(0),
 m_oNumberFreed(0)
{
	nalloced=0;
	Y=NULL;
	I=NULL;
	firstValidIndex = -1;
	lastValidIndex = -1;
	laserLineStartPos.x = -1;
	laserLineEndPos.y = -1;

	bIsValid=false;
}

LaserlineResultT::~LaserlineResultT()
{

}

void LaserlineResultT::swap(LaserlineResultT &rhs) {
	std::swap(nalloced, rhs.nalloced);
	std::swap(Y, rhs.Y);
	std::swap(I, rhs.I);
	std::swap(firstValidIndex, rhs.firstValidIndex);
	std::swap(lastValidIndex, rhs.lastValidIndex);
	std::swap(bIsValid, rhs.bIsValid);
}


int LaserlineResultT::allocLaserLine(int nnelements)
{

	if (m_oNumberAllocated  == static_cast<unsigned int>(nnelements)) 
		return 0;

	// es wurden schon elemente allokiert
	//--> Groesse aendern
	if(m_oNumberAllocated > 0)
	{
		m_pY.reset(new int[nnelements]);
		m_pI.reset(new int[nnelements]);
		}
	else
	{

		m_pY =  std::unique_ptr<int[]>(new int[nnelements]);
		m_pI =  std::unique_ptr<int[]>(new int[nnelements]);
	}


	Y = m_pY.get();
	I = m_pI.get();

	if( (Y==NULL)||(I==NULL))
	{
		m_oNumberAllocated = 0;
		return 0;
	}

	m_oNumberCalled++;
	m_oNumberAllocated = nnelements;
	nalloced = nnelements;
	
	return(nnelements);
}

int LaserlineResultT::getAllocated()
{
	int dummy = static_cast<unsigned int>(m_oNumberAllocated);
	return(dummy);
}


void LaserlineResultT::freeLaserLine()
{


	if(m_oNumberAllocated > 0)
	{
		if(Y!=NULL)
		{
			//std::cout<<"Y: "<<Y<<"m_pY: "<<m_pY.get()<<" "<<m_oNumberCalled<<" "<<m_oNumberFreed<<std::endl;
			Y=NULL;
		}
		if(I!=NULL)
		{
			I=NULL;
		}
		m_oNumberFreed++;
	}
	m_oNumberAllocated=0;
	nalloced = 0;
}
