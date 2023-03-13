#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include "drop.hpp"
using namespace std;

int main()
{
    vector<Drop> drops;
    vector<float> result;

    ofstream myfile;
    myfile.open("little_test_03.txt");

    for (int _ = 0; _ < 1000; _++)
    {
        // drops.push_back(Drop(9.f + ((double)rand() / (RAND_MAX)), 1.7f + 0.1 * ((double)rand() / (RAND_MAX)), 2.0f, 0.0064516f, 1.0 * ((double)rand() / (RAND_MAX)), 1.0 * ((double)rand() / (RAND_MAX))));
        drops.push_back(Drop(0.5 + 0.5 * rand_num(), 1.0 + 0.5 * rand_num(), 0.7, 4.0 + 0.5 * rand_num(), 1.0 * rand_num(), 2.0 + 0.5 * rand_num(), 20 + 5 * rand_num()));
        // drops.push_back(Drop(2.5 + 0.5 * rand_num(), 1.5 + 0.5 * rand_num(), fabs(rand_num()), 3.0 + fabs(rand_num()), 1.5 + 0.5 * rand_num()));
    }

    float t_init = 2.0f;
    float t_end = 3.f;

    for (float t = t_init; t < t_end; t += 1. / 44100.f)
    {
        float res = 0.0f;

        for (int i = 0; i < drops.size(); i++)
        {
            res += drops[i].sample_at_simplified(t);
        }

        result.push_back(res);
        myfile << res << endl;
        cout << res << endl;
    }

    // auto max_val = *std::max_element(result.begin(), result.end(), [](float a, float b)
    //                                  { return std::abs(a) < std::abs(b); });

    // for (auto &elem : result)
    // {
    //     elem /= max_val;
    //     myfile << elem << endl;
    //     cout << elem << endl;
    // }

    myfile.close();

    return 0;
}