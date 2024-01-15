// Kleines Demo Programm,
// welches die "get image" Funktion in einen Thread ausgelagert hat,
// welcher einen Puls an die Darstellung  sendet:
// dieser Mechanismus wird in SOUVIS und im weldmaster verwendet


// Demo wird mit dem grabber Controlled Mode aufgebohrt

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <string>
#include <iostream>

#include  <malloc.h>
#include  <assert.h>

//#include  <sys/neutrino.h>
//#include  <sys/netmgr.h>
#include  <unistd.h>
#include  <pthread.h>

//display functionality
//#include  <photon/PtWidget.h>
//#include  <photon/PtWindow.h>
//#include  <photon/PdDirect.h>

#include <semaphore.h>

#include "me4TestLinux.h"
#include "bitmap.h"


#define KAMERA 1

#define WRITE_IMAGES_TO_DISK     0

#if KAMERA
//camera header
#include "pf/photonFocus.h"
#endif


//#define GRABBER_CONTROLLED 1 ***

// pulse init -- InterruptSetup
//struct  sigevent ISREvent;
//struct _msg_info msgInfo;
//struct _pulse msgData;
//struct sigevent timeoutEvent;
//struct timespec timeoutTime;


//int conID;
//int chID;
//int rcvID;

//int threadconID;
//int threadchID;



// Semaphore
sem_t  eventSem;
sem_t  threadSem;

//struct _pulse pulse;
dma_mem* pmem0;
Fg_Struct* fg;
int frameNumber = 0;
bool active = true;

#define MAX_IMAGE_BUFFER 256
#define ANZAHL_LOOP 20
#define FRAMERATE 10

//#define IMAGE_PIXELS 512
//#define IMAGE_LINES  512
#define IMAGE_PIXELS 1024
#define IMAGE_LINES  1024

//static pthread_t threadA;






//blittype
 static long blittype=0;


 // this structure will contain our image data
 typedef struct
 {

   unsigned short *buffer;   // raw RGB565 data
   long width;               // width of image
   long height;              // height of image
   long pitch;               // pitch (bytes per row) of image
} CoolImage;



#define NUMIMAGES 48        // # images in our animation
#define WH 256              // width & height (we'll use a square buffer)
#define RAD (WH>>1)         // 1/2 the width/height for computing animation
#define REPS 100            // how many times we want to blit each image


int ErrorMessage(Fg_Struct *fg)
{
	int			error	= Fg_getLastErrorNumber(fg);
	const char*	err_str = Fg_getLastErrorDescription(fg);
	fprintf(stderr,"Error: %d : %s\n",error,err_str);
	return error;
}

int ErrorMessageWait(Fg_Struct *fg)
{
	int			error	= ErrorMessage(fg);
	printf (" ... press ENTER to continue\n");
	getchar();
	return error;
}


int handleError(int error)
{
#if 0
	const char *s;
	s = pfGetErrorString(error);
     if (error < 0) {
    	 std::printf("Error: %d - %s\n",error,s);
     }else {
    	 std::printf("Warning: %s\n", s);
     }
#endif
     return error;
}





 // returns the allocated buffer
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
    	i->pitch=w*2;
        //i->pitch=w;
    	if( (i->buffer=(unsigned short*)malloc(w*h*2)))
     		return i;
   }

   // if we fail, free the CoolImage structure, and return 0
   free(i);
   return 0;

}

// this function frees the image given
void FreeBuffer(CoolImage *i)
{

	// for blit type 0, we just free the memory previously malloced
	if (blittype==0)
	    free(i->buffer);
   // free the structure as well
   free(i);

}


#if 0
 // this function blits the given buffer using our blit type method
 void BlitBuffer(PtWidget_t *win,CoolImage *i)
{

   // For blit type 0, we use PgDrawImagemx(). We have to make sure
   // to set the region to the windows region first.  Don't forget
   // to flush! :)

   if (blittype==0)
   {
    	PhPoint_t pos={0,0};
    	PhDim_t size={i->width,i->height};
		PgSetRegion(PtWidgetRid(win));
		PgDrawImagemx(i->buffer,0,&pos,&size,i->pitch,0);
		PgFlush();
   }

}
#endif



#if 0
static CoolImage *ci=NULL;
static int color_depth=0;

//static int width,height;
static PtWidget_t *win;
static PtArg_t args[3];
static PhPoint_t pos={50,50};
#endif


#if 0
int  CreateDisplay	(int nColor,int nWidth,int nHeight)
{

	PhDim_t dim={nWidth,nHeight};
	int id=0;

    printf("nWidth,nHeight: %d.%d\n",nWidth,nHeight);

	ci			= AllocBuffer(nWidth,nHeight);
	color_depth	= nColor;

	PtInit("/dev/photon");
	PtSetArg(&args[0],Pt_ARG_POS,&pos,0);
	PtSetArg(&args[1],Pt_ARG_DIM,&dim,0);

	win=PtCreateWidget(PtWindow,Pt_NO_PARENT,2,args);

	PtRealizeWidget(win);
	return id;

}
#endif



void SetBufferWidth	(int nId,int nWidth,int nHeight)
{

}


void SetBuffer		(int nId,unsigned long *ulpBuf)
{

}



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


void copy_buffer_8to8(unsigned short *dest,unsigned char *source,int width,int height)
{

	int x,y;

	//unsigned char c;
	unsigned short s;

	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			s = source[x+y*width];
			dest[x+y*width] = s; // ((s <<8 ) & 0xf800) | ((s <<3) & 0x7e0) | ((s>>3) & 0x1f);

		}

	}
}





#if 0
void DrawBuffer		(int nId,unsigned long *ulpBuf,int nNr,char *cpStr)
{

	copy_buffer_8to565(ci->buffer,(unsigned char*)ulpBuf,ci->width,ci->height);
	//copy_buffer_8to8(ci->buffer,(unsigned char*)ulpBuf,ci->width,ci->height);
	BlitBuffer(win,ci);

}
#endif


#if 0
void CloseDisplay(int dnum)
{
    FreeBuffer(ci);

   /// hide the window and destroy it.
   PtUnrealizeWidget(win);
   PtDestroyWidget(win);
}
#endif



//int actualImageNumber = 0;
void *thread_getImage( void *arg )
{

   // maximalen timeout setzen	:  2,147,483,646 = 0x 7FFF FFFE
   int timeout=0x7FFFFFFE;      //0; //10000;//50000; 	// funktioniert der timeout  --0 infinite ?????
   int actualImageNumber = 0;
   int dmaNum = 0;


   // hier warten wir auf Freigabe, d.h. Fg_Acquire muss gestartet sein ...
   std::cout<<std::endl;
   std::cout<<"thread: auf sem_wait... "<<std::endl;
   sem_wait(&eventSem);
   std::cout<<"thread: sem received... "<<std::endl;

   actualImageNumber = 0;
   for(int nextImageNumber=1;nextImageNumber<=ANZAHL_LOOP;nextImageNumber++)
   {
   	   std::cout<<"thread: loop Nr. "<<nextImageNumber<<std::endl;
   	  // wait for the next frame
      // send pulse
      int status = 0;
      // Im Timeout Fall erneut abfragen
      do{
      	actualImageNumber = Fg_getLastPicNumberBlockingEx(fg,actualImageNumber+1,dmaNum,timeout,pmem0);

      	if(actualImageNumber > nextImageNumber)
      		std::cout<<"thread:grab is too fast: "<<actualImageNumber<<" > "<<nextImageNumber<<std::endl;

      	if(actualImageNumber < 0)
      	{
      		status = actualImageNumber;
      		std::cout<<"status: "<<status<<std::endl;
      	}
      	else
      	{
      	     	std::cout <<"thread: Fg_getLastPicNumberBlockingEx(" << fg << ", " << nextImageNumber << ", " << 0 << ", " << 500 << ", " << pmem0 << ")"
      	      			                << " = " << actualImageNumber << std::endl;


      	     	unsigned long* iPtr0 __attribute__((unused)) = (unsigned long*)Fg_getImagePtrEx(fg,actualImageNumber,0,pmem0);
      	   		   // loop Nummer wird nicht verwendet

#if WRITE_IMAGES_TO_DISK
      	     	std::string oFilename("Bitmap");
      	     	char oHelpStrg[10];
      	     	sprintf(oHelpStrg, "%03d.bmp", actualImageNumber);
      	     	oFilename += std::string(oHelpStrg);
      	     	fileio::Bitmap oBitmap( oFilename, IMAGE_PIXELS, IMAGE_LINES);
     	     	oBitmap.save((unsigned char *)iPtr0);
#endif

#if 0
      	     	DrawBuffer(0,iPtr0,actualImageNumber,0);
#endif


      	}
      	if(active == false)
        {
        	std::cout<<"thread: active flag auf false gesetzt thread wird beendet..."<<actualImageNumber<<std::endl;
        	sem_post(&threadSem);
        	pthread_exit( NULL );
        }

      	nextImageNumber = actualImageNumber;

      }while(status == -2120);    //( (actualImageNumber <ANZAHL_LOOP)&&(actualImageNumber != -2120));

      if(actualImageNumber==ANZAHL_LOOP)
    	  nextImageNumber = ANZAHL_LOOP;


      //MsgSendPulse( conID, getprio(0), 1, actualImageNumber);

   }

   std::cout<<"thread:thread wird beendet.."<<std::endl;
   sem_post(&threadSem);

//   sem_destroy( &eventSem);

   *((int*)arg) = EXIT_SUCCESS;
   pthread_exit(arg);
   return EXIT_SUCCESS;

}




int main(int argc, char *argv[])
{

    const char* str;
    int error = 0;
    int i = 0;

#if 0
	// create channel for receiving messages and pulses
    chID = ChannelCreate(0);

    // attach connection for sending pulse
	conID = ConnectAttach(ND_LOCAL_NODE,
			  getpid(),
			  chID,
			  _NTO_SIDE_CHANNEL,
			  0);

    std::cout<<"chID,conID "<<chID<<", "<<conID<<std::endl;
#endif


	sem_init( &eventSem,
	          0,    // not shared
	          0);   //SEAM_VALUE_MAX, 0 locked


    sem_init (&threadSem,0,0);

	std::cout<<" Lade hap file Simple_RT5_AquisitionLinux64.hap"<<std::endl;
	fg = Fg_InitEx("Simple_RT5_AquisitionLinux64.hap",0,0 );

	if(fg == NULL){
		fprintf(stderr, "Error in Fg_Init: %s\n", Fg_getLastErrorDescription(NULL));
		exit(-1);
	}


	int nrOfParameter = Fg_getNrOfParameter(fg);
	fprintf(stdout,"Nr of Parameter: %d\n",nrOfParameter);


	int ok = 0;

#if 0
	for (int i = 0; i < nrOfParameter-2; i++) {
		const char *name = Fg_getParameterName(fg,i);
		fprintf(stdout," Param %d: %s,%x\n",i,name,Fg_getParameterId(fg,i));
	}
#endif


//    std::cout<<"Eingabe weiter ..."<<std::endl;
//    int d;
//    std::cin>>d;

	int width0  = IMAGE_PIXELS; //1024;
	int height0 = IMAGE_LINES; //1024;
	int byteWidth0 = 1;


	 /* WM Methode test: Buffer bereitstellen ******************
	 const int	  	IBuffers		= 256;
	 const int	  	IWidth			= 256;
	 const int	  	IHeight			= 256;
	 const int	  	ISize			= ((IWidth * IHeight + 4095)) & 0xfffff000; // auf Pagegroesse aufrunden
	 const int		ITotalSize		= IBuffers * ISize;


	 unsigned char* wmbuffer = (unsigned char*)malloc(ITotalSize *sizeof(unsigned char*) );
	 int SingleImageSize = (width0 * height0 + 4095) & 0xfffff000; // auf Pagegroesse aufrunden

	 int INumBuffers = ITotalSize/SingleImageSize;

	 std::cout<<"Moegliche Anzahl Bilder im User Speicher bei "<<ITotalSize<<" user mem size und "<<SingleImageSize<<" Einzelbildgroesse"<<std::endl;
	 std::cout<<"width: "<<width0<<" height: "<<height0<<std::endl;
	 std::cout<<"---> "<<INumBuffers<<std::endl;

	 const int newSize = SingleImageSize * INumBuffers;

	 //free(wmbuffer);

	 // bug
	 if((pmem0 = Fg_AllocMemHead(fg,newSize,INumBuffers)) == NULL){
	 		error = ErrorMessageWait(fg);
	 		const char*str=Fg_getLastErrorDescription(fg);
	 		int error=Fg_getLastErrorNumber(fg);
	 		fprintf(stdout,"Init Error(%d): %s\n",error,str);
	 		return error;
	 	} else {
	 		fprintf(stdout,"%d framebuffer allocated \n",MAX_IMAGE_BUFFER );
	 	}

	****************************************************************/



	std::cout<<"Allokiere "<< MAX_IMAGE_BUFFER <<" Bilder ..."<<std::endl;
	// via user memory
	size_t totalBufSize = width0*height0*MAX_IMAGE_BUFFER*byteWidth0;

	if((pmem0 = Fg_AllocMemHead(fg,totalBufSize,MAX_IMAGE_BUFFER)) == NULL){
		error = ErrorMessageWait(fg);
		const char*str=Fg_getLastErrorDescription(fg);
		int error=Fg_getLastErrorNumber(fg);
		fprintf(stdout,"Init Error(%d): %s\n",error,str);
		return error;
	} else {
		fprintf(stdout,"%d framebuffer allocated \n",MAX_IMAGE_BUFFER );
	}

	//unsigned char** buffer = (unsigned char**)malloc(MAX_IMAGE_BUFFER*sizeof(unsigned char*));
    unsigned char* buffer[MAX_IMAGE_BUFFER];






	for(i=0;i<MAX_IMAGE_BUFFER;i++)
	{
		buffer[i] = (unsigned char*)malloc(width0*height0*byteWidth0);
		Fg_AddMem(fg,buffer[i],width0*height0*byteWidth0,i,pmem0);
	}



	//Allokieren von MAX_IMAGE_BUFFER mit siso Methode
	//pmem0 = Fg_AllocMemEx(fg,width0*height0*MAX_IMAGE_BUFFER*byteWidth0,MAX_IMAGE_BUFFER);
	//std::cout << "Fg_AllocMemEx(" << fg << ", " << width0*height0*MAX_IMAGE_BUFFER*byteWidth0 << ", " << 4 << ")" << " = " << pmem0 << std::endl;




    // Adressen ablegen
	unsigned long __attribute__((unused)) logAddr[MAX_IMAGE_BUFFER];
    void* imgPtr;
    for(int i = 0;i < MAX_IMAGE_BUFFER;i++)
    {
        imgPtr = (void *) Fg_getImagePtrEx(fg,i+1,0,pmem0);
        logAddr[i] = (unsigned long)imgPtr;
        //std::cout<<"Memory for image "<<i+1<<": "<<logAddr[i]<<std::endl;
    }




// ---------------------------------------------------------------------------


/*============ CameraGrayAreaBase : camera ============== */
	int Process0_camera_UseDval_Id = Fg_getParameterIdByName(fg,"Device1_Process0_Aquisition_UseDval");
	int Process0_camera_UseDval;
	Process0_camera_UseDval = CameraGrayAreaBase::DVAL_Enabled;
	ok = Fg_setParameter(fg,Process0_camera_UseDval_Id,&Process0_camera_UseDval,0);
	std::cout << "Fg_setParameter(Device1_Process0_Aquisition_UseDval " << fg << ", " << Process0_camera_UseDval_Id << ", " << Process0_camera_UseDval << ", " << 0 << ")" << " = " << ok << std::endl;

	int Process0_camera_Format_Id = Fg_getParameterIdByName(fg,"Device1_Process0_Aquisition_Format");
	int Process0_camera_Format;
	Process0_camera_Format = CameraGrayAreaBase::DualTap8Bit;
	ok = Fg_setParameter(fg,Process0_camera_Format_Id,&Process0_camera_Format,0);
	std::cout << "Fg_setParameter(Device1_Process0_Aquisition_Format " << fg << ", " << Process0_camera_Format_Id << ", " << Process0_camera_Format << ", " << 0 << ")" << " = " << ok << std::endl;


/*============ ImageBuffer : buffer ============== */

	int Process0_buffer_XOffset_Id = Fg_getParameterIdByName(fg,"Device1_Process0_ImageBuffer_XOffset");
	int Process0_buffer_XOffset;
	Process0_buffer_XOffset = 0;
	ok = Fg_setParameter(fg,Process0_buffer_XOffset_Id,&Process0_buffer_XOffset,0);
	std::cout << "Fg_setParameter(Device1_Process0_ImageBuffer_XOffset " << fg << ", " << Process0_buffer_XOffset_Id << ", " << Process0_buffer_XOffset << ", " << 0 << ")" << " = " << ok << std::endl;

	int Process0_buffer_XLength_Id = Fg_getParameterIdByName(fg,"Device1_Process0_ImageBuffer_XLength");
	int Process0_buffer_XLength;
	Process0_buffer_XLength = 1024;
	ok = Fg_setParameter(fg,Process0_buffer_XLength_Id,&Process0_buffer_XLength,0);
	std::cout << "Fg_setParameter(Device1_Process0_ImageBuffer_XLength " << fg << ", " << Process0_buffer_XLength_Id << ", " << Process0_buffer_XLength << ", " << 0 << ")" << " = " << ok << std::endl;

	int Process0_buffer_YOffset_Id = Fg_getParameterIdByName(fg,"Device1_Process0_ImageBuffer_YOffset");
	int Process0_buffer_YOffset;
	Process0_buffer_YOffset = 0;
	ok = Fg_setParameter(fg,Process0_buffer_YOffset_Id,&Process0_buffer_YOffset,0);
	std::cout << "Fg_setParameter(Device1_Process0_ImageBuffer_YOffset " << fg << ", " << Process0_buffer_YOffset_Id << ", " << Process0_buffer_YOffset << ", " << 0 << ")" << " = " << ok << std::endl;

	int Process0_buffer_YLength_Id = Fg_getParameterIdByName(fg,"Device1_Process0_ImageBuffer_YLength");
	int Process0_buffer_YLength;
	Process0_buffer_YLength = 1024;
	ok = Fg_setParameter(fg,Process0_buffer_YLength_Id,&Process0_buffer_YLength,0);
	std::cout << "Fg_setParameter(Device1_Process0_ImageBuffer_YLength " << fg << ", " << Process0_buffer_YLength_Id << ", " << Process0_buffer_YLength << ", " << 0 << ")" << " = " << ok << std::endl;





	/*============ Zugriff aus TrgPort Area ============== */
	// Vorraussetzung ist der Triggeroperator !!
	// --> simpleAquisition
	// Trigger Mode
    int Trigger_Mode_Id = Fg_getParameterIdByName(fg,"Device1_Process0_Trigger_TriggerMode");
	int Trigger_Mode;


	// grabber Controlled mode
	Trigger_Mode = TrgPortArea::GrabberControlled;
	if( Fg_setParameter(fg,Trigger_Mode_Id,&Trigger_Mode,0) <0 )
	{
		str = Fg_getLastErrorDescription(fg);
		error=Fg_getLastErrorNumber(fg);
		std::cout<<"Trigger Mode setzen fehlgeschlagen "<<error<<" "<<str<<std::endl;
		//return 0;
	}

    // ExSync enable
    int ExsyncEnable_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ExsyncEnable");
	int ExsyncEnable;
	ExsyncEnable = TrgPortArea::ON;
	Fg_setParameter(fg, ExsyncEnable_Id, &ExsyncEnable, 0);

	// Flash aus
	int FlashEnable_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_FlashEnable");
	int FlashEnable;
	FlashEnable = TrgPortArea::OFF;
	Fg_setParameter(fg, FlashEnable_Id, &FlashEnable, 0);

	// Software Trigger id holen: Im GrabberControlled Mode nicht notwendig
	int SoftwareTrgPulse_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_SoftwareTrgPulse");
	unsigned int SoftwareTrgPulse;
	SoftwareTrgPulse = 1;
	//Fg_setParameter(fg, SoftwareTrgPulse_Id, &SoftwareTrgPulse, 0);

    //dynamic read only parameter
	//int SoftwareTrgIsBusy_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_SoftwareTrgIsBusy");

//#ifndef GRABBER_CONTROLLED
	// Trigger Source
	int ImgTrgInSource_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ImgTrgInSource");
	int ImgTrgInSource;
	ImgTrgInSource = TrgPortArea::SoftwareTrigger; //InSignal0; //SoftwareTrigger;

	if(Fg_setParameter(fg, ImgTrgInSource_Id, &ImgTrgInSource, 0)<0)
	{
		str=Fg_getLastErrorDescription(fg);
		error=Fg_getLastErrorNumber(fg);
		std::cout<<"Trigger Eingang setzen fehlgeschlagen "<<error<<" "<<str<<std::endl;
		exit(0);
	}
//#endif

	// Trigger Polaritaet
	int ImgTrgInPolarity_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ImgTrgInPolarity");
	int ImgTrgInPolarity;
	ImgTrgInPolarity = TrgPortArea::HighActive;
	Fg_setParameter(fg, ImgTrgInPolarity_Id, &ImgTrgInPolarity, 0);


	// Trigger downscale
	int ImgTrgDownscale_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ImgTrgDownscale");
	unsigned int ImgTrgDownscale;
	ImgTrgDownscale = 1;
	Fg_setParameter(fg, ImgTrgDownscale_Id, &ImgTrgDownscale, 0);

	// Trigger Genauigkeit
	int Accuracy_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_Accuracy");
	double Accuracy;
	Accuracy = 10;
	Fg_setParameter(fg, Accuracy_Id, &Accuracy, 0);

	// Framerate
	int ExsyncFramesPerSec_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ExsyncFramesPerSec");
	double ExsyncFramesPerSec;

	ExsyncFramesPerSec = FRAMERATE;


	if(Fg_setParameter(fg, ExsyncFramesPerSec_Id, &ExsyncFramesPerSec, 0) <0 )
	{
	 		str=Fg_getLastErrorDescription(fg);
			error=Fg_getLastErrorNumber(fg);
			std::cout<<"Init Error "<<error<<" "<<str<<std::endl;
			//return 0;
	}


	// Exsync Exposure
	int ExsyncExposure_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ExsyncExposure");
	double ExsyncExposure;
	ExsyncExposure = 400;
	Fg_setParameter(fg, ExsyncExposure_Id, &ExsyncExposure, 0);

	//ExSync Delay
	int ExsyncDelay_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ExsyncDelay");
	double ExsyncDelay;
	ExsyncDelay = 0;
	Fg_setParameter(fg, ExsyncDelay_Id, &ExsyncDelay, 0);

	// Exsync Polaritaet
	int ExsyncPolarity_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_ExsyncPolarity");
	int ExsyncPolarity;
	ExsyncPolarity = TrgPortArea::HighActive;
	Fg_setParameter(fg, ExsyncPolarity_Id, &ExsyncPolarity, 0);

	// Flash delay
	int FlashDelay_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_FlashDelay");
	double FlashDelay;
	FlashDelay = 0;
	Fg_setParameter(fg, FlashDelay_Id, &FlashDelay, 0);

	// SW Trigger Timeout, GrabberControlled unnoetig
	int SoftwareTrgDeadTime_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_SoftwareTrgDeadTime");
	double SoftwareTrgDeadTime;
	SoftwareTrgDeadTime = 10000;
	Fg_setParameter(fg, SoftwareTrgDeadTime_Id, &SoftwareTrgDeadTime, 0);

	// Flash Polaritaet
	int FlashPolarity_Id = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_FlashPolarity");
	int FlashPolarity;
	FlashPolarity = TrgPortArea::LowActive;
	Fg_setParameter(fg, FlashPolarity_Id, &FlashPolarity, 0);

    // Ende Trigger Operator ******************************************************************


    //timeout setzen
    int generalTimeOut = 0x7FFFFFFE; //    2,147,483,646
    int statusTimeOut = Fg_setParameter(fg,FG_TIMEOUT,&generalTimeOut ,0);
    std::cout<<"Status Timeout: "<<statusTimeOut<<std::endl;


	// Is anybody out there ??
	int clock=0;
	Fg_getParameter(fg,FG_CAMSTATUS, &clock, 0);
    if( clock >0 )
    	std::cout<<"Kamera (Clock: "<<clock<<") vorhanden"<<std::endl;
    else
    	std::cout<<"Keine Kamera (Clock: "<<clock<<") gefunden"<<std::endl;



#if 0
// ---------------------------------------------------------------------------
    int drawWidth  = IMAGE_PIXELS; //1024;//512;
    int drawHeight = IMAGE_PIXELS; //1024;//512;
    //CreateDisplay(8,width0,height0);
    CreateDisplay(8,drawWidth,drawHeight);
// ---------------------------------------------------------------------------
    //int helpInt = 0;
#endif




#if KAMERA
// ----------------------------------------------------------------------------
// Kamera Initialisierung

/***************************************************************************************************/
   	int 	numOfPorts = 0;
	char 	name[50], manu[50];
	int 	port, mBytes, nBytes, version, type;
    port = 0; // Eine Me4 im System
	TOKEN oToken=0;



	int retVal;
	retVal = pfPortInit(&numOfPorts);
    std::cout << "Portinit: return: " << retVal << " numOfPorts: " << numOfPorts << std::endl;
    for(int i=0; i<numOfPorts; i++)
	{
		pfPortInfo(i, manu, &mBytes, name, &nBytes, &version, &type);
		std::cout << "Port " << i
				 << ": Manufacturer: " << manu
		     << " -- " << name
		     << "  mBytes: " << mBytes
		     << "  nBytes: " << nBytes
		     << "  ver: " << version
		     << "  type: " << type << std::endl;
	}

    std::cout << "call now pfDeviceOpen" << std::endl;
    error = pfDeviceOpen(0);
    std::cout << "after call of pfDeviceOpen" << std::endl;
    std::cout << "DeviceOpen: " << error << "  - " << pfGetErrorString(error) << std::endl;



#if 1
    // Auslesen CameraName
	oToken = pfProperty_ParseName(0, "CameraName");
	std::cout << "token : " << oToken << std::endl;
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << "CameraName:       <" << manu << ">" << std::endl;

    // Auslesen Serial number
	oToken = pfProperty_ParseName(0, "Header.Serial");
	std::cout << "token : " << oToken << std::endl;
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << "Header.Serial:    <" << manu << ">" << std::endl;

    // Auslesen Pixelclock
	oToken = pfProperty_ParseName(0, "Header.Pixelclock");
	std::cout << "token : " << oToken << std::endl;
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << "Header.Pixelclock <" << manu << ">" << std::endl;

    // Auslesen BlackLevelOffset
	oToken = pfProperty_ParseName(0, "Voltages.BlackLevelOffset");
	std::cout << "token : " << oToken << std::endl;
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << "Voltages.BlackLevelOffset <" << manu << ">" << std::endl;

	// Setzen ExposureTime
    oToken = pfProperty_ParseName(0, "ExposureTime");
	std::cout << "token : " << oToken << std::endl;
	sprintf(manu,"%f",15.0);
	error = pfDevice_SetProperty_String(port,oToken, manu);
	if(error< 0)
		handleError(error);

	// Auslesen ExposureTime
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << "ExposureTime      <" << manu << ">" << std::endl;

/******************************************************************************************************/
#endif
#endif

    //  thread anlegen:
   	pthread_attr_t attr;
   	pthread_attr_init( &attr );
   	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );


     //thread starten
	std::cout<<"Grab Start Eingabe ..."<<std::endl;
    std::string inStr;
    std::getline(std::cin, inStr);



    //jetzt thread starten
    pthread_t tid;
    void *status_thread = 0;
    int result;
    pthread_create(&tid ,&attr, thread_getImage, &result );
    //int frameNumber = 0;
   	std::cout<<"Thread ( Fg_getLastPicNumber... )gestartet..."<<std::endl;


	// - jetzt infinite grabben
    // Achtung im GrabberControlled Mode Anzahl Bilder mitgeben
    // Der GrabberControlled MOde wird mit Fg_Acquire sofort gestartet
    // und kann nur mit Fg_stopAcquireEx gestoppt werden
    ok = Fg_AcquireEx(fg,0,ANZAHL_LOOP,ACQ_STANDARD,pmem0);
	//std::cout << "Fg_AcquireEx(" << fg << ", " << 0 << ", " << ANZAHL_LOOP << ", " << "ACQ_STANDARD" << ", " << pmem0 << ")" << " = " << ok << std::endl;

	 sem_post(&eventSem);
     std::cout<<"thread sem freigegeben..."<<std::endl;

	 // Warten auf grabbed controlled mode
	 std::cout<<"Warten auf thread Ende ... "<<std::endl;

	 sem_wait(&threadSem);
	 std::cout<<"thread sem received ... "<<std::endl;

	 pthread_join(tid, &status_thread);
	 pthread_join(tid, &status_thread);
	 if (result != EXIT_SUCCESS) {
	      printf("%d\n",result);
	 }
//	 sem_destroy(&threadSem);


	 std::cout<<"grabber controlled mode fertig..."<<std::endl;


// Noch mal sw controlled

	std::cout<<"Noch mal den SW triggered mode - Eingabe"<<std::endl;
	std::getline(std::cin, inStr);

	ok = Fg_stopAcquireEx(fg,0,pmem0,0);
	std::cout << "Fg_stopAcquireEx(" << fg << ", " << 0 << ", " << pmem0  << ", " << 0 << ")" << " = " << ok << std::endl;


	Trigger_Mode = TrgPortArea::ExternSw_Trigger;
	if( Fg_setParameter(fg,Trigger_Mode_Id,&Trigger_Mode,0) <0 )
	{
		str = Fg_getLastErrorDescription(fg);
		error=Fg_getLastErrorNumber(fg);
		std::cout<<"Trigger Mode setzen fehlgeschlagen "<<error<<" "<<str<<std::endl;
	}
	// Wieder thread starten
	pthread_create(&tid ,&attr, thread_getImage, &result );
	// Wieder grab starten
	ok = Fg_AcquireEx(fg,0,ANZAHL_LOOP,ACQ_STANDARD,pmem0);

	std::cout<<"thread sollte erst jetzt ins Fg_getLastImage laufen - Eingabe..."<<std::endl;
	std::getline(std::cin, inStr);
	sem_post(&eventSem);

	// und triggern
	 for(int i = 1; i<=ANZAHL_LOOP; i++)
	 {
   		std::cout<<"SW Trigger Eingabe..."<<i<<std::endl;
       	std::getline(std::cin, inStr);

    	// Setzen ExposureTime
       	static bool oToggle = true;
    	TOKEN oTokenLocal=0;
    	oTokenLocal = pfProperty_ParseName(0, "ExposureTime");
    	if (oToggle)
    	{
    	    sprintf(manu,"%f",15.0);
    	    oToggle = false;
    	}
    	else
    	{
    	    sprintf(manu,"%f",2.0);
    	    oToggle = true;
    	}
    	error = pfDevice_SetProperty_String(port,oTokenLocal, manu);
    	if(error< 0)
        {
    		handleError(error);
        }

   		// SW Trigger starten
   		int status= Fg_setParameter(fg, SoftwareTrgPulse_Id, &SoftwareTrgPulse, 0);
   		if(status<0)
        {
   			std::cout<<"SW Trigger Error: "<<status<<std::endl;
        }
    }
	 // Abwarten bis thread beendet
	 sem_wait(&threadSem);

	 pthread_join(tid, &status_thread);
	 if (result != EXIT_SUCCESS) {
	      printf("%d\n",result);
	 }

#if KAMERA
#if 0
/*****************************************************************************************************/
    std::cout<<std::endl;
	std::cout<<"Belichtungszeit setzen nachdem Acquire gestartet wurde ..."<<std::endl;
	sprintf(manu,"%f",8.0);
	pfDevice_SetProperty_String(port,oToken, manu);
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << manu << std::endl;


    std::cout<<std::endl;
	std::cout<<"Belichtungszeit noch mal setzen nachdem Acquire gestartet wurde ..."<<std::endl;
	sprintf(manu,"%f",13.0);
	pfDevice_SetProperty_String(port,oToken, manu);
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << manu << std::endl;

/******************************************************************************************************/
#endif
#endif


#if KAMERA
#if 0
/*************************************************************************************************************/
    std::cout<<std::endl;
 	std::cout<<"Belichtungszeit setzen nachdem Fg_getLastPicNumberBlockingEx gestartet wurde ..."<<std::endl;
	sprintf(manu,"%f",17.0);
	pfDevice_SetProperty_String(port,oToken, manu);
	pfDevice_GetProperty_String(port, oToken, manu, 50);
	std::cout << manu << std::endl;


	std::cout<<std::endl;
 	std::cout<<"noch mal Belichtungszeit setzen nachdem Fg_getLastPicNumberBlockingEx gestartet wurde ..."<<std::endl;


	sprintf(manu,"%f",8.0);
	pfDevice_SetProperty_String(port,oToken, manu);  // worauf wartet der ???
    pfDevice_GetProperty_String(port, oToken, manu, 50);
    std::cout << manu << std::endl;

#endif
#endif

/*********************************************************************
    std::cout<<std::endl;
    std::cout<<"Linlog Werte setzen ..."<<std::endl;


    // linlog Mode auf User Defined setzen
    t = pfProperty_ParseName(0, "LinLog.Mode");
   	std::cout << "linlog mode token : " << t << std::endl;
   	sprintf(manu,"%d",4);
   	pfDevice_SetProperty_String(port,t, manu);

    t = pfProperty_ParseName(0, "LinLog.Value1");
    std::cout << "linlog value1 token : " << t << std::endl;
    sprintf(manu,"%d",25);
    pfDevice_SetProperty_String(port,t, manu);

    t = pfProperty_ParseName(0, "LinLog.Value2");
    std::cout << "linlog value2 token : " << t << std::endl;
    sprintf(manu,"%d",10);
    pfDevice_SetProperty_String(port,t, manu);


    t = pfProperty_ParseName(0, "LinLog.Time1");
    std::cout << "linlog time1 token : " << t << std::endl;
    sprintf(manu,"%d",980);
    pfDevice_SetProperty_String(port,t, manu);

    t = pfProperty_ParseName(0, "LinLog.Time2");
    std::cout << "linlog time2 token : " << t << std::endl;
    sprintf(manu,"%d",800);
    pfDevice_SetProperty_String(port,t, manu);

    std::cout<<"Linlog Werte gesetzt ..."<<std::endl;

******************************************************************/

#if KAMERA
#if 0
    // ROI setzen, reihenfolge beachten, sonst kann u.U. nichts gesetzt werden
    int dx = 800;;
    oToken = pfProperty_ParseName(0, "Window.W");
    std::cout << "token : " << oToken << std::endl;
    sprintf(manu,"%d",dx); //UserMode
    error  =pfDevice_SetProperty_String(0,oToken, manu);

    int dy= 1000;
    oToken = pfProperty_ParseName(0, "Window.H");
    std::cout << "token : " << oToken << std::endl;
    sprintf(manu,"%d",dy); //UserMode
    error = pfDevice_SetProperty_String(0,oToken, manu);

    int x = 112;
    oToken = pfProperty_ParseName(0, "Window.X");
    std::cout << "token : " << oToken << std::endl;
    sprintf(manu,"%d",x); //UserMode
    error = pfDevice_SetProperty_String(0,oToken, manu);

   	int y=12;
   	oToken = pfProperty_ParseName(0, "Window.Y");
    std::cout << "token : " << oToken << std::endl;
    sprintf(manu,"%d",y); //UserMode
    error = pfDevice_SetProperty_String(0,oToken, manu);



   	std::cout << "Camera Roi gesetzt" << x << " : " <<  y << " : " <<  dx << " : " << dy << std::endl;
/**********************************************************************************/
#endif
#endif



	//thread abschiessen
	//pthread_attr_destroy (&attr);
	std::cout<<"thread wird mit false beendet ..."<<std::endl;
	active = false;
	int status= Fg_setParameter(fg, SoftwareTrgPulse_Id, &SoftwareTrgPulse, 0);
	if(status<0)
			std::cout<<"SW Trigger Error: "<<status<<std::endl;

	std::cout<<"Eingabe Stop Acquire Ausloesen..."<<std::endl;
	std::getline(std::cin, inStr);


	ok = Fg_stopAcquireEx(fg,0,pmem0,0);
	std::cout << "Fg_stopAcquireEx(" << fg << ", " << 0 << ", " << pmem0  << ", " << 0 << ")" << " = " << ok << std::endl;


	//Kamera noch zumachen ????


	//ok = Fg_FreeMemEx(fg, pmem0);
	//std::cout << "Fg_MemEx" << fg << ", " << pmem0 << " = " << ok << std::endl;

	//user memory freigeben ...
	for(i=0;i<MAX_IMAGE_BUFFER;i++){
			Fg_DelMem(fg,pmem0,i);
			free(buffer[i]);
	}

	//free(buffer);


//	for(i=0;i<MAX_IMAGE_BUFFER;i++)
//	{
//		free(buffer[i]);
//	}

	Fg_FreeMemHead(fg,pmem0);


	//	fprintf(stdout,"freeing grabber\n");
	ok = Fg_FreeGrabber(fg);
	fprintf(stdout,"free status %d\n", ok);
//}


return EXIT_SUCCESS;
}
// ---------------------------------------------------------------------------






