#pragma once
#include <cmath>

template <class T>
inline T sine(T n)
{
    T nn = n * n;
    return n * (T(3.138982) + nn * (T(-5.133625) +
                                    nn * (T(2.428288) - nn * T(0.433645))));
}

float soft_clip(float x)
{
    if (x > 1.f)
        return 1.f;
    if (x < -1.f)
        return -1.f;
    return 3.f * x / 2.f - x * x * x / 2.f;
}

template <typename T>
T mtof(T m)
{
    return T(440) * pow(T(2), (m - T(69)) / T(12));
}
template <typename T>
T dbtoa(T db)
{
    return pow(T(10), db / T(20));
}

template <class T>
inline T wrap(T v, T hi, T lo)
{
    if (lo == hi)
        return lo;

    // if(v >= hi){
    if (!(v < hi))
    {
        T diff = hi - lo;
        v -= diff;
        if (!(v < hi))
            v -= diff * (T)(unsigned)((v - lo) / diff);
    }
    else if (v < lo)
    {
        T diff = hi - lo;
        v += diff; // this might give diff if range is too large, so check at end
                   // of block...
        if (v < lo)
            v += diff * (T)(unsigned)(((lo - v) / diff) + 1);
        if (v == diff)
            return std::nextafter(v, lo);
    }
    return v;
}

struct Cycle
{
    float t = 0;
    float next_sample(float hertz)
    {
        // caveats when frequency goes too big
        float value = sine(t);
        t += hertz / 48000;
        t = wrap(t, 1.f, -1.f);

        return value;
    }
};
