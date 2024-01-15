///////////////////////////////////////////////////////////
//  OverlayCanvas.h
//  Implementation of the Class OverlayCanvas
//  Created on:      22-Apr-2008 16:38:30
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_DEAAD78D_5D0D_4c4f_83F4_99FC9A3B96C0__INCLUDED_)
#define EA_DEAAD78D_5D0D_4c4f_83F4_99FC9A3B96C0__INCLUDED_

#include "fliplib/Fliplib.h"

namespace fliplib
{
		
	// Die Overlay funktionalitaet wurde komplette aus der Fliplib in Analyser.Interface verschoben,
	// damit zirkulare Importhirachien aufgeloest werden koenne. In der Fliplib ist nur noch ein 
	// Minimalinterface fuer das FilterControlInterface vorhanden
	class OverlayCanvasInterface
	{
		public:
			OverlayCanvasInterface(int width, int height) : width_(width), height_(height) {}
			virtual ~OverlayCanvasInterface() {}
			
			inline int width() 	{ return width_; }
			inline int height() { return height_; }
			
			inline int size() 	{ return height_*width_; }
						
		protected:
			int width_;
			int height_;		
	};	
}

#endif // !defined(EA_DEAAD78D_5D0D_4c4f_83F4_99FC9A3B96C0__INCLUDED_)
