
/**
 * @brief Funtinen zur Display Darstellung unter QNX
 * @details diese Funtktionen werden nur zum Testen verwendet
 * @file

 *
 * @author JS
 * @date   20.05.10
 * @copyright    Precitec Vision GmbH & Co. KG
*/


//System headers
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>

#include "debugDisplay.h"

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <cstring>


/**
 * Allokiert ein Display Fenster
 *  mit Hoehe und Breite
 */
 CoolImage *AllocBuffer(long w,long h)
{

   	CoolImage *i=(CoolImage*)malloc(sizeof(*i));

   	if (!i) return 0;

   	// the width/height are always what we're passed in
   	i->width=w;
   	i->height=h;
   	// our blit type 0 is a straight memory blit
   	if (blittype==0)
	{
    	i->pitch=w*4;
        //i->pitch=w;
    	if( (i->buffer=(uint8_t*)malloc(w*h*4)))
     		return i;
   }

   // if we fail, free the CoolImage structure, and return 0
   free(i);
   i = nullptr;
   return 0;

}

 /**
  * Gibt Display frei
  *
  */
void FreeBuffer(CoolImage *i)
{

	if (!i)
	{
		return;
	}
	// for blit type 0, we just free the memory previously malloced
	if (blittype==0)
	    free(i->buffer);
   // free the structure as well
   free(i);

}

static struct {
	xcb_connection_t *connection = nullptr;
	xcb_window_t root = XCB_WINDOW_NONE;
	xcb_window_t window = XCB_WINDOW_NONE;
	int depth = 0;
	xcb_gcontext_t gc = XCB_NONE;
} s_x11;

/**
 *  Wirft das Display in einen Photon Widgset: Anzeige
 *
 */
//void BlitBuffer(PtWidget_t *win,CoolImage *i)
void BlitBuffer(CoolImage *i)
{

   // For blit type 0, we use PgDrawImagemx(). We have to make sure
   // to set the region to the windows region first.  Don't forget
   // to flush! :)

   if (blittype==0)
   {
		if (s_x11.connection)
		{
 			xcb_put_image(s_x11.connection, XCB_IMAGE_FORMAT_Z_PIXMAP, s_x11.window, s_x11.gc,
 						  uint32_t(i->width), uint32_t(i->height), 0, 0, 0, s_x11.depth,
 						  i->pitch * i->height, i->buffer);
			xcb_flush(s_x11.connection);
		}
   }

}

/**
 *  Display anlegen
 */
int  CreateDisplay	(int nColor,int nWidth,int nHeight)
{
	ci = AllocBuffer(nWidth,nHeight);
	int screen = 0;
	s_x11.connection = xcb_connect(nullptr, &screen);
	if (xcb_connection_has_error(s_x11.connection))
	{
		xcb_disconnect(s_x11.connection);
		s_x11.connection = nullptr;
		return 0;
	}
	
	auto defaultScreen = [&] () -> xcb_screen_t * {
		int s = screen;
		for (auto it = xcb_setup_roots_iterator(xcb_get_setup(s_x11.connection)); it.rem; --s, xcb_screen_next(&it))
		{
			if (s == 0)
			{
				return it.data;
			}
		}
		return nullptr;
	};
	xcb_screen_t *s = defaultScreen();
	s_x11.root = s->root;
	s_x11.depth = s->root_depth;
	
	s_x11.window = xcb_generate_id(s_x11.connection);
	xcb_create_window(s_x11.connection, s_x11.depth, s_x11.window, s_x11.root,
					  0, 0, nWidth, nHeight, 0,
					  XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT, 0, nullptr);
	xcb_size_hints_t hints;
	memset(&hints, 0, sizeof(hints));
	xcb_icccm_size_hints_set_size(&hints, 1, nWidth, nHeight);
	xcb_icccm_set_wm_normal_hints(s_x11.connection, s_x11.window, &hints);
	xcb_map_window(s_x11.connection, s_x11.window);
	
	
	s_x11.gc = xcb_generate_id(s_x11.connection);
	xcb_create_gc(s_x11.connection, s_x11.gc, s_x11.window, 0, nullptr);
	
	xcb_flush(s_x11.connection);	
	
	return 0;
}


/**
 * Speicher so umkopieren, dass er zur 565 Farbpalette passt
 *
 */
void copy_buffer_8to565(unsigned short *dest,unsigned char *source,int width,int height)
{

	int x,y;

	//unsigned char c;
	unsigned short s;

	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			s = source[x+y*width];
			dest[x+y*width] = ((s <<8 ) & 0xf800) | ((s <<3) & 0x7e0) | ((s>>3) & 0x1f);

		}

	}
}


/**
 * Speicher so umkopieren, dass er zur 8er Farbpalette passt
 *
 */
void copy_buffer_8to8888(uint8_t *dest,unsigned char *source,int width,int height)
{

	int x,y;

	//unsigned char c;
	uint8_t s;

	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			s = source[x+y*width];
			for (int i = 0; i < 3; i++)
			{
				dest[x*4+y*width*4+i] = s; // ((s <<8 ) & 0xf800) | ((s <<3) & 0x7e0) | ((s>>3) & 0x1f);
			}
			// set alpha byte, it's not used, but needed
			dest[x*4+y*width*4 + 4] = 0xFF;

		}

	}
}



/**
 * Speicher anzeigen
 *
 */
void DrawBuffer	(int nId,unsigned long *ulpBuf,int nNr,char *cpStr)
{

	//copy_buffer_8to565(ci->buffer,(unsigned char*)ulpBuf,ci->width,ci->height);
	copy_buffer_8to8888(ci->buffer,(unsigned char*)ulpBuf,ci->width,ci->height);
//	BlitBuffer(win,ci);
	BlitBuffer(ci);
}


/**
 * Display schliessen
 *
 */
void CloseDisplay(void)
{
    FreeBuffer(ci);
   
	if (s_x11.connection)
	{
		if (s_x11.gc)
		{
			xcb_free_gc(s_x11.connection, s_x11.gc);
			s_x11.gc = XCB_NONE;
		}
		if (s_x11.window != XCB_WINDOW_NONE)
		{
			xcb_unmap_window(s_x11.connection, s_x11.window);
			xcb_destroy_window(s_x11.connection, s_x11.window);
			xcb_flush(s_x11.connection);
			s_x11.window = XCB_WINDOW_NONE;
		}
		xcb_disconnect(s_x11.connection);
		s_x11.connection = nullptr;
	}
}


