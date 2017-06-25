#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <time.h>

using namespace std;

// =============================================================================
// Global
// =============================================================================

const int N_PATHS = 4;
const int N_LEVELS = 4;

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

    // Provide a double random number from a uniform distribution between (low, high).
    double uniform_real(double low, double high) {
        uniform_real_distribution<double> dist(low, high);
        return dist(RandomState::eng);
    }

    // Provide a long random number from a uniform distribution between (low, high).
    long uniform_int(long low, long high) {
        uniform_int_distribution<long> dist(low, high-1);
        return dist(RandomState::eng);
    }

    // Upper bound for long random numbers [..., high).
    long MAX_INT = numeric_limits<long>::max();

protected:
    // Mersenne twister
    mt19937 eng;
};

// =============================================================================
// Multi Paths Data
// =============================================================================

// No data protection is needed when parallelizing process_path(),
// because only one path is writing the data, and only following paths
// that do not exist yet when the data is written will only read the data.

struct PathData {
    PathData(long parent_id, const vector<double>& data1D, const vector<vector<double>> data2D)
    : parent_id(parent_id), data1D(data1D), data2D(data2D) { }
    long                    parent_id;
    vector<double>          data1D;
    vector<vector<double>>  data2D;
};

// !!!

// =============================================================================
// Path Processing Queue (FIFO)
// =============================================================================

// A queue (FIFO) provides breath first processing of the path structure (tree),
// vs. a stack (LIFO) that provides depth first processing.
// Breath first processing is better for parallel processing,
// as it adds more paths to the processing queue quicker.

struct PathInfo {
    PathInfo(long parent_id, long id, long level, RandomState random_state)
    : parent_id(parent_id), id(id), level(level), random_state(random_state) { };
    long parent_id;
    long id;
    long level;
    RandomState random_state;
};

queue<PathInfo> path_info_queue;

// =============================================================================
// Path Processing
// =============================================================================

void process_path(PathInfo& path_info) {

    cout << endl;
    cout << "Process Path " << path_info.id << endl;

    // !!! PROCESSING

    double r = path_info.random_state.uniform_real(0.0, 1.0);
    cout << "random number: " << r << endl;

    if (path_info.level < N_LEVELS) {

        // Calculate the following paths ids using a formula (vs. using a shared counter)
        // to avoid synchronization between parallel executions of process_path().
        // lowest path id in current level
        long id_min_level = 0;
        for (long l=0; l<path_info.level; ++l) id_min_level += pow(N_PATHS, l);
        // lowest path id in next level
        long id_min_next_level = id_min_level + pow(N_PATHS, path_info.level);
        // first path id of following paths in next level
        long path_id = id_min_next_level + (path_info.id - id_min_level)*N_PATHS;

        // Enqueue path following paths information
        for (long p=0; p<(N_PATHS-1); ++p) {
            // Random Number Generator
            // new random states based on different seeds create with random state from current path
            unsigned long seed = path_info.random_state.uniform_int(0, path_info.random_state.MAX_INT);
            cout << "Child Seed " << p << ": " << seed << endl;
            RandomState random_state = RandomState(seed);
            path_info_queue.emplace(PathInfo(path_info.id, path_id + p, path_info.level + 1, random_state));
            // (parent_id, id, level, random_state)
        }
        // Pass on random state from current path to one of the following paths
        // after the random state has been used to generate new seeds for the following paths
        // to make sure that different seeds are generated in the following path.
        path_info_queue.emplace(PathInfo(path_info.id, path_id + (N_PATHS-1), path_info.level+1, path_info.random_state));
        // (parent_id, id, level, random_state)
    }

    return;
}

// =============================================================================
// Multi Path Processing Program
// =============================================================================

int main() {

    // !!! INPUT

    // Random Number Generator
    unsigned long seed = 0; // fixed seed for reproducibility, otherwise use RandomState()
    cout << "Root Seed: " << seed << endl;
    RandomState random_state = RandomState(seed); // root path

    // Multi Paths Data
    // shared memory for process_path() for writing and reading final and shared results

    // !!!

    // Enqueue root path information
    path_info_queue.emplace(PathInfo(-1, 0, 0, random_state));
    // (parent_id -1 for no parent, id, level, random_state)

    // SERIAL IMPLEMENTATION
    // can easily be parallelized:
    // as all paths that are in the queue can be processed in parallel!

    // Loop: all paths
    while (!path_info_queue.empty()) {

        // Dequeue path information
        PathInfo path_info = path_info_queue.front();
        path_info_queue.pop();

        // Process path
        process_path(path_info);
    }

    // !!! OUTPUT

    return 0;
}