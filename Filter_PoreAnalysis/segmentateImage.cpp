/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, HS
 * 	@date		2012
 * 	@brief		Labeling algorithm.
 */


#include "segmentateImage.h"

#undef min
#undef max
#include <algorithm>
#include <array>

namespace precitec {
	using geo2d::Blob;
	using geo2d::DataBlobDetectionT;
namespace filter {


unsigned long long  usedflags;
unsigned long long  RFOflags; ///< ready for output flag
unsigned long long  touchedflags; //64 Bit Bitmask
unsigned long long  rootflags; //64 Bit Bitmask
unsigned long long  RFFflags; //64 Bit Bitmask ready for free flags (reusable nodes)

unsigned short currentspotnumber; //6 bit

std::array<Blob, _ATR_preprocessing_N_spots> g_oBlobBuffer;
unsigned short  rootbuf[_ATR_preprocessing_N_spots]; //root aus g_oBlobBuffer herausgenommen

spotline line[_ATR_preprocessing_N_spots_per_line][2];

unsigned char (* segmentateimage_binfunction) (SSF_SF_InputStruct & img,unsigned long i)=default_segmentateimage_binfunction;

unsigned short lastLineOffset; //6 Bit
unsigned short nextLineOffset; //6 Bit


unsigned short segmentateimage(SSF_SF_InputStruct & img,DataBlobDetectionT & dbd, unsigned int p_oMinBlobSize)
{
	int rc = 0;
	dbd.nspots = 0;
	unsigned short  M = 0; ///< variable just for better readable code (myspotnumber)
	unsigned short  tmpoffset; //fuer swap lastline newline offsets
	unsigned short  pixelvalue;
	unsigned long long bitmask;
	unsigned short 	xg; //< Grenze fuer x
	unsigned short	rootBufferM;
	unsigned short 	llcxmax;
	bool processingdone = false;
	bool ongap = false;
	bool diagonalNeighborFlag = false;

	//Maximal noutspotsmax Werte im Ausgabespotspeicher
	mPowerup_init();
	int errorout=0;
	bool imagedone=false;
	unsigned short xs = 0; ///< 9 bit column number of current pixel
	unsigned short ys = 0; ///< 9 bit row number of current pixel
	bool onspot = false;
    unsigned short nextLineCounter = 0; ///< 6 bit line counter (Schreibposition des Linienbereichsbuffers fuer die naechste Zeile)
    unsigned short lastLineCounter = 0; ///< 6 bit last line counter (Leseposition des aktuellen Linienbereichsbuffers)
    unsigned short lastLineCounterMax = 0; ///< 6 bit maximum value of lastLineCounter for last line, no last line -> no blob in last line

	while (!imagedone)
	{
		if(processingdone)
		{
			//processing already done -> output open root spots - one per clock
			if(RFOflags)
			{
				outputspot(dbd.nspots, dbd.outspot, dbd.noutspotsmax, p_oMinBlobSize);
				if(dbd.nspots >= dbd.noutspotsmax)
				{
					imagedone = true;
					errorout = ERR_TOO_MANY_OUTPUTSPOTS;
				}
			}
			else
            {
                imagedone = true;
            }
			continue; //Image already done? -> YES do nothing
		}

		//normales processing: Zustand nach wait1 11111111111111111111111111111111111111111111111111111111111111
		//vorhanden: pixelvalue, LLC

        pixelvalue = *(img.img + xs + ys*img.pitch);

		xg = line[lastLineCounter][lastLineOffset].xmin; //Derzeit aktueller spotline Eintrag xmin -> xg Grenze fuer x
		llcxmax = line[lastLineCounter][lastLineOffset].xmax; //dito xmax -> llcxmax wird xs>llcxmax wird der naechste spotline Eintrag aktuell
		//Der derzeit aktuelle Spotline Eintrag line[LLC][lloffset] liegt immer rechts von xs oder ueber xs
		if(pixelvalue != 0)  //A. Pixel true in binaerbild
		{

			if(onspot || ongap)    //A.1 Pixel true in binaerbild, Nicht erster Pixel dieses Bereichs in der Zeile
			{
				//onspot: Vorheriger pixel in der Zeile war auch schon true in binaerbild
				//ongap: Vorheriger pixel in der Zeile war dunkel aber dessen Vorgaenger true in binaerbild
				onspot=true; ongap=false;

				//M = spotnummer des aktuellen spotline Eintrags:
				if(lastLineCounter < lastLineCounterMax)
				{
					M = line[lastLineCounter][lastLineOffset].myspotnumber;
				}
				else
				{
					M = 0;
				}

				bitmask = 1ull << M;  //1ll
				rootBufferM = rootbuf[M];
				//Bem: unten ist xs==xg richtig, also nur Verbindung nach oben denn:
				//das wird ja auch auf dem spotgap ueberprueft
				//und leistet dann die diagonale Verbindung nach vorn: vgl. A.2 !(xs+1<xg)
				if (lastLineCounter<lastLineCounterMax && xs==xg) //xmin des aktueller spotline Eintrag erreicht:
				{
					//Derzeit bearbeiteter Spot beruehrt aktuellen spotline Eintrag zum ersten Mal
					//currentspot bleibt, myspot des spotline Eintrag wird aufgeloest: currentspot wird seine root
					touchedflags = touchedflags|(bitmask); //M touched markieren
					if (currentspotnumber != rootBufferM )
					{
						//glue M->root to currentspotnumber and release M->root
						updatenode(currentspotnumber, rootBufferM); //add data of M->root into currentspot
						changeroot(rootBufferM, currentspotnumber); //all childs of M->root get childs of currentspot
					}
				}

				//warte bis Speicherzugriff auf g_oBlobBuffer[currentspotnumber] und line wieder moeglich:
				//std::printf("A.1 ");
				addpixelto(currentspotnumber,pixelvalue,xs,ys); //zugriff g_oBlobBuffer[currentspotnumber]
			} // end A.1
			else // onspot==false und ongap==false //A.2
			{  //A.2: we got the FIRST bright pixel of the row

				if (nextLineCounter >= _ATR_preprocessing_N_spots_per_line)
				{
					//Error: zuviele Eintraege in spotline: error 2 , RFO Spots abraeumen und beenden
					errorout = ERR_TOO_MANY_ENTRIES_IN_SPOTLINE;
					std::cout << "error ERR_TOO_MANY_ENTRIES_IN_SPOTLINE  " << ERR_TOO_MANY_ENTRIES_IN_SPOTLINE << std::endl;
					processingdone = true;
					continue;
				}

				onspot = true; ongap = false; //now we are on blob

				if (!diagonalNeighborFlag)
				{
					//Normal: Kein zurueckliegender diagonaler Nachbar in letzter Zeile: (xs-1,ys-1) war dunkel
					//M ist Spotnummer des aktuellen spotline Eintrag
					if(lastLineCounter < lastLineCounterMax)
                    {
                        M = line[lastLineCounter][lastLineOffset].myspotnumber;
                    }
					else
                    {
                        M = 0;
                    }
				}
				else
				{
					//Zurueckliegender diagonaler Nachbar in letzter Zeile: (xs-1,ys-1) war true in binaerbild
					//M ist Spotnummer des vorherigen spotline Eintrag (SN des diagonalen Nachbarn)
					if(lastLineCounter <= lastLineCounterMax)
                    {
                        M = line[lastLineCounter-1][lastLineOffset].myspotnumber;
                    }
					else
                    {
                        M = 0;
                    }
				}

				bitmask = 1ull<<M;
				//determine currentspot:
				if( (lastLineCounter>=lastLineCounterMax || xs+1<xg) && !diagonalNeighborFlag ) //A.2.1
				{
					//A.2.1: Bin nicht auf Spot von letzter Zeile -> neuen Spot anlegen
					//Bin nicht auf Spot von letzter Zeile denn:
					//dnflag== false also kein zurueckliegender diagonaler Nachbar
					//und xs+1<xg also kein darueberliegender Nachbar (xs==xg) und kein vorranliegender diagonaler Nachbar
					//    oder LLC>=LLCMAX d.h. letzten spotline Eintrag verlassen
// 					std::printf("A.2.1 LLC=%d  LLCMAX=%d  xs=%d  xg=%d  dnflag=%d\n",(int)LLC,(int)LLCMAX,(int)xs,(int)xg,(int)diagonaleNachbarFlag);
					rc = getnewspot(currentspotnumber);
					if(rc)
					{
						//zuviele Eintraege in g_oBlobBuffer: error 1 , RFO Spots abraeumen und beenden
						errorout=ERR_TOO_MANY_ENTRIES_IN_SPOTBUF;
						std::cout << "error ERR_TOO_MANY_ENTRIES_IN_SPOTBUF  "<<ERR_TOO_MANY_ENTRIES_IN_SPOTBUF<<std::endl;
						processingdone = true;
						continue;
					}

					//initialise data of new blob
					g_oBlobBuffer[currentspotnumber].sx		= xs;		// case binary image
					g_oBlobBuffer[currentspotnumber].sy		= ys;		// case binary image
					g_oBlobBuffer[currentspotnumber].si		= 1;		// case binary image
					g_oBlobBuffer[currentspotnumber].npix	= 1;
					g_oBlobBuffer[currentspotnumber].xmin	= g_oBlobBuffer[currentspotnumber].xmax =xs;
					g_oBlobBuffer[currentspotnumber].ymin	= g_oBlobBuffer[currentspotnumber].ymax =ys;
					g_oBlobBuffer[currentspotnumber].startx	= xs;
					g_oBlobBuffer[currentspotnumber].starty	= ys;
				} //end A.2.1
				else // if(LLC>=LLCMAX || xs<xg)  A.2.2
				{
					//A.2.2: bin auf Spot von letzter Zeile -> dessen root wird neuer currentspot
					touchedflags = touchedflags|(bitmask); //M touched markieren
					currentspotnumber = rootbuf[M];
					//std::printf("A.2.2 ");
					addpixelto(currentspotnumber, pixelvalue, xs, ys); //g_oBlobBuffer zugriff
				}

				//DONE: determine currentspot
				bitmask = 1ull<<currentspotnumber;
				touchedflags = touchedflags|(bitmask); //currentspot touched markieren
				//DEBUG std::cout<<"touch: "; printflags(rootflags);
				line[nextLineCounter][nextLineOffset].xmin = xs;
				line[nextLineCounter][nextLineOffset].myspotnumber = currentspotnumber;
				g_oBlobBuffer[currentspotnumber].startx	= xs;
				g_oBlobBuffer[currentspotnumber].starty	= ys;
			} //end: else // onspot==false //A.2
		}//end: A.
		else // B.  Pixel dunkel
		{
			if (lastLineCounter < lastLineCounterMax)
			{
				M = line[lastLineCounter][lastLineOffset].myspotnumber;
			}
			else
			{
				M = 0;
			}

			if (ongap)
			{
				//B.1
				//Bem.: NIE: ongap&&onspot
				//std::printf("B.1\n");
				ongap = false;
				line[nextLineCounter][nextLineOffset].xmax = xs-2;
				++nextLineCounter;
			}
			if (onspot)
			{
				//B.2
				//we are onspot but pixel is dark: end of blob reached -> possibly only a gap between two bright areas
				onspot = false;
				ongap = true;

				//Ist ueber dem Gap ein Spot -> Anhaengen
				bitmask = ((unsigned long long)1)<<M;
				rootBufferM = rootbuf[M];

				if (lastLineCounter<lastLineCounterMax && xs==xg)
				{
					//currentspot bleibt, myspot des Bereichs wird aufgeloest: currentspot wird seine root
					touchedflags = touchedflags|(bitmask); //M touched markieren
					if (currentspotnumber!=rootBufferM )
					{    //glue M->root to currentspotnumber and release M->root
						updatenode(currentspotnumber,rootBufferM ); //add data of M->root into currentspot
						changeroot( rootBufferM,currentspotnumber); //all childs of M->root get childs of currentspot
					}
				}
			} // end if(onspot)
		} // end B.
		++xs;
		      diagonalNeighborFlag = false; //erstmal false setzen, spaeter bei Aktualisierung von LLC wird das evtl. true

		if(xs >= img.npixx)
		{
			//END OF LINE
			xs = 0;
			++ys;
			if(ys >= img.npixy)
			{
				//processing already done -> output open root spots - one per clock
				RFOflags = (usedflags) & (rootflags);
				processingdone = true;
				continue;
			}
			if(onspot || ongap)
			{
				//line done but yet onspot: complete spotdata:
				line[nextLineCounter][nextLineOffset].xmax = img.npixx-1;
				++nextLineCounter;
			}
            lastLineCounterMax = nextLineCounter;
			tmpoffset = nextLineOffset;
            nextLineOffset = lastLineOffset;
            lastLineOffset = tmpoffset;
			onspot = false;
			ongap = false;
            nextLineCounter = lastLineCounter = 0;
			M = line[lastLineCounter][lastLineOffset].myspotnumber;
			RFFflags = (usedflags) & (~rootflags) & (~touchedflags); // mark spots ready to free
			releasefreespots();
			RFOflags = (usedflags) & (rootflags) & (~touchedflags); // mark spots ready for output
			// std::cout << "RFOfl: "; printflags(RFOflags);
			if(RFOflags)
			{
				outputspot(dbd.nspots, dbd.outspot, dbd.noutspotsmax, p_oMinBlobSize);
				if(dbd.nspots >= dbd.noutspotsmax)
				{
					imagedone = true;
					errorout = ERR_TOO_MANY_OUTPUTSPOTS;
				}
			}
			touchedflags = 0ll;
		}
		else // if(xs>=img.npixx)
		{   //not end of a line
			//Aktualisierung von LLC jetzt wieder: aktueller spotline Eintrag ist line[LLC][lloffset]
			if (lastLineCounter<lastLineCounterMax && xs>llcxmax)
			{
                lastLineCounter++;
				            diagonalNeighborFlag = true;
			}
		}

	}//endfor
	return errorout;
}

//*****************************************************************************************************


unsigned char default_segmentateimage_binfunction(SSF_SF_InputStruct & img,unsigned long i)
{
	return img.img[i];
}

//*****************************************************************************************************

void mPowerup_init(void)
{
	touchedflags=0ll; //->none of the nodes is now touched
	rootflags=0ll; //none of the nodes is a root
	RFFflags=0ll; //none of the nodes is ready for free

	//Bitmasks setzen:
	/*LW=1;
	for(i=0;i<_ATR_preprocessing_N_spots;++i)
	{
	g_oBlobBuffer[i].bitmask=LW;
	LW<<=1;
	}*/

	   lastLineOffset=0;
    nextLineOffset=1;

}

//**********************************************************************************************
//***************************************************************************************
// release used spots which are not root and not touched
void releasefreespots(void)
{
	//release all nodes which are untouched and not root
	unsigned long long LW = 1;

	//das kann man auch etwas spaeter machen: ********************
	for(int i = 0; i < _ATR_preprocessing_N_spots; ++i, LW = LW<<1)
	{
		if( (RFFflags & LW) != 0)
		{
			//free node i
			RFFflags=RFFflags & (~LW); //reset bit: ready for free
			usedflags=usedflags & (~LW); //reset used bit -> free
			//std::cout<<"R release"<<i<<std::endl;
		}
	}
	//***********************************************************
}

//***************************************************************************************
// release used spots which are not root and not touched
//NEW: output and release only one blob (the register LSBIT blob) per call (per line)

void outputspot(int & nspots,Blob *outspot,int nspotsmax, unsigned int p_oMinBlobSize)
{
	//output nodes which are used , untouched and root
	int i;
	unsigned long long LW=1ll;

	//das kann man auch etwas spaeter machen: ********************
	for(i=0;i<_ATR_preprocessing_N_spots;++i,LW=LW<<1)
	{
		if( (RFOflags & LW) != 0)
		{
			if(g_oBlobBuffer[i].npix >= p_oMinBlobSize) //nur ausgeben wenn nicht zu klein
			{
				//				std::cout<<"output:"<<nspots<<std::endl;
				//				std::cout<<"nspotsmax:"<<nspotsmax<<std::endl;
				if(nspots<nspotsmax) {
					outspot[nspots] = g_oBlobBuffer[i];
					nspots++;
				} // if
			}
			//free node after output:
			RFOflags=RFOflags & (~LW); //reset bit: ready for output
			usedflags=usedflags & (~LW); //reset used bit -> free

		}
	}
}

//***************************************************************************************
// get a new free blob to use
int getnewspot(unsigned short & newspotnumber)
{
	//get rightmost unset bit of usedflags 64 bit word
	//quick and slow hack

	int i;
	unsigned long long LW;
	unsigned long long bitmask;

	//DEBUG std::cout<<"getnewspot"<<std::endl;
	if(usedflags==0xFFFFFFFFFFFFFFFFull) return 1; //no more spots free to allocate

	for(LW=1,i=0;i<_ATR_preprocessing_N_spots;++i)
	{
		if( (usedflags & LW) == 0) break;
		LW<<=1;
	}
	//std::cout<<"alloc:"<<i<<std::endl;
	newspotnumber=i;
	rootbuf[i]=newspotnumber; //new blob is its own root

	bitmask=1ull<<i;
	rootflags=rootflags|bitmask; //set rootbit
	usedflags=usedflags|bitmask; //set usedflag true -> blob is now used

	return 0;
}

//***************************************************************************************
// update currentspot's data with current pixel
void addpixelto(unsigned short spotnumber, unsigned short wpixwert, unsigned short x, unsigned short y)
{
	if (wpixwert == 0)
    {
		wpixwert = 1;
    }
	g_oBlobBuffer[spotnumber].sx	+= x;	// case binary image
	g_oBlobBuffer[spotnumber].sy	+= y;	// case binary image
	g_oBlobBuffer[spotnumber].si	+= 1;	// case binary image
	g_oBlobBuffer[spotnumber].npix	+= 1;

	g_oBlobBuffer[spotnumber].xmin = std::min(g_oBlobBuffer[spotnumber].xmin,x);
	g_oBlobBuffer[spotnumber].xmax = std::max(g_oBlobBuffer[spotnumber].xmax,x);
	g_oBlobBuffer[spotnumber].ymin = std::min(g_oBlobBuffer[spotnumber].ymin,y);
	g_oBlobBuffer[spotnumber].ymax = std::max(g_oBlobBuffer[spotnumber].ymax,y);

}

//***************************************************************************************
//add data of M->root into currentspot
void updatenode(unsigned short rootspotnumber,unsigned short childspotnumber)
{
	unsigned long long bitmask;
	//std::cout<<"updatenode root:"<<rootspotnumber<<" child:"<<childspotnumber <<std::endl;
	g_oBlobBuffer[rootspotnumber].sx+=g_oBlobBuffer[childspotnumber].sx;
	g_oBlobBuffer[rootspotnumber].sy+=g_oBlobBuffer[childspotnumber].sy;
	g_oBlobBuffer[rootspotnumber].si+=g_oBlobBuffer[childspotnumber].si;	
	g_oBlobBuffer[rootspotnumber].npix+=g_oBlobBuffer[childspotnumber].npix;
	g_oBlobBuffer[rootspotnumber].xmin = std::min(g_oBlobBuffer[rootspotnumber].xmin,g_oBlobBuffer[childspotnumber].xmin);
	g_oBlobBuffer[rootspotnumber].xmax = std::max(g_oBlobBuffer[rootspotnumber].xmax,g_oBlobBuffer[childspotnumber].xmax);
	g_oBlobBuffer[rootspotnumber].ymin = std::min(g_oBlobBuffer[rootspotnumber].ymin,g_oBlobBuffer[childspotnumber].ymin);
	g_oBlobBuffer[rootspotnumber].ymax = std::max(g_oBlobBuffer[rootspotnumber].ymax,g_oBlobBuffer[childspotnumber].ymax);
	bitmask=1ull<<childspotnumber;
	rootflags=rootflags & (~(bitmask)); //reset root bit of M

}

//***************************************************************************************
//all childs of oldroot get childs of newroot
void changeroot(unsigned short oldroot, unsigned short newroot)
{

	int i;
	for(i=0;i<_ATR_preprocessing_N_spots;++i)
	{
		if(rootbuf[i]==oldroot) rootbuf[i]=newroot;
	}

}


//***************************************************************************************
void printflags(unsigned long long w)
{
	unsigned long long LW;
	int i;
	LW=1ll;
	for(i=0;i<64;++i,LW*=2L)
	{
		//std::printf("usedflags=%Ld LW=%Ld %Ld\n",usedflags,LW,(usedflags&LW));
		if(w&LW) std::cout<<"X";
		else std::cout<<"O";
	}
	std::cout<<std::endl;
}




//*********SPOTLINE******************************************************************************************
spotline::spotline()
{
	xmin=0;
	xmax=0;
	myspotnumber=0;
}

inline bool spotline::operator == (const spotline & b) const
{
	if(b.xmin != xmin) return false;
	if(b.xmax != xmax) return false;
	if(b.myspotnumber != myspotnumber) return false;
	return true;
}

inline spotline& spotline::operator = (const spotline & b)
{
	xmin=b.xmin;
	xmax=b.xmax;
	myspotnumber=b.myspotnumber;
	return *this;
}

inline  std::ostream& operator << ( std::ostream& os,  spotline const & v )
{
	os <<" xmin=" << v.xmin;
	os <<" xmax=" << v.xmax;
	os <<" myspotnumber=" << v.myspotnumber;
	return os;
}

} // namespace filter
} // namespace precitec
