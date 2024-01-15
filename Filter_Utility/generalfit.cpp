#include "generalfit.h"
#include "gaussjordan.h"


//needed functions
// gauss jordan algorithm
void gaussj(MatDoub_IO &a, MatDoub_IO &b);

void GeneralFit::hold(const Int i, const Doub val)
{
    ia[i]=false;
    a[i]=val;
}
void GeneralFit::free(const Int i)
{
    ia[i]=true;
}

int GeneralFit::testFunc(int value)
{

    int dd = value;
    return dd;

}



void GeneralFit::fit()
{
	int i, j, k, l, m, mfit = 0;
	double ym, wt, sum, sig2i;
	std::vector<double> afunc(ma);

	for (j = 0; j < ma; j++)
		if (ia[j])
			mfit++;

	if (mfit == 0)
	{
		// Behandlung ...
		//qDebug()<<"lfit: no parameters to be fitted";
	}


	MatDoub temp(mfit, mfit, 0.);
	MatDoub beta(mfit, 1, 0.);

    for (i=0;i<ndat;i++)
    {
            afunc = funcs(x[i]);
            ym=y[i];
            if (mfit < ma)
            {
                for (j=0;j<ma;j++)
                    if (!ia[j]) ym -= a[j]*afunc[j];
            }
            sig2i=1.0/SQR(sig[i]);
            for (j=0,l=0;l<ma;l++)
            {
                if (ia[l])
                {
                    wt=afunc[l]*sig2i;
                    for (k=0,m=0;m<=l;m++)
                        if (ia[m])
                            temp[j][k++] += wt*afunc[m];
                    beta[j++][0] += ym*wt;
                }
            }
    }
    for (j=1;j<mfit;j++)
            for (k=0;k<j;k++)
                temp[k][j]=temp[j][k];

    gaussj(temp,beta);

    for (j=0,l=0;l<ma;l++)
        if (ia[l])
            a[l]=beta[j++][0];

    chisq=0.0;
    for (i=0;i<ndat;i++)
    {
            afunc = funcs(x[i]);
            sum=0.0;
            for (j=0;j<ma;j++) sum += a[j]*afunc[j];
            chisq += SQR((y[i]-sum)/sig[i]);
    }
    for (j=0;j<mfit;j++)
		for (k=0;k<mfit;k++)
			covar[j][k]=temp[j][k];
    for (i=mfit;i<ma;i++)
         for (j=0;j<i+1;j++)
			 covar[i][j]=covar[j][i]=0.0;
   k=mfit-1;
   for (j=ma-1;j>=0;j--)
   {
		if (ia[j])
		{
			for (i=0;i<ma;i++)
				SWAP(covar[i][k],covar[i][j]);
            for (i=0;i<ma;i++)
				SWAP(covar[k][i],covar[j][i]);
            k--;
        }
   }
}


std::vector<double> GeneralFit::getCoefficients()
{
   std::vector<double> resultVec;

   for(int i=0;i<ma;i++)
     resultVec.push_back(a[i]);

   return resultVec;

}



