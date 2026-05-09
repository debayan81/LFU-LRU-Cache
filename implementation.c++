#include <iostream>
#include <unordered_map>
#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

using namespace std;

// ==========================================
// 1. Metrics & Utilities
// ==========================================

struct CacheMetrics {
    size_t hits = 0;
    size_t misses = 0;
    size_t evictions = 0;

    double hitRate() const {
        size_t total = hits + misses;
        return total == 0 ? 0.0 : (double)hits / total * 100.0;
    }

    void print() const {
        cout << "Hits: " << hits << " | Misses: " << misses 
             << " | Evictions: " << evictions 
             << " | Hit Rate: " << fixed << setprecision(2) << hitRate() << "%\n";
    }
};

// ==========================================
// 2. Thread-Safe LRU Cache
// ==========================================

template <typename K, typename V>
class LRUCache {
private:
    struct Node {
        K key;
        V value;
        Node(K k, V v) : key(k), value(v) {}
    };

    size_t capacity;
    list<Node> cacheList;
    unordered_map<K, typename list<Node>::iterator> cacheMap;
    
    mutable mutex mtx; // Protects concurrent access
    CacheMetrics metrics;

public:
    LRUCache(size_t cap) : capacity(cap) {}

    bool get(const K& key, V& outValue) {
        lock_guard<mutex> lock(mtx);
        
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            metrics.misses++;
            return false;
        }

        metrics.hits++;
        cacheList.splice(cacheList.begin(), cacheList, it->second);
        outValue = it->second->value;
        return true;
    }

    void put(const K& key, const V& value) {
        if (capacity == 0) return;
        lock_guard<mutex> lock(mtx);

        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            it->second->value = value;
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            return;
        }

        if (cacheMap.size() == capacity) {
            metrics.evictions++;
            cacheMap.erase(cacheList.back().key);
            cacheList.pop_back();
        }

        cacheList.emplace_front(key, value);
        cacheMap[key] = cacheList.begin();
    }

    CacheMetrics getMetrics() const {
        lock_guard<mutex> lock(mtx);
        return metrics;
    }
};

// ==========================================
// 3. Thread-Safe LFU Cache
// ==========================================

template <typename K, typename V>
class LFUCache {
private:
    struct Node {
        K key;
        V value;
        size_t freq;
        Node(K k, V v, size_t f = 1) : key(k), value(v), freq(f) {}
    };

    size_t capacity;
    size_t minFreq;
    unordered_map<K, typename list<Node>::iterator> keyMap;
    unordered_map<size_t, list<Node>> freqMap;

    mutable mutex mtx;
    CacheMetrics metrics;

public:
    LFUCache(size_t cap) : capacity(cap), minFreq(0) {}

    bool get(const K& key, V& outValue) {
        lock_guard<mutex> lock(mtx);
        
        if (capacity == 0 || keyMap.find(key) == keyMap.end()) {
            metrics.misses++;
            return false;
        }

        metrics.hits++;
        auto nodeIt = keyMap[key];
        outValue = nodeIt->value;
        size_t freq = nodeIt->freq;

        freqMap[freq].erase(nodeIt);
        if (freqMap[freq].empty()) {
            freqMap.erase(freq);
            if (minFreq == freq) minFreq++;
        }

        freqMap[freq + 1].emplace_front(key, outValue, freq + 1);
        keyMap[key] = freqMap[freq + 1].begin();
        return true;
    }

    void put(const K& key, const V& value) {
        if (capacity == 0) return;
        lock_guard<mutex> lock(mtx);

        auto it = keyMap.find(key);
        if (it != keyMap.end()) {
            auto nodeIt = it->second;
            size_t freq = nodeIt->freq;
            
            freqMap[freq].erase(nodeIt);
            if (freqMap[freq].empty()) {
                freqMap.erase(freq);
                if (minFreq == freq) minFreq++;
            }

            freqMap[freq + 1].emplace_front(key, value, freq + 1);
            keyMap[key] = freqMap[freq + 1].begin();
            return;
        }

        if (keyMap.size() == capacity) {
            metrics.evictions++;
            auto& minFreqList = freqMap[minFreq];
            K keyToRemove = minFreqList.back().key;
            minFreqList.pop_back();
            keyMap.erase(keyToRemove);
            
            if (minFreqList.empty()) {
                freqMap.erase(minFreq);
            }
        }

        freqMap[1].emplace_front(key, value, 1);
        keyMap[key] = freqMap[1].begin();
        minFreq = 1;
    }

    CacheMetrics getMetrics() const {
        lock_guard<mutex> lock(mtx);
        return metrics;
    }
};

// ==========================================
// 4. Multi-Threaded Workload Simulator
// ==========================================

// Simulates a realistic workload where 80% of operations access 20% of the keys (Pareto Principle)
template <typename CacheType>
void runSimulation(CacheType& cache, int numThreads, int opsPerThread, int keyRange) {
    auto worker = [&](int seed) {
        mt19937 gen(seed);
        uniform_int_distribution<> actionDist(1, 100);
        
        // 80/20 rule distribution
        uniform_int_distribution<> hotKeys(1, keyRange * 0.2);
        uniform_int_distribution<> coldKeys(keyRange * 0.2 + 1, keyRange);
        uniform_int_distribution<> localityDist(1, 100);

        for (int i = 0; i < opsPerThread; ++i) {
            int key = (localityDist(gen) <= 80) ? hotKeys(gen) : coldKeys(gen);
            
            // 70% Reads, 30% Writes
            if (actionDist(gen) <= 70) {
                string val;
                cache.get(key, val);
            } else {
                cache.put(key, "DataPayload_" + to_string(key));
            }
        }
    };

    vector<thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i + 42);
    }

    for (auto& t : threads) {
        t.join();
    }
}

// ==========================================
// 5. Main Execution
// ==========================================

int main() {
    const int CACHE_CAPACITY = 1000;
    const int NUM_THREADS = 8;
    const int OPS_PER_THREAD = 25000; // 200k total operations
    const int KEY_RANGE = 5000;

    cout << "--- Starting Cache Simulation Workload ---\n";
    cout << "Configuration: " << NUM_THREADS << " Threads, " 
         << (NUM_THREADS * OPS_PER_THREAD) << " Total Ops, Capacity: " << CACHE_CAPACITY << "\n\n";

    // Test LRU
    LRUCache<int, string> lru(CACHE_CAPACITY);
    auto startLRU = chrono::high_resolution_clock::now();
    runSimulation(lru, NUM_THREADS, OPS_PER_THREAD, KEY_RANGE);
    auto endLRU = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> durationLRU = endLRU - startLRU;

    cout << "[LRU Simulator Results]\n";
    lru.getMetrics().print();
    cout << "Time taken: " << durationLRU.count() << " ms\n\n";

    // Test LFU
    LFUCache<int, string> lfu(CACHE_CAPACITY);
    auto startLFU = chrono::high_resolution_clock::now();
    runSimulation(lfu, NUM_THREADS, OPS_PER_THREAD, KEY_RANGE);
    auto endLFU = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> durationLFU = endLFU - startLFU;

    cout << "[LFU Simulator Results]\n";
    lfu.getMetrics().print();
    cout << "Time taken: " << durationLFU.count() << " ms\n";

    return 0;
}
