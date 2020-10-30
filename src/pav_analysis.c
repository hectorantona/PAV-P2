#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x,unsigned int N) {
    float pow = 0;
    for(int i = 0; i < N; i++)
        pow += x[i]*x[i];
    pow /= N;
    pow = 10*log10(pow);
    return pow;
}
    
float compute_am(const float *x, unsigned int N) {
    float amp = 0;
    for(int i = 0; i < N; i++) {
        if(x[i] < 0) amp -= x[i];
        else amp += x[i];
    }
    amp /= N;
    return amp;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float zcr = 0;
    for (int i = 1; i < N; i++) {
        zcr += ((x[i] < 0 && x[i-1] >= 0) || (x[i] >= 0 && x[i-1] < 0));
    }
    zcr = zcr*fm/(2*(N-1));
    return zcr;
}