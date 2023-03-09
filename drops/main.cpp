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
    ofstream myfile;
    myfile.open("drops.txt");

    for (int _ = 0; _ < 1000; _++)
    {
        drops.push_back(Drop(9.f + ((double)rand() / (RAND_MAX)), 1.7f + 0.1 * ((double)rand() / (RAND_MAX)), 2.0f, 0.0064516f));
    }
    // cout << drops[0].t_init << " " << drops[0].t_end << endl;
    // cout << drops[2].t_init << " " << drops[2].t_end << endl;

    float t_init = 0.00f;
    float t_end = 1.f;

    for (float t = t_init; t < t_end; t += 1. / 44100.f)
    {
        float res = 0.0f;

        for (int i = 0; i < drops.size(); i++)
        {
            res += drops[i].sample_at(t);
        }

        myfile << res << endl;
        cout << res << endl;
    }

    myfile.close();

    return 0;
}
