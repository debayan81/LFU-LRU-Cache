#ifndef CACHE_IMPL_H
#define CACHE_IMPL_H

#include <iostream>
#include <unordered_map>
#include <list>
#include <climits>

// Node structure for both LFU and LRU caches
struct CacheNode {
    int key;
    int value;
    int freq; // Only used for LFU cache
    CacheNode(int k, int v, int f = 1) : key(k), value(v), freq(f) {}
};

/******************************
 * LRU Cache Implementation
 ******************************/
class LRUCache {
private:
    int capacity;
    std::list<CacheNode> cacheList;
    std::unordered_map<int, std::list<CacheNode>::iterator> cacheMap;

public:
    // Constructor
    LRUCache(int cap);
    
    // Get value by key
    int get(int key);
    
    // Put key-value pair into cache
    void put(int key, int value);
    
    // Utility function to print cache (for debugging)
    void printCache() const;
};

/******************************
 * LFU Cache Implementation
 ******************************/
class LFUCache {
private:
    int capacity;
    int minFreq;
    std::unordered_map<int, std::list<CacheNode>::iterator> keyMap;
    std::unordered_map<int, std::list<CacheNode>> freqMap;

public:
    // Constructor
    LFUCache(int cap);
    
    // Get value by key
    int get(int key);
    
    // Put key-value pair into cache
    void put(int key, int value);
    
    // Utility function to print cache (for debugging)
    void printCache() const;
};

#endif // CACHE_IMPL_H