#include "drop.hpp"

class Drops_Simplified
{
public:
    std::vector<Drop> drops;
    float frequency;
    float base_center;
    uint num_drops;
    uint counter;

    // randomness is a number between 0 and 1
    // this generates a sampler between 0 to 4 seconds
    Drops_Simplified(float randomness = 0.5, uint num_drops = 100, float frequency = 25.0, float base_center = 2.0)
    {
        this->num_drops = num_drops;
        this->frequency = frequency;
        this->base_center = base_center;
        this->counter = 0;
        for (int _ = 0; _ < num_drops; _++)
        {
            drops.push_back(Drop(0.5 + randomness * rand_num(), 1.0 + randomness * rand_num(), 0.7, 4.0 + randomness * rand_num(), 1.0 * rand_num(), base_center + 0.5 * rand_num(), frequency + 5 * rand_num()));
        }
    }
};