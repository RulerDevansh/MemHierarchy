/**
 * csim.cpp
 *
 * Cache Simulator
 *
 * Ifrah Attar
 * iattar1@jh.edu
 *
 * Morgan Huberty
 * mhubert3@jh.edu
 */

 #include <iostream>
 #include <vector>
 #include <string>
 #include <sstream>
 #include <cmath>
 #include <cstdint>
 #include <stdexcept>
 #include <limits> // for std::numeric_limits
 
 // Data Structures
 
 /**
  * @brief Represents a single line (or block) in the cache
  */
 struct CacheLine {
     bool valid = false;
     bool dirty = false;
     uint64_t tag = 0;
     
     // Counter for LRU is updated on each access
     // Higher value means more recent
     uint64_t lru_counter = 0;
 
     // Timestamp for FIFO is set only when a block is loaded
     uint64_t load_ts = 0;
 };
 
 /**
  * @brief Represents a set in the cache, containing one or more cache lines
  */
 using CacheSet = std::vector<CacheLine>;
 
 /**
  * @brief Holds all configuration parameters for cache simulator
  */
 struct Config {
     uint32_t num_sets;
     uint32_t blocks_per_set;
     uint32_t bytes_per_block;
     bool write_allocate;
     bool write_back;
     std::string eviction_policy; // "lru" or "fifo"
 };
 
 /**
  * @brief Holds all statistics gathered during simulation
  */
 struct Stats {
     uint64_t total_loads = 0;
     uint64_t total_stores = 0;
     uint64_t load_hits = 0;
     uint64_t load_misses = 0;
     uint64_t store_hits = 0;
     uint64_t store_misses = 0;
     uint64_t total_cycles = 0;
 };
 
 
 // Helper Functions
 
 /**
  * @brief Checks if a number is a positive power of 2
  * @param n The number to check
  * @return True if n is a power of 2, false otherwise
  */
 bool isPowerOfTwo(uint32_t n) {
     return (n > 0) && ((n & (n - 1)) == 0);
 }
 
 /**
  * @brief Parses command line arguments and validates
  * @param argc Argument count
  * @param argv Argument values
  * @return Config struct with parsed parameters
  * @throws std::runtime_error on invalid parameters
  */
 Config parse_and_validate_args(int argc, char* argv[]) {
    // Check for the correct number of command-line arguments 
    if (argc != 7) {
         throw std::runtime_error("Invalid number of arguments. Expected 6.");
     }
 
     Config config;
     // Parse the command-line arguments and store them in the config struct
     try {
         config.num_sets = std::stoul(argv[1]);
         config.blocks_per_set = std::stoul(argv[2]);
         config.bytes_per_block = std::stoul(argv[3]);
     } catch (const std::invalid_argument& e) {
         throw std::runtime_error("Set, block, or byte count is not a valid number.");
     }
 
     // Parse the write-allocate and write-back policies
     std::string wa_policy = argv[4];
     std::string wb_policy = argv[5];
     config.eviction_policy = argv[6]; // Store the eviction policy directly
 
    // Validate the write-allocate policy
     if (wa_policy == "write-allocate") {
         config.write_allocate = true;
     } else if (wa_policy == "no-write-allocate") {
         config.write_allocate = false;
     } else {
         throw std::runtime_error("Invalid write-allocate policy.");
     }
 
    // Validate the write-through/write-back policy
     if (wb_policy == "write-back") {
         config.write_back = true;
     } else if (wb_policy == "write-through") {
         config.write_back = false;
     } else {
         throw std::runtime_error("Invalid write-through/back policy.");
     }
     
    // Validate the eviction policy
     if (config.eviction_policy != "lru" && config.eviction_policy != "fifo") {
         throw std::runtime_error("Invalid eviction policy. Must be 'lru' or 'fifo'.");
     }
 
    // Final Validation of Parameter Constraints

    // Check if set, block, or byte counts are positive powers of 2
    // Also, ensure block size is at least 4 bytes
     if (!isPowerOfTwo(config.num_sets) || !isPowerOfTwo(config.blocks_per_set) || 
         !isPowerOfTwo(config.bytes_per_block) || config.bytes_per_block < 4) {
         throw std::runtime_error("Set, block, or byte counts must be positive powers of 2 (block size >= 4).");
     }

    // Check for the invalid combination of write-back and no-write-allocate
     if (config.write_back && !config.write_allocate) {
         throw std::runtime_error("Invalid combination: write-back and no-write-allocate.");
     }
 
     // Return validated config
     return config;
 }
 
 
 // Core Simulation Logic
 
 /**
  * @brief Simulates a single memory access on cache
  * @param cache The cache data structure
  * @param config The cache configuration
  * @param stats The statistics to update
  * @param access_type 'l' for load, 's' for store
  * @param address The 32-bit memory address
  * @param time_counter A counter to track event order for LRU and FIFO
  */
 void simulate_access(
     std::vector<CacheSet>& cache,
     const Config& config,
     Stats& stats,
     char access_type,
     uint32_t address,
     uint64_t& time_counter) {
 
     // Decompose Address & Define Cycle Costs
     uint32_t offset_bits = log2(config.bytes_per_block);
     uint32_t index_bits = log2(config.num_sets);
     
     uint32_t set_index = (address >> offset_bits) & (config.num_sets - 1);
     uint64_t tag = address >> (offset_bits + index_bits);
 
     const uint64_t BLOCK_TRANSFER_CYCLES = 100 * (config.bytes_per_block / 4);
     // write-through or no-write-allocate store writes <= 4 bytes
     const uint64_t MEMORY_WRITE_CYCLES = 100;
 
     CacheSet& current_set = cache.at(set_index);
     int hit_index = -1;
 
     // Search for a Hit
     for (size_t i = 0; i < current_set.size(); ++i) {
         if (current_set[i].valid && current_set[i].tag == tag) {
             hit_index = i;
             break;
         }
     }
     
     // Handle a HIT
     if (hit_index != -1) {
         stats.total_cycles += 1; // 1 cycle for any cache access
         current_set[hit_index].lru_counter = time_counter;
 
         if (access_type == 'l') {
             stats.load_hits++;
         } else { // Store Hit
             stats.store_hits++;
             if (config.write_back) {
                 current_set[hit_index].dirty = true;
             } else { // Write-through writes to cache and memory
                 stats.total_cycles += MEMORY_WRITE_CYCLES;
             }
         }
     } 
     // Handle a MISS
     else {
         stats.total_cycles += 1; // 1 cycle for the failed cache check
 
         if (access_type == 'l') {
             stats.load_misses++;
             stats.total_cycles += BLOCK_TRANSFER_CYCLES; // Must fetch block from memory
         } else { // Store Miss
             stats.store_misses++;
             if (config.write_allocate) {
                 stats.total_cycles += BLOCK_TRANSFER_CYCLES; // Fetch block before writing
             } else { // No-write-allocate
                 stats.total_cycles += MEMORY_WRITE_CYCLES; // Write directly to memory
                 return; // Cache is not modified, so we are finished
             }
         }
         
         // Find Victim Line (for loads and write-allocate stores)
         int victim_index = -1;
         
         // Look for an invalid (empty) line
         for (size_t i = 0; i < current_set.size(); ++i) {
             if (!current_set[i].valid) {
                 victim_index = i;
                 break;
             }
         }
 
         // If no invalid line, find victim using eviction policy
         if (victim_index == -1) {
             // Corrected brace structure and implemented FIFO logic (previously had a bug)
             if (config.eviction_policy == "fifo") {
                 uint64_t min_load_ts = std::numeric_limits<uint64_t>::max();
                 for (size_t i = 0; i < current_set.size(); ++i) {
                     if (current_set[i].load_ts < min_load_ts) {
                         min_load_ts = current_set[i].load_ts;
                         victim_index = i;
                     }
                 }
             } else { // lru
                 uint64_t min_lru = std::numeric_limits<uint64_t>::max();
                 for (size_t i = 0; i < current_set.size(); ++i) {
                     if (current_set[i].lru_counter < min_lru) {
                         min_lru = current_set[i].lru_counter;
                         victim_index = i;
                     }
                 }
             }
         }
         
         CacheLine& victim_line = current_set.at(victim_index);
 
         // If victim is valid and dirty, write it back to memory
         if (victim_line.valid && victim_line.dirty) {
             stats.total_cycles += BLOCK_TRANSFER_CYCLES;
         }
 
         // Place new block in victim's spot
         victim_line.valid = true;
         victim_line.tag = tag;
         victim_line.lru_counter = time_counter;
         victim_line.load_ts = time_counter; // Set insertion time for FIFO
 
         // Set dirty bit based on policy
         victim_line.dirty = (access_type == 's' && config.write_back);
 
         // Added cycle cost for a write-allocate/write-through store miss
         // The write must still go through to main memory
         if (access_type == 's' && !config.write_back) {
             stats.total_cycles += MEMORY_WRITE_CYCLES;
         }
     }
 }
 
 
 // Main
 
 int main(int argc, char* argv[]) {
     Config config;
     try {
         config = parse_and_validate_args(argc, argv);
     } catch (const std::runtime_error& e) {
         std::cerr << "Error: " << e.what() << std::endl;
         return 1;
     }
 
     std::vector<CacheSet> cache(config.num_sets, CacheSet(config.blocks_per_set));
     Stats stats;
     uint64_t time_counter = 0;
 
     std::string line;
     while (std::getline(std::cin, line)) {
         if (line.empty() || line[0] == '#') continue;
 
         std::stringstream ss(line);
         char access_type;
         uint32_t address;
         uint32_t ignored_size;
         ss >> access_type >> std::hex >> address >> ignored_size;
 
         if (ss.fail()) continue;
         
         if (access_type == 'l') {
             stats.total_loads++;
         } else if (access_type == 's') {
             stats.total_stores++;
         } else {
             continue; // Ignore other trace line types
         }
         
         time_counter++; // Acts as a global timestamp
         simulate_access(cache, config, stats, access_type, address, time_counter);
     }
 
     // Print final statistics in required format
     std::cout << "Total loads: " << stats.total_loads << std::endl;
     std::cout << "Total stores: " << stats.total_stores << std::endl;
     std::cout << "Load hits: " << stats.load_hits << std::endl;
     std::cout << "Load misses: " << stats.load_misses << std::endl;
     std::cout << "Store hits: " << stats.store_hits << std::endl;
     std::cout << "Store misses: " << stats.store_misses << std::endl;
     std::cout << "Total cycles: " << stats.total_cycles << std::endl;
 
     return 0;
 }