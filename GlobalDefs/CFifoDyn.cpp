/***************************************************************************/
/*                                                                         */
/*     C F I F O D Y N . C P P                                         	   */
/*                                                                         */
/*     Klasse fuer die Verwaltung eines dyn. Ringbuffer	                   */
/*                                                                         */
/***************************************************************************/



#include "CFifoDyn.h"




#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFifoDyn::CFifoDyn(){
	m_Is_Initialized=0;
	m_Last_Element	=-1;

	MESSAGE Msg;Msg.ID=0;// Eric 10.03.2008 09:21:52 Wegen Compilerwarnung
	m_HeaderSize = sizeof(Msg.ID) + sizeof(Msg.Length); // Sar 31.01.05 10:05:41
}


CFifoDyn::~CFifoDyn()
{
	if (m_Is_Initialized==0) return ;
	if (m_pWurzel!=NULL) delete []m_pWurzel;
}

bool CFifoDyn::Create(int Size,int Item_Size)
{//Allokiert den Speicher mit "Size" Bytes Groesse
 	if ((m_pWurzel=new char[Size])==NULL) return 0;

	memset(m_pWurzel,0,Size);

    m_Is_Initialized=1;
    m_Size			=Size;
    m_Item_Size		=Item_Size;
	m_In			=0;
	m_Out			=0;
	m_Last_Element	=-1;
	return 1;

}

bool CFifoDyn::Reset()
{// Reset des Speichers und der Lauf-Pointer

 	if (m_Is_Initialized==0) return 0;
    m_In=0;
	m_Out=0;
	m_Last_Element=-1;
	return 1;

}


int CFifoDyn::Ready_To_Receive(unsigned short Length)
{
	if ((m_In+m_Item_Size+Length+m_HeaderSize)>m_Size)
	{
 		if(m_Out==0) return 0;
		//if( (Length+m_HeaderSize)>m_Out ) return FALSE; // Vom Anfang gerechnet
 	} //Ueberlauf des FIFO beim Sprung zum Anfang


    if(m_In<m_Out)
	{
		if( (m_In+Length+m_HeaderSize)>=m_Out )  return 0;//Ueberlauf des FIFO
	}

	return 1;
}




/* Erweiterungen 						 				  	*/
/* Nur verwenden wenn sichergestellt ist, dass nur eine 	*/
/* Funktion lesend oder schreibend auf das Fifo zugreift !!	*/


void* CFifoDyn::Get_Next_Input_Pointer(unsigned short Length)
{// Zeiger auf die naechste Einfuegestelle 					*/
	if (Ready_To_Receive(Length)==0) return NULL;
	return (void*)(((long)m_pWurzel)+m_In);
}

int CFifoDyn::Inc_Input_Pointer(unsigned short Length)
{// Zeiger auf die naechste Einfuegestelle weitersetzen


	if (Ready_To_Receive(Length)==0) return 0;

    m_Last_Element=m_In;

    if( (m_In+Length+m_HeaderSize+m_Item_Size)>m_Size ) m_In=0;
    else m_In+=Length+m_HeaderSize;

	CalculateFreeMemory();

    return 1;

}

void* CFifoDyn::Get_Next_Output_Pointer(void)
{// Zeiger auf die naechste Auslesestelle
	if (m_Out==m_In) return NULL;//Keine Daten zu Lesen da

    return (void*)((long)m_pWurzel+m_Out);
}

int CFifoDyn::Inc_Output_Pointer(void)
{//Zeiger auf die naechste Auslesestelle weitersetzen

	MESSAGE* pMsg	= NULL;
	pMsg			= (MESSAGE*)((long)m_pWurzel+m_Out); //aktuelle Auslesestelle

	if(pMsg==NULL) return 0;
 	m_Out+=pMsg->Length+m_HeaderSize;

    if( (m_Out+m_Item_Size)>m_Size ) m_Out=0; //Sprung zum Speicheranfang

	CalculateFreeMemory();

    return 1;
}

/*
BOOL CFifoDyn::Inc_Output_Pointer_n(__int32 n){
//Zeiger  weitersetzen
 	m_Out+=n;
    if ((m_Out+m_Item_Size)>m_Size) m_Out=0; //Sprung zum Speicheranfang

    return TRUE;
}
*/


void CFifoDyn::Kill()
{
	delete []m_pWurzel;
	m_pWurzel=NULL;
}


void CFifoDyn::CalculateFreeMemory()
{
	if(m_In<m_Out)	m_FreeMemory = m_Out-m_In;
	else			m_FreeMemory = m_Size-(m_In-m_Out);
}

int CFifoDyn::GetFreeMemory()
{
	return m_FreeMemory;
}

float CFifoDyn::GetFreeMemoryInPercent()
{
	return ((float)(m_FreeMemory*100)/(float)m_Size);
}
