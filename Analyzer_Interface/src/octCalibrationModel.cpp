#include "coordinates/octCalibrationModel.h"

#include <cmath>
#include <algorithm>
#include <limits>
#include <cstdint>

#include <fstream>
#include <iomanip>

namespace precitec
{
namespace coordinates
{

static void matrixCofactor(const double M[6][6], double C[6][6], int p, int q,
int n)
{
    int i = 0;
    int j = 0;
    for (int r= 0; r< n; r++)
    {
        //Copy only those elements which are not in given row r and column c
        for (int c = 0; c < n; c++)
        {
            if (r != p && c != q)
            {
                //If row is filled increase r index and reset c index
                C[i][j++] = M[r][c];
                if (j == n - 1)
                {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

static double matrixDeterminant(double M[6][6], std::size_t n)
{
    auto determinant = 0.0;
    if (n == 1)
    {
        return M[0][0];
    }
    double C[6][6];
    int sign = 1;

    for (std::size_t f = 0; f < n; f++)
    {
        matrixCofactor(M, C, 0, f, n);
        determinant += sign * M[0][f] * matrixDeterminant(C, n - 1);
        sign = -sign;
    }
    return determinant;
}

static void matrixAdjucate(double M[6][6], double A[6][6])
{
    int sign = 1;
    double C[6][6];
    for (std::size_t i=0; i < 6; i++)
    {
        for (std::size_t j = 0; j < 6; j++)
        {
            matrixCofactor(M, C, i, j, 6);
            sign = ((i + j) % 2 == 0) ? 1 : -1;
            A[j][i] = sign * matrixDeterminant(C, 6 - 1);
        }
    }
}

static bool matrixInverse(double M[6][6], double inverse[6][6])
{
    double determinant = matrixDeterminant(M, 6);
    if (determinant == 0)
    {
        return false;
    }

    double adjucate[6][6];
    matrixAdjucate(M, adjucate);
    for (std::size_t i = 0; i < 6; i++)
    {
        for (std::size_t j = 0; j < 6; j++)
        {
            inverse[i][j] = adjucate[i][j] / determinant;
        }
    }

    return true;
}

static float surfaceOffset(const std::vector<float>& z1, const std::vector<float>& z2, float threshold)
{
    const auto size = z1.size();
    std::vector<float> dz(size);

    int maxCount = 0;
    float offset = 0.0f;

    for (std::size_t i = 0; i < size; ++i)
    {
        dz[i] = z2[i] - z1[i];
    }

    for (std::size_t i = 0; i < size; ++i)
    {
        int count = 0;
        double sum = 0.0;
        for (std::size_t j = 0; j < size; ++j)
        {
            if (std::abs(dz[i] - dz[j]) < threshold)
            {
                count++;
                sum += dz[j];
            }
        }

        if (count > maxCount)
        {
            maxCount = count;
            offset = sum / count;
        }
    }

    return offset;
}

static void rectifyIdmData(const std::vector<float>& x, const std::vector<float>& y, const float* z, float* zRectify, const std::vector<double>& k)
{
    const auto size = x.size();
    std::copy(z, z + size, zRectify);

    for (std::size_t i = 0; i < size; ++i)
    {
        const auto u = x[i];
        const auto v = y[i];

        if (k[0] + k[1] * u * u + k[2] * v * v + k[3] * u + k[4] * v < 0)
        {
            zRectify[i] = -zRectify[i];
        }
    }
}

static std::vector<double> ransacIdmData(const float* x, const float* y, const float* z, uint8_t* zValid, int size, float threshold, int iteration)
{
    int countMax = 0;
    std::vector<double> modelMax;

    for (int it = 0; it < iteration; ++it)
    {
        //1. randomly select 5 data points to fit the model
        double A[6][6];
        double Z[6];
        for (int j = 0; j < 5; ++j)
        {
            const int randomIndex = rand() % (size);
            const double u = x[randomIndex];
            const double v = y[randomIndex];
            A[j][0] = 1.0;
            A[j][1] = u * u;
            A[j][2] = v * v;
            A[j][3] = u;
            A[j][4] = v;
            A[j][5] = 0.0;

            Z[j] = z[randomIndex];
        }

        A[5][5] = 1.0;

        double B[6][6];
        matrixInverse(A, B);

        std::vector<double> model{0, 0, 0, 0, 0};
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 5; ++j)
            {
                model[i] += B[i][j] * Z[j];
            }
        }

        //2. find the best model where the most data points agree with the model
        int count = 0;
        for (int i = 0; i < size; ++i)
        {
            const double u = x[i];
            const double v = y[i];

            const float zModel = std::abs(model[0] + model[1] * u * u + model[2] * v * v + model[3] * u + model[4] * v);
            const float zActual = z[i];

            if (std::abs(zModel - zActual) <= threshold)
            {
                ++count;
            }
        }

        if (count > countMax)
        {
            countMax = count;
            modelMax = model;
        }
    }

    //3. determine valid regions of the dataset
    for (int i = 0; i < size; ++i)
    {
        const double u = x[i];
        const double v = y[i];

        const float zModel = std::abs(modelMax[0] + modelMax[1] * u * u + modelMax[2] * v * v + modelMax[3] * u + modelMax[4] * v);
        const float zActual = z[i];
        zValid[i] = std::abs(zModel - zActual) <= threshold;
    }
    return modelMax;
}

static std::vector<double> fitIdmData(const std::vector<float>& x, const std::vector<float>& y, const std::vector<std::vector<float>>& z, const std::vector<std::vector<uint8_t>>& zValid)
{
    std::size_t count = 0;
    for (std::size_t j = 0; j < zValid.size();++j)
    {
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            if (zValid[j][i])
            {
                count++;
            }
        }
    }

    auto A = new double [count][6];
    auto Z = new float [count];

    std::size_t k = 0;
    for (std::size_t index = 0; index < zValid.size(); ++index)
    {
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            if(!zValid[index][i])
            {
                continue;
            }

            const double u = x[i];
            const double v = y[i];
            A[k][0] = 1.0;
            A[k][1] = u * u;
            A[k][2] = v * v;
            A[k][3] = u;
            A[k][4] = v;
            A[k][5] = 0.0;

            Z[k] = z[index][i];

            ++k;
        }
    }

    double B[6][6];

    for (std::size_t i = 0; i < 6; ++i)
    {
        for (std::size_t j = i; j < 6; ++j)
        {
            B[i][j] = 0;
            for (std::size_t k = 0; k < count; ++k)
            {
                B[i][j] += A[k][i] * A[k][j];
            }
            B[j][i] = B[i][j];
        }
    }

    B[5][5] = 1.0; //inverting a 5x5 matrix with a function that only takes a 6x6 matrix

    double inverseB[6][6];
    matrixInverse(B, inverseB);

    auto D = new double [count][6];

    for (std::size_t i = 0; i < 6; ++i)
    {
        for (std::size_t j = 0; j < count; ++j)
        {
            D[j][i] = 0;
            for (std::size_t k = 0; k < 6; ++k)
            {
                D[j][i] += inverseB[i][k] * A[j][k];
            }
        }
    }

    std::vector<double> model{0, 0, 0, 0, 0};
    for (std::size_t i = 0; i < 5; ++i)
    {
        for (std::size_t k = 0; k < count; ++k)
        {
            model[i] += D[k][i] * Z[k];
        }
    }

    delete[] A;
    delete[] Z;
    delete[] D;
    return model;
}

std::pair<std::vector<double>, std::vector<double>> idmModel(const std::vector<float>& x, const std::vector<float>& y, const std::vector<std::vector<float>>& z, float threshold, int iteration)
{
    std::vector<std::vector<float>> zRectify(z.size());
    std::vector<std::vector<uint8_t>> zValid(z.size());

    for (std::size_t i = 0; i < z.size(); ++i)
    {
        zRectify[i] = std::vector<float>(z[i].size());
        zValid[i] = std::vector<uint8_t>(z[i].size());
    }

    for (std::size_t i = 0; i < z.size(); ++i)
    {
        auto modelRough = ransacIdmData(x.data(), y.data(), z[i].data(), zValid[i].data(), x.size(), threshold, iteration);
        if (modelRough[1] < 0)
        {
            for (auto& k : modelRough)
            {
                k = -k;
            }
        }

        rectifyIdmData(x, y, z[i].data(), zRectify[i].data(), modelRough);
    }

    std::vector<float> dl0{0.0f};
    for (std::size_t i = 1; i < zRectify.size(); ++i)
    {
        const auto offset = surfaceOffset(zRectify[i - 1], zRectify[i], threshold);
        dl0.emplace_back(offset);
        for (std::size_t j = 0; j < zRectify[i].size(); ++j)
        {
            zRectify[i][j] = zRectify[i][j] - offset;
        }
    }

    auto k = fitIdmData(x, y, zRectify, zValid);

    std::vector<double> l0(z.size());
    for (std::size_t i = 0; i < l0.size(); ++i)
    {
        l0[i] = k[0] + dl0[i];
    }

    return {l0, {k[1], k[2], k[3], k[4]}};
}

int fiberSwitchSelect(const std::pair<std::vector<double>, std::vector<double>>& model, double idmHalfRange, double sx, double sy)
{
    const auto& l0 = model.first;
    const auto& k = model.second;

    double min = std::numeric_limits<double>::infinity();
    int n = 0;

    for (std::size_t i = 0; i < l0.size(); ++i)
    {
        const double distance = std::abs(std::abs(l0[i] + k[0] * sx * sx + k[1] * sy * sy) - idmHalfRange);
        if (distance < min)
        {
            min = distance;
            n = i;
        }
    }

    return n;
}

void saveIdmRawData(const std::vector<float>& x, const std::vector<float>& y, const std::vector<std::vector<float>>& z, const std::string& fileName)
{
    std::ofstream file{fileName};

    if (!file.is_open())
    {
        return;
    }

    for (std::size_t i = 0; i < x.size(); ++i)
    {
        file << std::left << std::setw(8) << x[i] << " ";
        file << std::left << std::setw(8) << y[i] << " ";
        for (const auto& zj : z)
        {
            file << std::left << std::setw(10) << zj[i] << " ";
        }
        file << std::endl;
    }

    file.close();
}

}
}
