#include <vector>
#include "fitlm.h"


// Generiere Gaussfit mit Stoerungen
std::vector<double> FitLM::generateGauss(std::vector<double> &coeff,std::vector<double> &x,double &min,double &max)
{

    std::vector<double> yData;
    double arg = 0.0;
    double ex =0.0;
    double y=0.0;
    min = 1000000000;
    max = -1000000000;
    if( (x.size()<=5)|| (coeff.size()<3) )
    {
       return yData ;
    }
    else
    {
        for (unsigned int i =0;i<x.size();++i)
        {
            arg=(x[i]-a[1])/a[2];
            ex =  exp(-SQR(arg));
            y =   a[0]*ex;

            if(i%15==0)
                y += 115;
            if(i%25==0)
                y-= 130;

            if((i>50)&&(i<100))
                    y+=200;

            if(y<min)
            {
                min = y;
            }
            if(y>max)
            {
               max = y;
            }
            yData.push_back(y);
        }
        return yData;
    }

}


std::vector<double> FitLM::gaussVal(std::vector<double> &coefficients,std::vector<double> &x,double &min,double &max)
{

    std::vector<double> yData;
    double arg = 0.0;
    double ex =0.0;
    double y=0.0;
    min = 10000000;
    max = -10000000;
    if( (x.size()<=5)|| (coefficients.size()<3) )
    {
       return yData ;
    }
    else
    {
        for (unsigned int i =0;i<x.size();++i)
        {
            arg=(x[i]-coefficients[1])/coefficients[2];
            ex =  exp(-SQR(arg));
            y =   coefficients[0]*ex;
            if(y<min)
            {
                min = y;
            }
            if(y>max)
            {
               max = y;
            }
            yData.push_back(y);
        }
        return yData;
    }

}

std::vector<double> FitLM::sinusVal(std::vector<double> &coefficients,std::vector<double> &x,double &min,double &max)
{

    std::vector<double> yData;
    double arg = 0.0;
    double farg = 0.0;
    double y=0.0;
    min = 10000000;
    max = -10000000;
    if( (x.size()<=5)|| (coefficients.size()<3) )
    {
       return yData ;
    }
    else
    {
        for (unsigned int i =0;i<x.size();++i)
        {
            arg=(x[i]*coefficients[2]) + coefficients[3];
            farg = sin(arg);
            y = coefficients[0] + coefficients[1]*farg;
            if(y<min)
            {
                min = y;
            }
            if(y>max)
            {
               max = y;
            }
            yData.push_back(y);
        }
        return yData;
    }

}

void FitLM::mrqcof(std::vector<double> &a, MatDoub_O &alpha, std::vector<double> &beta)
{
    Int i,j,k,l,m;
    double ymod,wt,sig2i,dy;
    std::vector<double> dyda(ma);
    for (j=0;j<mfit;j++)
    {
        for (k=0;k<=j;k++) alpha[j][k]=0.0;
        beta[j]=0.;
    }
    chisq=0.;
    for (i=0;i<ndat;i++)
    {
        funcs(x[i],a,ymod,dyda);
        sig2i=1.0/(sig[i]*sig[i]);
        dy=y[i]-ymod;
        for (j=0,l=0;l<ma;l++)
        {
            if (ia[l])
            {
                wt=dyda[l]*sig2i;
                for (k=0,m=0;m<l+1;m++)
                    if (ia[m]) alpha[j][k++] += wt*dyda[m];
                beta[j++] += dy*wt;
            }
        }
        chisq += dy*dy*sig2i;
    }
    for (j=1;j<mfit;j++)
        for (k=0;k<j;k++)
            alpha[k][j]=alpha[j][k];
}

void FitLM::covsrt(MatDoub_IO &covar)
{
    Int i,j,k;
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


void FitLM::fit()
{

	//const int NDONE=10;                 // Amount orig: 4
	//const int ITMAX=1000;               // Max Amount of Iterations
	//const double learnfactor1 = 0.1;
	//const double learnfactor2 = 2.0;

	std::cout<<"check params: "<<m_oDone<<","<<m_oMaxIteration<<","<<m_oLearnParam1<<","<<m_oLearnParam2<<std::endl;


    int j,k,l,iter,done=0;
    double alamda=.001;
    double ochisq;
    std::vector<double> atry(ma),beta(ma),da(ma);

    mfit=0;
    for (j=0;j<ma;j++)
    {
        if (ia[j])
        {
            mfit++;
        }
    }
    MatDoub oneda(mfit,1);
    MatDoub temp(mfit,mfit);

    mrqcof(a,alpha,beta);

    for (j=0;j<ma;j++)
    {
        atry[j]=a[j];
    }
    ochisq=chisq;
    for (iter=0;iter<m_oMaxIteration;iter++)
    {
        if (done==m_oDone)
        {
            alamda=0.;
        }
        for (j=0;j<mfit;j++)
        {
            for (k=0;k<mfit;k++)
            {
                covar[j][k]=alpha[j][k];
            }
            covar[j][j]=alpha[j][j]*(1.0+alamda);
            for (k=0;k<mfit;k++)
            {
                temp[j][k]=covar[j][k];
            }
            oneda[j][0]=beta[j];
        }
        gaussj(temp,oneda);
        for (j=0;j<mfit;j++)
        {
            for (k=0;k<mfit;k++)
            {
                covar[j][k]=temp[j][k];
            }
            da[j]=oneda[j][0];
        }
        if (done==m_oDone)
        {
            covsrt(covar);
            covsrt(alpha);
            m_oCoefficients = a;

            return;
        }
        for (j=0,l=0;l<ma;l++)
        {
            if (ia[l])
            {
                atry[l]=a[l]+da[j++];
            }
        }
        mrqcof(atry,covar,da);
        if (abs(chisq-ochisq) < MAX(tol,tol*chisq))
        {
            done++;
        }
        if (chisq < ochisq)
        {
			alamda *= m_oLearnParam1;//   0.1;
            ochisq=chisq;
            for (j=0;j<mfit;j++)
            {
                for (k=0;k<mfit;k++)
                {
                    alpha[j][k]=covar[j][k];
                }
                beta[j]=da[j];
            }
            for (l=0;l<ma;l++)
            {
                a[l]=atry[l];
            }
        }
        else
        {
			alamda *= m_oLearnParam2; // 10.0;
            chisq=ochisq;
        }
    }
    throw("Fitmrq too many iterations");
}

double FitLM::getchisq()
{
    return chisq;
}


