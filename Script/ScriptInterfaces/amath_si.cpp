#include "amath_si.h"
#include "arandomhub.h"

#include <QDebug>
#include <QVector>

#include <cmath>

AMath_SI::AMath_SI() :
    RandomHub(ARandomHub::getInstance())
{
    Description = "Basic and advanced math";

    Help["random"] = "Return a random number between 0 and 1.\nGenerator respects the seed set by SetSeed method of the sim module!";
    Help["gauss"] = "Return a random value sampled from Gaussian distribution with the given mean and sigma";
    Help["poisson"] = "Return a random value sampled from Poisson distribution with the given mean";
    Help["maxwell"] = "Return a random value sampled from Maxwell distribution with the given Sqrt(kT/M)";
    Help["exponential"] = "Return a random value sampled from exponential decay with the given decay time";

    Help["interpolateToRegulareArray"] = "Convert an arbitrary array (can be unsorted) of double [x,y] pairs to a constant bin_size array. "
                                         "The input arguments defines the number of bins and the range (lower and upper boundaries) of the output array. "
                                         "The result is an array of [Xbin, Ybin] pairs, where Xbin is the middle of the bin and Ybin is linear-interpolated value. "
                                         "The array is sorted (increasing values of Xbin)";

    Help["fit1D"] = "Fit the array of [x,y] pairs using the provided TFormula of Cern ROOT.\n"
                  "Optional startParValues arguments can hold array of initial parameter values.\n"
                  "Returned value depends on the extendedOutput argument (false by default),\n"
                  "false: array of parameter values; true: array of [value, error] for each parameter";

    Help["getAnglesBetween3DVectors"] = "Caluculate angles in radians between two or three 3D vectors.\n"
                                        "For the case of two vectors, the method returns an array containing one element: the angle between the vectors;\n"
                                        "For the case of three vectors, the method returns an array of three angles: between 1-2, 2-3, and 3-1.";

    Help["generateDirectionIsotropic"] = "Return [Vx,Vy,Vz] unit vector sampled from isotripic distribution";
}

double AMath_SI::abs(double val)
{
    return std::abs(val);
}

double AMath_SI::acos(double val)
{
    return std::acos(val);
}

double AMath_SI::asin(double val)
{
    return std::asin(val);
}

double AMath_SI::atan(double val)
{
    return std::atan(val);
}

double AMath_SI::atan2(double y, double x)
{
    return std::atan2(y, x);
}

double AMath_SI::ceil(double val)
{
    return std::ceil(val);
}

double AMath_SI::cos(double val)
{
    return std::cos(val);
}

double AMath_SI::cosh(double val)
{
    return std::cosh(val);
}

double AMath_SI::sinh(double val)
{
    return std::sinh(val);
}

double AMath_SI::exp(double val)
{
    return std::exp(val);
}

double AMath_SI::floor(double val)
{
    return std::floor(val);
}

double AMath_SI::log(double val)
{
    return std::log(val);
}

double AMath_SI::log10(double val)
{
    return std::log10(val);
}

double AMath_SI::max(double val1, double val2)
{
    return std::max(val1, val2);
}

double AMath_SI::min(double val1, double val2)
{
    return std::min(val1, val2);
}

double AMath_SI::pow(double val, double power)
{
    return std::pow(val, power);
}

double AMath_SI::sin(double val)
{
    return std::sin(val);
}

double AMath_SI::sqrt(double val)
{
    return std::sqrt(val);
}

double AMath_SI::tan(double val)
{
    return std::tan(val);
}

double AMath_SI::round(double val)
{
    int f = std::floor(val);
    if (val > 0)
    {
        if (val - f < 0.5) return f;
        else return f+1;
    }
    else
    {
        if (val - f < 0.5 ) return f;
        else return f+1;
    }
}

double AMath_SI::random()
{
    return RandomHub.uniform();
}

double AMath_SI::gauss(double mean, double sigma)
{
    return RandomHub.gauss(mean, sigma);
}

double AMath_SI::poisson(double mean)
{
    return RandomHub.poisson(mean);
}

double AMath_SI::maxwell(double a)
{
    double v2 = 0;
    for (int i=0; i<3; i++)
    {
        double v = RandomHub.gauss(0, a);
        v *= v;
        v2 += v;
    }
    return std::sqrt(v2);
}

double AMath_SI::exponential(double tau)
{
    return RandomHub.exp(tau);
}

/*
#include "ahistogram.h"
QVariantList AMath_SI::interpolateToRegulareArray(QVariantList arrayOfPairs, int numBins, double from, double to)
{
    QVariantList res;
    if (numBins < 2)
    {
        abort("interpolateToRegulareArray(): minimum numBins is 2");
        return res;
    }
    if (to <= from)
    {
        abort("interpolateToRegulareArray(): the value of 'to' should be larger than the value of 'from'");
        return res;
    }
    const size_t arraySize = arrayOfPairs.size();
    if (arraySize < 1)
    {
        abort("interpolateToRegulareArray(): input array cannot be empty");
        return res;
    }

    std::vector<std::pair<double,double>> dist;
    dist.resize(arraySize);
    for (size_t iBin = 0; iBin < arraySize; iBin++)
    {
        const QVariantList el = arrayOfPairs[iBin].toList();
        if (el.size() != 2)
        {
            abort("interpolateToRegulareArray(): input array should contain pairs of doubles");
            return res;
        }
        dist[iBin] = {el[0].toDouble(), el[1].toDouble()};
    }

    std::sort(dist.begin(), dist.end(), [](const auto & lhs, const auto & rhs){return (lhs.first < rhs.first);});
    //qDebug() << dist;

    const double step = (to - from) / numBins;
    size_t positionInDist = 0;
    for (int iBin = 0; iBin < numBins; iBin++)
    {
        const double pos = from + iBin * step;
        while (dist[positionInDist].first < pos)
            positionInDist++;

        double val;
        if (dist[positionInDist].first == pos) val = dist[positionInDist].second; // exact match
        else
        {
            // need to interpolate

            const double interpolationFactor = (pos - dist[positionInDist-1].first) / (dist[positionInDist].first - dist[positionInDist-1].first);
            val = AHistogram1D::interpolateHere(dist[positionInDist-1].second, dist[positionInDist].second, interpolationFactor);
        }

        QVariantList thisPair;
        thisPair << pos + 0.5*step << val;
        res.push_back(thisPair);
    }
    return res;
}
*/
