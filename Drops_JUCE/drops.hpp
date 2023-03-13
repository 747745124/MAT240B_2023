#include "drop.hpp"

class Drops
{
public:
    // because we have to normalize the sound in advance (4 seconds sounds)
    // it's better we precompute all the samples
    std::vector<Drop> drops;
    std::vector<float> result;
    uint num_drops;
    uint counter;

    float frequency;
    float base_center;
    float start_time;
    float end_time;

    // randomness is a number between 0 and 1
    // this generates a sampler between 0 to 4 seconds
    Drops(float randomness = 0.5, uint num_drops = 100, float start_time = 1.0, float end_time = 3.0, float frequency = 25.0, float base_center = 2.0)
    {
        this->num_drops = num_drops;
        this->frequency = frequency;
        this->base_center = base_center;
        this->start_time = start_time;
        this->end_time = end_time;
        this->counter = 0;
        for (int _ = 0; _ < num_drops; _++)
        {
            drops.push_back(Drop(0.5 + randomness * rand_num(), 1.0 + randomness * rand_num(), 0.7, 4.0 + randomness * rand_num(), 1.0 * rand_num(), base_center + 0.5 * rand_num(), frequency + 5 * rand_num()));
        }

        // precompute all the samples
        for (float t = start_time; t < end_time; t += 1. / 44100.f)
        {
            float res = 0.0f;

            for (int i = 0; i < drops.size(); i++)
            {
                res += drops[i].sample_at_simplified(t);
            }

            result.push_back(res);
        }

        normalize(result);
    }

    void recompute(float randomness = 0.5, uint num_drops = 100, float start_time = 1.0, float end_time = 3.0, float frequency = 25.0, float base_center = 2.0)
    {
        drops.clear();
        this->num_drops = num_drops;
        this->frequency = frequency;
        this->base_center = base_center;
        this->start_time = start_time;
        this->end_time = end_time;
        this->counter = 0;
        for (int _ = 0; _ < num_drops; _++)
        {
            drops.push_back(Drop(0.5 + randomness * rand_num(), 1.0 + randomness * rand_num(), 0.7, 4.0 + randomness * rand_num(), 1.0 * rand_num(), base_center + 0.5 * rand_num(), frequency + 5 * rand_num()));
        }

        // precompute all the samples
        for (float t = start_time; t < end_time; t += 1. / 44100.f)
        {
            float res = 0.0f;

            for (int i = 0; i < drops.size(); i++)
            {
                res += drops[i].sample_at_simplified(t);
            }

            result.push_back(res);
        }

        normalize(result);
    }

    float next_sample(int start_sample, int end_sample)
    {
        if (counter >= end_sample)
        {
            counter = start_sample;
        }

        return this->result[counter++];
    }

    float next_sample()
    {
        if (counter >= result.size())
        {
            counter = 0;
        }
        return this->result[counter++];
    }

    void normalize(std::vector<float> &result)
    {
        auto max_val = *std::max_element(result.begin(), result.end(), [](float a, float b)
                                         { return std::abs(a) < std::abs(b); });

        for (auto &elem : result)
        {
            elem /= max_val;
        }
    }
};