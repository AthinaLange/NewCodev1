#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <time.h>
#include "random.h"

using namespace std;

// =============================================================================
// Global
// =============================================================================

//const int N_PATHS  = 2;
//const int N_LEVELS = 2;
//const int N_CLOCKS = 20;

int N_PATHS;
int N_LEVELS;
int N_CLOCKS;


// =============================================================================
// !!! Multi Paths Data
// =============================================================================

// Shared memory for process_path()for writing and reading final and shared results.

// No data protection is needed when parallelizing process_path(),
// because only one path is writing the data, and only following paths,
// that do not exist yet when the data is written, will only read the data.

// Can be implemented as C-style dynamic memory (malloc, free),
// C++-style dynamic memory (new, delete []), unique pointers (C++11), or
// STL containers, like vectors, arrays (C++11), lists.

// This example of a vector implementation assumes
// that the PathData for each path has the same dimension.

struct PathData { //initial distribution (what everyone needs to know/can read)
    PathData(long n_data1D, long n_data2D_1, long n_data2D_2) // dimension parameters of the PathData
            : n_data1D(n_data1D), n_data2D_1(n_data2D_1), n_data2D_2(n_data2D_2),
              valid(false), parent_id(-1), data1D(n_data1D, 0.0), data2D(n_data2D_1, vector<double>(n_data2D_2, 0.0)) { }
    //constructor(parameters):initialization
    //values given when PathData constructed (malloc) and initialized
    //vectors given dynamic memory - data1D (dimension, initial value)

    // memory is allocated using constructor initialization
    // explicit initialization of vectors with 0.0
    bool                    valid; // identify processed paths
                                   // given that N_CLOCKS is a stop criteria, not all levels might be reached
    long                    parent_id;
    vector<double>          data1D;
    vector<vector<double>>  data2D;
    long                    n_data1D; // dimension parameters of the PathData
    long                    n_data2D_1;
    long                    n_data2D_2;
};

vector<PathData> multi_paths_data; //all data is in here


// =============================================================================
// Path Processing Queue (FIFO)
// =============================================================================

// A queue (FIFO) provides breath first processing of the path structure (tree),
// vs. a stack (LIFO) that provides depth first processing.
// Breath first processing is better for parallel processing,
// as it adds more paths to the processing queue quicker.

struct PathInfo {
    PathInfo(long parent_id, long id, long level, long clock, RandomState random_state)
    : parent_id(parent_id), id(id), level(level), clock(clock), random_state(random_state) { };
    long parent_id;
    long id;
    long level;
    long clock;
    RandomState random_state;
};

queue<PathInfo> path_info_queue;

// =============================================================================
// Path Processing
// =============================================================================

void process_path(PathInfo& path_info) { //& - reference

    cout << endl;
    cout << "Process Path " << path_info.id << " (level " << path_info.level << " )" << endl;

    // Read ancestors PathData
    long ancestor_id = path_info.parent_id;
    while (ancestor_id >= 0) { // root path id is 0, parent of root path id is -1
        cout << "ancestor: " << ancestor_id << endl;
        // How to access PathData
        // multi_paths_data[ancestor_id].valid ... always valid
        // multi_paths_data[ancestor_id].parent_id
        // multi_paths_data[ancestor_id].data1D[i]
        // multi_paths_data[ancestor_id].data2D[i][j]
        ancestor_id = multi_paths_data[ancestor_id].parent_id; // next ancestor
    }

    // !!! PROCESSING
    //jump condition example - replace with what we want
    long clock = path_info.clock;
    while (clock < N_CLOCKS) {
        double r = path_info.random_state.uniform_real(0.0, 1.0);
        cout << "clock: " << clock << " random number: " << r << en4dl;
        clock++;
        if (r > 0.95) break;
    }

    // Write PathData
    multi_paths_data[path_info.id].valid = true;
    multi_paths_data[path_info.id].parent_id = path_info.parent_id;
    for (long i=0; i < multi_paths_data[path_info.id].n_data1D; ++i) {
        multi_paths_data[path_info.id].data1D[i] = path_info.id;
    }
    for (long i=0; i < multi_paths_data[path_info.id].n_data2D_1; ++i) {
        for (long j=0; j < multi_paths_data[path_info.id].n_data2D_2; ++j) {
            multi_paths_data[path_info.id].data2D[i][j] = path_info.id;
        }
    }

    if ((path_info.level < N_LEVELS) &&  (clock < N_CLOCKS)) {

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
            RandomState temp_random_state = RandomState(seed); //this exists only inside this for loop
            path_info_queue.emplace(PathInfo(path_info.id, path_id + p, path_info.level + 1, clock, temp_random_state));
            // (parent_id, id, level, clock, random_state)
        }
        // Pass on random state from current path to one of the following paths
        // after the random state has been used to generate new seeds for the following paths
        // to make sure that different seeds are generated in the following path.
        path_info_queue.emplace(PathInfo(path_info.id, path_id + (N_PATHS-1), path_info.level+1, clock, path_info.random_state));
        // (parent_id, id, level, clock, random_state)
    }

    return;
}

// =============================================================================
// Multi Path Processing Program
// =============================================================================

int main() {

    // !!! INPUT
    cout << "Input system parameters:" << endl;
    cout << "Number of Paths, Number of Levels, Timestep" << endl;
    cin >> N_PATHS >> N_LEVELS >> N_CLOCKS;

    // Random Number Generator
    unsigned long seed = 0; // fixed seed for reproducibility, otherwise use RandomState()
    cout << "Root Seed: " << seed << endl;
    RandomState random_state = RandomState(seed); // root path

    // !!! Multi Paths Data
    long n_data1D   = 5; // dimension parameters
    long n_data2D_1 = 5;
    long n_data2D_2 = 6;
    long n_paths = pow(N_PATHS, (N_LEVELS+1.0)) - 1;

    multi_paths_data.resize(n_paths, PathData(n_data1D, n_data2D_1, n_data2D_2)); //similar to malloc  //(new size/how many paths, initialization of new data structure)

    // Enqueue root path information
    path_info_queue.emplace(PathInfo(-1, 0, 0, 0, random_state));
    // (parent_id -1 for no parent, id, level, clock, random_state)

    // SERIAL IMPLEMENTATION
    // can easily be parallelized:
    // as all paths that are in the queue can be processed in parallel!

    // Loop: all paths
    while (!path_info_queue.empty()) {

        // Dequeue path information
        PathInfo path_info = path_info_queue.front(); //reads first element
        path_info_queue.pop(); //removes first element

        // Process path
        process_path(path_info);
    }

    // !!! OUTPUT

    long path = n_paths-1; // choose any one between 0 and n_paths-1

    cout << endl;
    cout << "path " << path << endl;

    cout << "valid: " << multi_paths_data[path].valid << endl;
    cout << "parent_id: " << multi_paths_data[path].parent_id << endl;
    for (long i=0; i< multi_paths_data[path].n_data1D; ++i) {
        cout << "multi_paths_data[" << path << "].data1D[" << i << "]: " << multi_paths_data[path].data1D[i] << endl;
    }
    for (long i=0; i< multi_paths_data[path].n_data2D_1; ++i) {
        for (long j=0; j < multi_paths_data[path].n_data2D_2; ++j) {
            cout << "multi_paths_data[" << path << "].data2D[" << i << "][" << j << "]: " << multi_paths_data[path].data2D[i][j] << endl;
        }
    }

    // !!! Multi Paths Data
    // When implemented as C-style dynamic memory (malloc, free) or
    // C++-style dynamic memory (new, delete []),
    // memory needs to be de-allocated explicitly here.

    return 0;
}