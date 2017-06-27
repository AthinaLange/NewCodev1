#include "MultiPathProcessing.h"

// Global Data
PathInfoQueue_t  path_info_queue;
PathDataVector_t multi_paths_data;

// =============================================================================
// Path Processing
// =============================================================================

void process_path(PathInfo& path_info) {

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

    long clock = path_info.clock;
    while (clock < N_CLOCKS) {
        double r = path_info.random_state.uniform_real(0.0, 1.0);
        cout << "clock: " << clock << " random number: " << r << endl;
        clock++;
        if (r > 0.95) break;
    }

    // Write PathData
    multi_paths_data[path_info.id].valid = true;
    multi_paths_data[path_info.id].parent_id = path_info.parent_id;
    for (long i=0; i< multi_paths_data[path_info.id].n_data1D; ++i) {
        multi_paths_data[path_info.id].data1D[i] = path_info.id;
    }
    for (long i=0; i< multi_paths_data[path_info.id].n_data2D_1; ++i) {
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
            RandomState random_state = RandomState(seed);
            path_info_queue.emplace(PathInfo(path_info.id, path_id + p, path_info.level + 1, clock, random_state));
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


