#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

template <typename T>
T dbtoa(T db)
{
    return pow(T(10), db / T(20));
}

float mix(float a, float b, float t)
{
    return a * (1 - t) + b * t;
}

float rand_num()
{
    return ((double)rand() / (RAND_MAX));
}

float Fast_InvSqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number * 0.5f;
    y = number;
    i = *(long *)&y;           // Floating point bit hack
    i = 0x5f3759df - (i >> 1); // Magic number
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y)); // Newton 1st iteration
                                         //  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration (disabled)

    return y;
}

// valid when -1 <= x <= 1
float fast_acos(float x)
{
    if (x < -1)
        return 0.f;
    if (x > 1)
        return 0.f;

    float negate = float(x < 0);
    x = abs(x);
    float ret = -0.0187293;
    ret = ret * x;
    ret = ret + 0.0742610;
    ret = ret * x;
    ret = ret - 0.2121144;
    ret = ret * x;
    ret = ret + 1.5707288;
    ret = ret * sqrt(1.0 - x);
    ret = ret - 2 * negate * ret;
    return negate * 3.14159265358979 + ret;
}

class Drop
{
public:
    float rho_0 = 1.29; // kg/m^3
    // pressure level
    float c = 340.f;     // m/s
    float v_term = 10.f; // m/s
    float ke = 0.1 * 0.5 * v_term * v_term;
    float H = 1.7f;       // human height, m
    float x_0 = 2.0f;     // listener distance,m
    float a = 0.0064516f; // splash area, m^2
    float Rs;
    float Rl;
    float t_init;
    float t_end;
    float t2_start, t2_end;

    float a_1 = 1.0;
    float m = 1.33;
    float q = 2.67;
    float f = 29;
    float delta_1 = 0.1;
    float delta_2 = 0.1;

    // easier modelling
    float A0 = 1.1;
    float k = 3.0;
    float s = 1.2;

    Drop(float v_term, float H, float x_0, float a, float delta_1, float delta_2)
    {
        this->v_term = v_term;
        this->H = H;
        this->x_0 = x_0;
        this->a = a;
        this->Rs = sqrtf((x_0 - a) * (x_0 - a) + H * H);
        this->Rl = sqrtf((x_0 + a) * (x_0 + a) + H * H);
        this->t_init = this->Rs / c;
        this->t_end = this->Rl / c;
        this->t2_start = this->t_init + delta_1;
        this->t2_end = this->t_end + delta_2;
    };

    std::vector<float> res;
    float sample_at(float time_stamp)
    {
        if (time_stamp < t_init)
        {
            return 0.f;
        }

        if (time_stamp > t_end && time_stamp < t2_start)
        {
            return 0.f;
        }

        if (time_stamp > t2_end)
        {
            return 0.f;
        }

        float pressure = 0.0f;
        if (time_stamp < t2_start)
        {
            float _t = time_stamp;
            float upper = (c * c * _t * _t - H * H + x_0 * x_0 - a * a);
            float lower = (2 * x_0 * sqrtf(c * c * _t * _t - H * H));
            pressure = rho_0 * c * ke / M_PI * acos(upper / lower);
        }
        else
        {
            float _t = time_stamp;
            pressure = 1.7 * a_1 * exp(-m * (_t - q)) * sin(f * (_t - q));
        }

        res.push_back(pressure);
        return pressure;
    }

    /// @brief A simplified model of 2 stage rain drop
    /// @param delta_1  start time of the second stage, time offset from the end of the first stage
    /// @param delta_2  end time of the second stage, time offset from the end of the first stage
    /// @param A0 amplitude coeff of the first stage
    /// @param k frequency coeff of the first stage, controls the width
    /// @param s center time of the first stage
    /// @param q center time of the second stage
    /// @param f frequency of the second stage
    Drop(float delta_1, float delta_2, float A0, float k, float s, float q, float f)
    {
        this->t_init = s - Fast_InvSqrt(k);
        this->t_end = s + Fast_InvSqrt(k);
        this->t2_start = this->t_end + delta_1;
        this->t2_end = this->t_end + delta_2;
        this->A0 = A0;
        this->k = k;
        this->s = s;
        this->q = q;
        this->f = f;
    };

    float sample_at_simplified(float time_stamp)
    {
        if (time_stamp < t_init)
        {
            return 0.f;
        }

        if (time_stamp > t_end && time_stamp < t2_start)
        {
            return 0.f;
        }

        if (time_stamp > t2_end)
        {
            return 0.f;
        }

        float pressure = 0.0f;
        if (time_stamp < t2_start)
        {
            float _t = time_stamp;
            pressure = A0 * fast_acos(k * (_t - s) * (_t - s));
        }
        else
        {
            float _t = time_stamp;
            pressure = 1.7 * a_1 * exp(-m * (_t - q)) * sin(f * (_t - q));
        }

        res.push_back(pressure);
        return pressure;
    }
};