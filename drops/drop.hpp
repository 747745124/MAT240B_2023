#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

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

    Drop(float v_term, float H, float x_0, float a)
    {
        this->v_term = v_term;
        this->H = H;
        this->x_0 = x_0;
        this->a = a;
        this->Rs = sqrtf((x_0 - a) * (x_0 - a) + H * H);
        this->Rl = sqrtf((x_0 + a) * (x_0 + a) + H * H);
        this->t_init = this->Rs / c;
        this->t_end = this->Rl / c;
    };

    std::vector<float> res;
    float sample_at(float time_stamp)
    {
        if (time_stamp < t_init)
        {
            return 0.f;
        }

        if (time_stamp > t_end)
        {
            return 0.f;
        }

        float _t = time_stamp;
        float upper = (c * c * _t * _t - H * H + x_0 * x_0 - a * a);
        float lower = (2 * x_0 * sqrtf(c * c * _t * _t - H * H));
        float pressure = rho_0 * c * ke / M_PI * acos(upper / lower);

        res.push_back(pressure);
        return pressure;
    }
};