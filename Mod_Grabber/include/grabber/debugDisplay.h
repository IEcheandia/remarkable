
/**
 * @file
 * @brief  Prototypen und Strukturen zur Darstellung eines Bildes in einem fenster unter QNX
 *
 * @author JS
 * @date   20.05.10
 * @copyright    Precitec Vision GmbH & Co. KG
 *
 */


#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>


///blittype
 static long blittype=0;


 /// Struktur haelt Display Daten
 typedef struct
 {

   uint8_t *buffer;          // raw RGB888 data
   long width;               // width of image
   long height;              // height of image
   long pitch;               // pitch (bytes per row) of image
} CoolImage;


/**
 * Allokiert ein Display Fenster
 *  mit Hoehe und Breite
 */
 CoolImage *AllocBuffer(long w,long h);


 /**
  * Gibt Display frei
  *
  */
void FreeBuffer(CoolImage *i);


/**
 *  Wirft das Display in einen Photon Widgset: Anzeige
 *
 */
//void BlitBuffer(PtWidget_t *win,CoolImage *i);
void BlitBuffer(void);

static CoolImage *ci=NULL;
static int color_depth=0;


/**
 *  Display anlegen
 */
int  CreateDisplay	(int nColor,int nWidth,int nHeight);


/**
 * Speicher so umkopieren, dass er zur 565 Farbpalette passt
 *
 */
void copy_buffer_8to565(unsigned short *dest,unsigned char *source,int width,int height);


/**
 * Speicher so umkopieren, dass er zur 8er Farbpalette passt
 *
 */
void copy_buffer_8to8888(uint8_t *dest,unsigned char *source,int width,int height);



/**
 * Speicher anzeigen
 *
 */
void DrawBuffer	(int nId,unsigned long *ulpBuf,int nNr,char *cpStr);



/**
 * Display schliessen
 *
 */
void CloseDisplay(void);







