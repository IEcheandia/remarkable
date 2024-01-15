//#include <math.h>
//#include <stdio.h>
//#define MAX
//#include "lib.h"


/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			JS
*  @date			10/2014
*  @file
*  @brief			Performs canny edge detection operation:
*                   Create 1D gauss convolution with standard deviation s
*					Create 1D mask for the first derivative of the gauss in x and y: Gx, Gy
*					Convolve with G alomng the rows and along the columns --> Image x, Image y
*					Convolve Image x with Gx --> Image x' and Image y with Gy --> Image y'
*					
*					
*/
#include "edgeDetectionImpl.h"

#include <opencv2/opencv.hpp>

// std lib
#include <string>

#include "image/image.h"				///< BImage

// system includes
#include <new> 
#include <cmath>
// std lib includes
#include <limits>



/* Scale double point magnitudes and angles to 8 bits */
#define ORI_SCALE 40.0
#define MAG_SCALE 20.0

/* Biggest possible filter mask */
#define MAX_MASK_SIZE 20
#define PI 3.1415

/* Fraction of pixels that should be above the HIGH threshold */
double ratio = 0.1;
int		WIDTH = 0;



namespace precitec {
	using namespace image;
	namespace filter {

int trace(int i, int j, int low,int high, BImage& im, BImage& ori);
int thresh(int threshold, BImage& p_rDestin);

double gauss(double x, double sigma);
double dGauss(double x, double sigma);
double meanGauss(double x, double sigma);


int hysteresis(int high, int low, BImage& im, BImage& oriim);

void canny(double s, const BImage& im, BImage& mag, BImage& ori);

void seperable_convolution(const BImage& im, double *gau, int width, double **smx, double **smy);


void dxy_seperable_convolution(double** im, int nr, int nc, double *gau,int width, double**sm, int which);
void nonmax_suppress(double **dx, double **dy, int nr, int nc,BImage& mag, BImage& ori);
void estimate_thresh(BImage mag, int *low, int *hi);




/**
* @brief					performs the canny edge detection
* @param p_rSource	        input image
* @param p_rDestin		    result image
* @param pMode              decides filtering of the detected edges
*/
int cannyEdge(const BImage& p_rSource, BImage& p_rDestin, int pMode)
{
	int i,j; //k; //,n;
	double s=1.0;        //gauss standard deviation 
	int low = 0, high = -1;  //lower and higher starting threshold



	

	int height = p_rSource.size().height;
	int width  = p_rSource.size().width;
	

	Size2d size(width, height);

	//create local images
	//BImage im(size), magim(size), oriim(size);
	BImage oriim(size); //orientation image - not calculated at the moment
	
	/* Apply the filter */
	//canny (s, im, magim, oriim);
	//canny(s, p_rSource, magim, oriim);
	canny(s, p_rSource, p_rDestin, oriim);

	/* Hysteresis thresholding of edge pixels */
	//p_rDestin = magim;

	if (pMode == 0) // simple thresholding with lower threshold
	{
		//Schwellen rechnen
		if (high < low)
		{
			//estimate_thresh (mag, &high, &low);
			estimate_thresh(p_rDestin, &high, &low);
#ifndef NDEBUG
			std::cout << "Hysteresis thresholds (from image): HI " << high << " LOW " << low << std::endl;
#endif
		}
		thresh(low-1, p_rDestin);

	}
	if(pMode==1) // adaptive threshold ans tracing
	{
		//Schwellen rechnen
		if (high < low)
		{
			//estimate_thresh (mag, &high, &low);
			estimate_thresh(p_rDestin, &high, &low);
#ifndef NDEBUG
			std::cout << "Hysteresis thresholds (from image): HI " << high << " LOW " << low << std::endl;
#endif
		}

		//hysteresis (high, low,p_rDestin, magim, oriim);
		//-- trace pixel between low and high
		hysteresis(high, low, p_rDestin, oriim);
	}

	// binaerbild spiegeln
	/*
	for (i = 0; i < p_rDestin.height(); i++)
	{
		for (j = 0; j < p_rDestin.width(); j++)
		{
			if (p_rDestin[i][j] == 0)
				p_rDestin[i][j] = 255;
			else
				p_rDestin[i][j] = 0;
		}
	}
	*/


	
	for (i=0; i<WIDTH; i++)
	  for (j=0; j<width; j++)
	    p_rDestin[i][j] = 255;

	for (i=height-1; i>height-1-WIDTH; i--)
	  for (j=0; j<width; j++)
		  p_rDestin[i][j] = 255;

	for (i=0; i<height; i++)
	  for (j=0; j<WIDTH; j++)
		  p_rDestin[i][j] = 255;

	for (i=0; i<height; i++)
	  for (j=width-WIDTH-1; j<width; j++)
		  p_rDestin[i][j] = 255;
   
	return (0);
}


/**
* @brief					delivers euclidian distance or magnitued of two double values.
* @param nr			        number of rows.
* @param nc			        number of columns
* @return 		            pointer to 2dim dpouble array
*/
double norm (double x, double y)
{
	return (double) sqrt ( (double)(x*x + y*y) );
}



/**
* @brief					creates 2 dim double array.
* @param nr			        number of rows.
* @param nc			        number of columns
* @return 		            pointer to 2dim dpouble array
*/
double ** f2d(int nr, int nc)
{
	double **x; // *y;
	int i;

	x = (double **)calloc(nr, sizeof (double *));
	if (x == 0)
	{
		std::cout << "Out of storage: F2D" << std::endl;
		exit(1);
	}

	for (i = 0; i<nr; i++)
	{
		x[i] = (double *)calloc(nc, sizeof (double));
		if (x[i] == 0)
		{
			std::cout << "Out of storage: F2D " << i << std::endl;
			exit(1);
		}
	}
	return x;
}


/* range check*/
int range(int i, int j, int height,int width)
{
	if ( (i<0) || (i >= height ))
		return 0;
	if ( (j<0) || (j >= width) )
		return 0;

	return 1;
}


/**
* @brief					performs the canny edge detection
* @param s 		            sigma for gauss kernel
* @param im			        input image
* @param mag			    magnitude image
* @param ori	            orientation image
*/
void canny(double s, const  BImage& im, BImage& mag, BImage& ori)
{
	int width=0;
	double **smx, **smy;
	double **dx, **dy;
	int i, j; // k, n;
	double gau[MAX_MASK_SIZE];  // gauss array
	double dgau[MAX_MASK_SIZE]; // derivated gauss array
	double z;

	int imageHeight = im.height();
	int imageWidth = im.width();

	// Create the gauss and derivatedgauss filter masks 
	// gau: gauss
	for (i = 0; i < MAX_MASK_SIZE; i++)
	{
		gau[i] = meanGauss((float)i, s);
		if (gau[i] < 0.005)
		{
			width = i; // width of gauss
			break;
		}
		dgau[i] = dGauss((float)i, s);
	}

	WIDTH = width / 2;     // gauss width
#ifndef NDEBUG
	int n = width + width + 1; // pixel number of gauss width
	std::cout << "Smoothing with a Gaussian (width = " << n << " )" << std::endl;
#endif

	// creates double array
	smx = f2d(imageHeight, imageWidth);

	// creates double arry
	smy = f2d(imageHeight, imageWidth);

	/* Convolution of source image with a Gaussian in X and Y directions  */
	seperable_convolution(im, gau, width, smx, smy);

	/* Now convolve smoothed data with a derivative */
#ifndef NDEBUG
	printf("Convolution with the derivative of a Gaussian...\n");
#endif
	dx = f2d(imageHeight, imageWidth);

	dxy_seperable_convolution(smx, imageHeight, imageWidth, dgau, width, dx, 1);
	for (int i = 0; i < imageHeight;i++)
	{
		free(smx[i]);
	}
	free(smx);

	dy = f2d(imageHeight, imageWidth);
	dxy_seperable_convolution(smy, imageHeight, imageWidth, dgau, width, dy, 0);
	for (int i = 0; i < imageHeight;i++)
	{
		free(smy[i]);
	}
	free(smy);
    

	/* Create an image of the norm of dx,dy */
	byte dummy = 0;
	//int hdummy = mag.height();
	//int wdummy = mag.width();
		
	for (i = 0; i < imageHeight-1; i++)
	{
		for (j = 0; j < imageWidth-1; j++)
		{
			z = norm(dx[i][j], dy[i][j]);
			dummy = static_cast<byte>(z*MAG_SCALE);
			mag[i][j] = dummy;
			//mag[i][j] = (unsigned char)(z*MAG_SCALE);
		}
	}
    
	
	/* Non-maximum suppression - edge pixels should be a local max */
	//void nonmax_suppress(double **dx, double **dy, int nr, int nc, BImage& mag, BImage& ori);
	nonmax_suppress(dx, dy, imageHeight, imageWidth, mag, ori);

	for (int i = 0; i < imageHeight;i++)
	{
		free(dx[i]);
	}; 
	free(dx);
	for (int i = 0; i < imageHeight;i++)
	{
		free(dy[i]);
	}; 
	free(dy);
}

/*      Gaussian        */
double gauss(double x, double sigma)
{
    double xx;

    if (sigma == 0) return 0.0;
    xx = (double)exp((double) ((-x*x)/(2*sigma*sigma)));
    return xx;
}

/* mean gauss value over 3 pixel*/
double meanGauss (double x, double sigma)
{
	double z;

	z = (gauss(x,sigma)+gauss(x+0.5,sigma)+gauss(x-0.5,sigma))/3.0;
	z = z/(PI*2.0*sigma*sigma);
	return z;
}

/*      First derivative of Gaussian    */
double dGauss (double x, double sigma)
{
	return -x/(sigma*sigma) * gauss(x, sigma);
}

/*      HYSTERESIS thersholding of edge pixels. Starting at pixels with a
	value greater than the HIGH threshold, trace a connected sequence
	of pixels that have a value greater than the LOW threhsold.        */

int hysteresis (int high, int low, BImage& im, BImage& oriim)
{
	int i,j; //k;

#ifndef NDEBUG
	printf ("Beginning hysteresis thresholding...\n");
#endif
    
	/*
	for (i=0; i<im.height(); i++)
	  for (j=0; j<im.width(); j++)
		  im[i][j] = 0;
	*/

//	if (high<low)
//	{
//	  //estimate_thresh (mag, &high, &low);
//	  estimate_thresh(im, &high, &low);
//	  std::cout << "Hysteresis thresholds (from image): HI " << high << " LOW " << low << std::endl;
//	}

   /*	For each edge with a magnitude above the high threshold, begin
		tracing edge pixels that are above the low threshold.               
   */
	for (i = 0; i < im.height(); i++)
	{
		for (j = 0; j < im.width(); j++)
		{
			if (im[i][j] >= high)//mag
				trace(i, j, low,high, im,oriim);  // does only mark the pixels ...
		}
	}
    
	//Make the edge black (to be the same as the other methods) 
	/*
	for (i = 0; i < im.height(); i++)
	{
		for (j = 0; j < im.width(); j++)
		{
			if (im[i][j] == 0)
				im[i][j] = 255;
			else
				im[i][j] = 0;
		}
	}
	*/
	return(0);
}

/*      TRACE - recursively trace edge pixels that have a threshold > the low
	edge threshold around i,j     */

int trace(int i, int j, int low,int high, BImage& im, BImage& ori)
{
	int n,m;
	char flag = 0;
	int height = im.height();
	int width = im.width();

	if (im[i][j] == 0)
	{
	  im[i][j] = 255;
	  flag=0;
	  for (n= -1; n<=1; n++)
	  {
	    for(m= -1; m<=1; m++)
	    {
	      if (i==0 && m==0) 
			  continue;
	      if (range(i+n, j+m,height,width) && im[i+n][j+m] >= low)
			if (   trace(i+n, j+m, low,high, im,ori)   )
			{
				flag=1;
				break;
			}
		}
	    if (flag) 
			break;
	  }
	  return(1);
	}
	
	//im[i][j] = 255; //test
	//std::cout << "i,j " << i <<" "<< j << std::endl;
	return(0);
}



/**
* @brief					performs edge travelling
* @param i 		            starting point: value over threshold
* @param j			        starting point
* @param im				    input image
* @param im				    input image
* @param ori	            orientation image
*/

int traceNeu(int i, int j, int low, int high, BImage& im, BImage& mag, BImage& ori)
{
	int n, m;
	//char flag = 0;
	int height = im.height();
	int width = im.width();

	
	for (n = -1; n <= 1; n++)
	{
		for (m = -1; m <= 1; m++)
		{
			if (range(i + n, j + m, height, width) && mag[i + n][j + m] >= low)
			{ // mark pixel:
				mag[j][j] = high;
				std::cout << "i,j " << i << " " << j << std::endl;
			}
		}
	}
		

	//im[i][j] = 255; //test
	std::cout << "i,j " << i << " " << j << std::endl;
	return(0);
}


/**
* @brief					simple thresholding
* @param low                threshold
* @param p_rDestin   	    input image
*/
int thresh(int threshold, BImage& p_rDestin)
{
	for (int i = 0; i < p_rDestin.height(); i++)
	{
		for (int j = 0; j < p_rDestin.width(); j++)
		{
			if (p_rDestin[i][j] > threshold)
				p_rDestin[i][j] = 255;
			else
				p_rDestin[i][j] = 0;
		}
	}
	
	return 0;
}


/**
* @brief					fast separated convolution ( separated )
* @param gau			    kernel - symmetric
* @param width		        kernel width
* @param smx	            result image x
* @param smy	            result image y
*/
void seperable_convolution (const BImage& im, double *gau, int width,	double **smx, double **smy)
{
	int i,j,k, I1, I2, nr, nc;
	double x, y;

	nr = im.height();
	nc = im.width();

	for (i=0; i<nr; i++)
	  for (j=0; j<nc; j++)
	  {
	    x = gau[0] * im[i][j]; 
		y = gau[0] * im[i][j];
	    for (k=1; k<width; k++)
	    {
	      I1 = (i+k)%nr; 
		  I2 = (i-k+nr)%nr;
	      y += gau[k]*im[I1][j] + gau[k]*im[I2][j];

	      I1 = (j+k)%nc; 
		  I2 = (j-k+nc)%nc;
	      x += gau[k]*im[i][I1] + gau[k]*im[i][I2];
	    }
	    smx[i][j] = x; smy[i][j] = y;
	  }
}

/**
* @brief					fast column or row convolution ( separated )
* @param im			        input image
* @param nr			        number of rows
* @param nc		            number of columns
* @param gau	            gauss kernel
* @param width	            kernel width
* @param sm 	            result double array
* @param which	            x or y convolution
*/
void dxy_seperable_convolution (double** im, int nr, int nc,  double *gau, int width, double **sm, int which)
{
	int i,j,k, I1, I2;
	double x;

	for (i=0; i<nr; i++)
	  for (j=0; j<nc; j++)
	  {
	    x = 0.0;
	    for (k=1; k<width; k++)
	    {
	      if (which == 0)
	      {
			I1 = (i+k)%nr; 
			I2 = (i-k+nr)%nr;
			x += -gau[k]*im[I1][j] + gau[k]*im[I2][j];
	      }
	      else
	      {
			I1 = (j+k)%nc; 
			I2 = (j-k+nc)%nc;
			x += -gau[k]*im[i][I1] + gau[k]*im[i][I2];
	      }
	    }
	    sm[i][j] = x;
	  }
}


/**
* @brief					suppress non local maxima
* @param dx			        input double array ( derivatives) x direction
* @param dy			        input double array ( derivatives) y direction
* @param nr		            number of rows
* @param nr		            number of columns
* @param mag	            magnituge image
* @param ori	            orientation image
*/
void nonmax_suppress (double **dx, double **dy, int nr, int nc, BImage& mag, BImage& ori)
{
	int i,j;//  k,n,m;
	//int top, bottom, left, right;
	double xx, yy, g2, g1, g3, g4, g, xc, yc;

	for (i=1; i<mag.height()-1; i++)
	{
	  for (j=1; j<mag.width()-1; j++)
	  {
	    mag[i][j] = 0;

		/* Treat the x and y derivatives as components of a vector */
	    xc = dx[i][j];
	    yc = dy[i][j];
	    if (fabs(xc)<0.01 && fabs(yc)<0.01) 
			continue;

	    g  = norm (xc, yc);

		/* Follow the gradient direction, as indicated by the direction of
		the vector (xc, yc); retain pixels that are a local maximum. */

	    if (fabs(yc) > fabs(xc))
	    {

		  /* The Y component is biggest, so gradient direction is basically UP/DOWN */
	      xx = fabs(xc)/fabs(yc);
	      yy = 1.0;

	      g2 = norm (dx[i-1][j], dy[i-1][j]);
	      g4 = norm (dx[i+1][j], dy[i+1][j]);
	      if (xc*yc > 0.0)
	      {
			g3 = norm (dx[i+1][j+1], dy[i+1][j+1]);
			g1 = norm (dx[i-1][j-1], dy[i-1][j-1]);
	      } else
	      {
			g3 = norm (dx[i+1][j-1], dy[i+1][j-1]);
			g1 = norm (dx[i-1][j+1], dy[i-1][j+1]);
	      }

	    } else
	    {

		/* The X component is biggest, so gradient direction is basically LEFT/RIGHT */
	      xx = fabs(yc)/fabs(xc);
	      yy = 1.0;

	      g2 = norm (dx[i][j+1], dy[i][j+1]);
	      g4 = norm (dx[i][j-1], dy[i][j-1]);
	      if (xc*yc > 0.0)
	      {
			g3 = norm (dx[i-1][j-1], dy[i-1][j-1]);
			g1 = norm (dx[i+1][j+1], dy[i+1][j+1]);
	      }
	      else
	      {
			g1 = norm (dx[i-1][j+1], dy[i-1][j+1]);
			g3 = norm (dx[i+1][j-1], dy[i+1][j-1]);
	      }
	    }

		/* Compute the interpolated value of the gradient magnitude */
	    if ( (g > (xx*g1 + (yy-xx)*g2)) && (g > (xx*g3 + (yy-xx)*g4)) )
	    {
	      if (g*MAG_SCALE <= 255)
				mag[i][j] = (unsigned char)(g*MAG_SCALE);
	      else
				mag[i][j] = 255;
	      ori[i][j] = byte(atan2 (yc, xc) * ORI_SCALE);
	    } 
		else
	    {
			mag[i][j] = 0;
			ori[i][j] = 0;
	    }

	  }
	}
}

void estimate_thresh (BImage mag, int *hi, int *low)
{
	int i,j,k, hist[256], count;

/* Build a histogram of the magnitude image. */
	for (k=0; k<256; k++) hist[k] = 0;

	for (i=WIDTH; i<mag.height()-WIDTH; i++)
	  for (j=WIDTH; j<mag.width()-WIDTH; j++)
	    hist[mag[i][j]]++;

/* The high threshold should be > 80 or 90% of the pixels 
	j = (int)(ratio*mag->info->nr*mag->info->nc);
*/
	j = mag.height();
	if (j<mag.width()) j = mag.width();
	j = (int)(0.9*j);
	k = 255;

	count = hist[255];
	while (count < j)
	{
	  k--;
	  if (k<0) break;
	  count += hist[k];
	}
	*hi = k;

	i=0;
	while (hist[i]==0) i++;

	*low = int((*hi+i)/2.0);
}

void cannyEdgeCv(const image::BImage& imageIn, image::BImage& imageOut, int mode)
{
    cv::Mat imageInCv(imageIn.height(), imageIn.width(), CV_8UC1, (void*)(imageIn.begin()), imageIn.stride());
    cv::Mat imageOutCv(imageOut.height(), imageOut.width(), CV_8UC1, (void*)(imageOut.begin()), imageOut.stride());

    double lowerThreshold;
    double upperThreshold;

    switch (mode)
    {
    case 0: //Otsu threshold method
    {
        upperThreshold = cv::threshold(imageInCv, imageOutCv, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        lowerThreshold = upperThreshold * 0.5;
        break;
    }
    default: //Manual threshold setting
    {
        lowerThreshold = mode;
        upperThreshold = mode * 1.5;
    }
    }

    cv::Canny(imageInCv, imageOutCv, lowerThreshold, upperThreshold);
}

} // namespace filter
} // namespace precitec
