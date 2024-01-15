#include "gaussjordan.h"


void gaussj(MatDoub_IO &a, MatDoub_IO &b)
{
	int i, j, k, l, ll, n = a.nrows(), m = b.ncols();
	int icol = 0;
	int irow = 0;

	double big, dum, pivinv;
	VecInt indxc(n), indxr(n), ipiv(n);

	for (j = 0; j<n; j++)
		ipiv[j] = 0;
	for (i = 0; i<n; i++)
	{
		big = 0.0;
		for (j = 0; j<n; j++)
		{
			if (ipiv[j] != 1)
				for (k = 0; k<n; k++)
				{
					if (ipiv[k] == 0) {
						if (std::abs(a[j][k]) >= big) {
							big = std::abs(a[j][k]);
							irow = j;
							icol = k;
						}
					}
				}
		}
		++(ipiv[icol]);
		if (irow != icol)
		{
			for (l = 0; l<n; l++)
				SWAP(a[irow][l], a[icol][l]);
			for (l = 0; l<m; l++)
				SWAP(b[irow][l], b[icol][l]);
		}
		indxr[i] = irow;
		indxc[i] = icol;

		if (a[icol][icol] == 0.0)
		{
			// Fehlerbehandlung ...
			//qDebug() << "gaussj: Singular Matrix";
		}


		pivinv = 1.0 / a[icol][icol];
		a[icol][icol] = 1.0;
		for (l = 0; l<n; l++)
			a[icol][l] *= pivinv;
		for (l = 0; l<m; l++)
			b[icol][l] *= pivinv;
		for (ll = 0; ll<n; ll++)
			if (ll != icol)
			{
				dum = a[ll][icol];
				a[ll][icol] = 0.0;
				for (l = 0; l<n; l++)
					a[ll][l] -= a[icol][l] * dum;
				for (l = 0; l<m; l++)
					b[ll][l] -= b[icol][l] * dum;
			}
	}
	for (l = n - 1; l >= 0; l--)
	{
		if (indxr[l] != indxc[l])
			for (k = 0; k<n; k++)
				SWAP(a[k][indxr[l]], a[k][indxc[l]]);
	}
}



void gaussj(MatDoub_IO &a)
{
	MatDoub b(a.nrows(), 0);
	gaussj(a, b);
}
