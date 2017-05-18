#include "randomnumber.h"

RandomNumber::RandomNumber(){
    timer.start();
}

int RandomNumber::irand(){
    srand(timer.nsecsElapsed());
    return rand();
}

int RandomNumber::irand(int min, int max){
    return ( irand() % (max-min+1) ) + min;
}

int RandomNumber::irand_max(){
    return RAND_MAX;
}

double RandomNumber::frand(){
    return (double)irand() / (double)irand_max();
}

double RandomNumber::frand(double min, double max){
    return abs(std::max(max, min)-std::min(max, min))*((double)irand() / (double)irand_max())+std::min(max, min);
}

double RandomNumber::random_wait_parser(QString random_wait){
    QStringList random_wait_ary = random_wait.split(',');
    if(random_wait_ary.at(0).compare("Constant") == 0 && random_wait_ary.length() == 3){
        double randsleep = frand((double)std::min(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()),
                                 (double)std::max(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()));
        return randsleep;
    }else if(random_wait_ary.at(0).compare("Gauss") == 0 && random_wait_ary.length() == 5){
        double randsleep = fgaussian(
                    (double)std::min(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()),
                    (double)std::max(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()),
                    random_wait_ary.at(3).toDouble(),
                    random_wait_ary.at(4).toDouble()
                    );
        return randsleep;
    }else if(random_wait_ary.at(0).compare("Cauchy") == 0 && random_wait_ary.length() == 5){
        double randsleep = fcauchy(
                    (double)std::min(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()),
                    (double)std::max(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()),
                    random_wait_ary.at(3).toDouble(),
                    random_wait_ary.at(4).toDouble()
                    );
        return randsleep;
    }else if(random_wait_ary.at(0).compare("MaxBoltz") == 0 && random_wait_ary.length() == 4){
        double randsleep = fmaxwellboltzman(
                    (double)std::min(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()),
                    (double)std::max(random_wait_ary.at(1).toDouble(), random_wait_ary.at(2).toDouble()),
                    random_wait_ary.at(3).toDouble()
                    );
        return randsleep;
    }else{
        return 30;
    }

}

double RandomNumber::fmaxwellboltzman(double min, double max, double fa){
    double y = frand();
    double x = max;
    for(int i = 0; i < 100; i++){
        x = M_SQRT2 * fa * erfi(y + sqrt(M_2_PI)* (x * exp(-(x*x)/(2.0*fa*fa))/(fa)));
    }
    if(x > max || x < min)
        return fmaxwellboltzman(min, max, fa);
    else
        return x;
}

double RandomNumber::fcauchy(double min, double max, double fmu, double fgamma){
    double result = fmu + fgamma * tan(M_PI * (frand()-0.5));
    if(result > max || result < min)
        return fcauchy(min, max, fmu, fgamma);
    else
        return result;
}

double RandomNumber::fgaussian(double min, double max, double fmu, double fsigma){
    double randnu  = frand();
    double result = fmu + sqrt(2) * fsigma * erfi(2.0*randnu-1.0);
    if(result > max || result < min){
        return fgaussian(min, max, fmu, fsigma);
    }else{
        return result;
    }
}


double RandomNumber::erf(double u){
    const double m_1_sqrtpi = M_2_SQRTPI/2.0;
    double msqrt2 = M_SQRT2;
    double z, y;

    double a[6] = {
        1.161110663653770e-002,
        3.951404679838207e-001,
        2.846603853776254e+001,
        1.887426188426510e+002,
        3.209377589138469e+003
    };

    double b[6] = {
        1.767766952966369e-001,
        8.344316438579620e+000,
        1.725514762600375e+002,
        1.813893686502485e+003,
        8.044716608901563e+003
    };

    double c[10] = {
        2.15311535474403846e-8,
        5.64188496988670089e-1,
        8.88314979438837594e00,
        6.61191906371416295e01,
        2.98635138197400131e02,
        8.81952221241769090e02,
        1.71204761263407058e03,
        2.05107837782607147e03,
        1.23033935479799725E03
    };

    double d[10] ={
        1.00000000000000000e00,
        1.57449261107098347e01,
        1.17693950891312499e02,
        5.37181101862009858e02,
        1.62138957456669019e03,
        3.29079923573345963e03,
        4.36261909014324716e03,
        3.43936767414372164e03,
        1.23033935480374942e03
    };

    double p[7] ={
        1.63153871373020978e-2,
        3.05326634961232344e-1,
        3.60344899949804439e-1,
        1.25781726111229246e-1,
        1.60837851487422766e-2,
        6.58749161529837803e-4
    };

    double q[7] ={
        1.00000000000000000e00,
        2.56852019228982242e00,
        1.87295284992346047e00,
        5.27905102951428412e-1,
        6.05183413124413191e-2,
        2.33520497626869185e-3
    };

    if( std::isnan(u) ){
        return(NAN);
    }

    if(std::isinf(u)) {
        return (u < 0 ? 0.0 : 1.0);
    }

    y =  abs(u);

    if (y <= 0.46875*msqrt2) {
        // evaluate erf() for |u| <= sqrt(2)*0.46875
        z = y*y;
        y = u*((((a[0]*z+a[1])*z+a[2])*z+a[3])*z+a[4])/((((b[0]*z+b[1])*z+b[2])*z+b[3])*z+b[4]);
        return 0.5+y;
    }

    z = exp(-y*y/2)/2;

    if (y <= 4.0) {
        // evaluate erfc() for sqrt(2)*0.46875 <= |u| <= sqrt(2)*4.0
        y = y/msqrt2;
        y = ((((((((c[0]*y+c[1])*y+c[2])*y+c[3])*y+c[4])*y+c[5])*y+c[6])*y+c[7])*y+c[8])
                / ((((((((d[0]*y+d[1])*y+d[2])*y+d[3])*y+d[4])*y+d[5])*y+d[6])*y+d[7])*y+d[8]);
        y = z*y;
    } else {
        // evaluate erfc() for |u| > sqrt(2)*4.0
        z = z*msqrt2/y;
        y = 2/(y*y);
        y = y*(((((p[0]*y+p[1])*y+p[2])*y+p[3])*y+p[4])*y+p[5])
                /(((((q[0]*y+q[1])*y+q[2])*y+q[3])*y+q[4])*y+q[5]);
        y = z*(m_1_sqrtpi-y);
    }
    return (u < 0.0 ? y : 1-y);
}


double RandomNumber::erfi(double s){
    double p = (s+1.0)/2.0;
    if(p < 0.0)
        return -INFINITY;
    if(p > 1.0)
        return +INFINITY;

    double a[6] = {
        -3.969683028665376e+01,
        2.209460984245205e+02,
        -2.759285104469687e+02,
        1.383577518672690e+02,
        -3.066479806614716e+01,
        2.506628277459239e+00
    };

    double b[5] = {
        -5.447609879822406e+01,
        1.615858368580409e+02,
        -1.556989798598866e+02,
        6.680131188771972e+01,
        -1.328068155288572e+01
    };

    double c[6] = {
        -7.784894002430293e-03,
        -3.223964580411365e-01,
        -2.400758277161838e+00,
        -2.549732539343734e+00,
        4.374664141464968e+00,
        2.938163982698783e+00
    };

    double d[4] = {
        7.784695709041462e-03,
        3.224671290700398e-01,
        2.445134137142996e+00,
        3.754408661907416e+00
    };
    double p_low  = 0.02425;
    double p_high = 1.0 - p_low;
    double q = 0.0, x = 0.0, r = 0.0;

    if(0.0 < p && p < p_low){
        q = sqrt(-2.0*log(p));
        x = (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5])/((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1.0);
        return x*M_SQRT1_2l;
    }

    if(p_low <= p && p <= p_high){
        q = p - 0.5;
        r = pow(q, 2.0);
        x = (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q/(((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1.0);
        return x*M_SQRT1_2l;
    }

    if(p_high < p && p < 1.0){
        q = sqrt(-2.0*log(1.0-p));
        x = -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5])/((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1.0);
        return x*M_SQRT1_2l;
    }
    return NAN;
}

