#ifndef RANDOMNUMBER_H
#define RANDOMNUMBER_H

#include <QObject>
#include <QElapsedTimer>
#include <QDebug>

#include <cmath>

#ifndef M_SQRT1_2l
#define M_SQRT1_2l 0.707106781186547524400844362104849039L
#endif

class RandomNumber
{
public:
    RandomNumber();
    int irand();
    int irand(int min, int max);
    int irand_max();
    double frand();
    double frand(double min, double max);
    double random_wait_parser(QString random_wait);

private:
    QElapsedTimer timer;

    double fgaussian(double min, double max, double fmu, double fsigma);
    double fcauchy(double min, double max, double fmu, double fgamma);
    double fmaxwellboltzman(double min, double max, double fa);
    double erf(double u);
    double erfi(double s);
};

#endif // RANDOMNUMBER_H
