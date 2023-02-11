#include <cmath>
#include <iostream>

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

// valid on (-1, 1)
template <class T>
inline T sine(T n)
{
    T nn = n * n;
    return n * (T(3.138982) +
                nn * (T(-5.133625) + nn * (T(2.428288) - nn * T(0.433645))));
}

template <class T>
inline T softclip(T x)
{
    if (x >= T(1))
        return T(1);
    if (x <= T(-1))
        return T(-1);
    return (T(3) * x - x * x * x) / T(2);
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

struct MeanFilter
{
    float x1{0};
    float operator()(float x0)
    {
        float mean = (x0 + x1) / 2;
        x1 = x0;
        return mean;
    }
};