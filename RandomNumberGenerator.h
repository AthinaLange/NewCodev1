#ifndef RANDOMNUMBERGENERATOR_H
#define RANDOMNUMBERGENERATOR_H

#include <random>
#include <time.h>

using namespace std;

// =============================================================================
// Random Number Generator
// =============================================================================

/// A random number generator.
class RandomState {

public:
    // Create and initialize random number generator with the current system time
    RandomState() : eng(static_cast<unsigned long>(time(nullptr))) { }

    // Create and initialize random number generator with a seed
    RandomState(unsigned long seed) : eng(seed) { }

    // Provide a double random number from a uniform distribution between [low, high).
    double uniform_real(double low, double high) {
        uniform_real_distribution<double> dist(low, high);
        return dist(RandomState::eng);
    }

    // Provide a long random number from a uniform distribution between [low, high).
    long uniform_int(long low, long high) {
        uniform_int_distribution<long> dist(low, high-1);
        return dist(RandomState::eng);
    }

    // !!! Other distributions, like gaussian and poisson, are defined and can be used here.

    // Upper bound for long random numbers [..., high).
    long MAX_INT = numeric_limits<long>::max();

protected:
    // Mersenne twister
    mt19937 eng;
};

#endif
