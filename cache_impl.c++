#include <iostream>
#include <unordered_map>
#include <list>
#include <climits>

using namespace std;

// Node structure for both LFU and LRU
struct Node {
    int key;
    int value;
    int freq; // Only used for LFU
    Node(int k, int v, int f = 1) : key(k), value(v), freq(f) {}
};

// LRU Cache Implementation
class LRUCache {
private:
    int capacity;
    list<Node> cacheList;
    unordered_map<int, list<Node>::iterator> cacheMap;

public:
    LRUCache(int cap) : capacity(cap) {}

    int get(int key) {
        if (cacheMap.find(key) == cacheMap.end()) {
            return -1; // Key not found
        }
        
        // Move the accessed node to front
        cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
        cacheMap[key] = cacheList.begin();
        return cacheMap[key]->value;
    }

    void put(int key, int value) {
        if (capacity == 0) return;
        
        if (cacheMap.find(key) != cacheMap.end()) {
            // Key exists, update value and move to front
            cacheMap[key]->value = value;
            cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
            cacheMap[key] = cacheList.begin();
            return;
        }
        
        if (cacheList.size() == capacity) {
            // Remove least recently used
            int lastKey = cacheList.back().key;
            cacheMap.erase(lastKey);
            cacheList.pop_back();
        }
        
        // Add new node to front
        cacheList.emplace_front(key, value);
        cacheMap[key] = cacheList.begin();
    }
};

// LFU Cache Implementation
class LFUCache {
private:
    int capacity;
    int minFreq;
    unordered_map<int, list<Node>::iterator> keyMap;
    unordered_map<int, list<Node>> freqMap;

public:
    LFUCache(int cap) : capacity(cap), minFreq(0) {}

    int get(int key) {
        if (capacity == 0 || keyMap.find(key) == keyMap.end()) {
            return -1;
        }
        
        // Get the node and its frequency
        auto node = keyMap[key];
        int value = node->value;
        int freq = node->freq;
        
        // Remove from current frequency list
        freqMap[freq].erase(node);
        if (freqMap[freq].empty()) {
            freqMap.erase(freq);
            if (minFreq == freq) {
                minFreq++;
            }
        }
        
        // Insert into new frequency list
        freqMap[freq + 1].emplace_front(key, value, freq + 1);
        keyMap[key] = freqMap[freq + 1].begin();
        
        return value;
    }

    void put(int key, int value) {
        if (capacity == 0) return;
        
        if (keyMap.find(key) != keyMap.end()) {
            // Key exists, update value and frequency
            auto node = keyMap[key];
            int freq = node->freq;
            
            // Remove from current frequency list
            freqMap[freq].erase(node);
            if (freqMap[freq].empty()) {
                freqMap.erase(freq);
                if (minFreq == freq) {
                    minFreq++;
                }
            }
            
            // Insert into new frequency list
            freqMap[freq + 1].emplace_front(key, value, freq + 1);
            keyMap[key] = freqMap[freq + 1].begin();
            return;
        }
        
        if (keyMap.size() == capacity) {
            // Remove least frequently used (and least recently used if multiple)
            auto& minFreqList = freqMap[minFreq];
            int keyToRemove = minFreqList.back().key;
            minFreqList.pop_back();
            keyMap.erase(keyToRemove);
            
            if (minFreqList.empty()) {
                freqMap.erase(minFreq);
            }
        }
        
        // Add new node with frequency 1
        freqMap[1].emplace_front(key, value, 1);
        keyMap[key] = freqMap[1].begin();
        minFreq = 1;
    }
};

// Test function
void testCaches() {
    cout << "Testing LRU Cache:\n";
    LRUCache lru(2);
    lru.put(1, 1);
    lru.put(2, 2);
    cout << lru.get(1) << endl; // returns 1
    lru.put(3, 3); // evicts key 2
    cout << lru.get(2) << endl; // returns -1 (not found)
    lru.put(4, 4); // evicts key 1
    cout << lru.get(1) << endl; // returns -1 (not found)
    cout << lru.get(3) << endl; // returns 3
    cout << lru.get(4) << endl; // returns 4

    cout << "\nTesting LFU Cache:\n";
    LFUCache lfu(2);
    lfu.put(1, 1);
    lfu.put(2, 2);
    cout << lfu.get(1) << endl; // returns 1
    lfu.put(3, 3); // evicts key 2
    cout << lfu.get(2) << endl; // returns -1 (not found)
    cout << lfu.get(3) << endl; // returns 3
    lfu.put(4, 4); // evicts key 1
    cout << lfu.get(1) << endl; // returns -1 (not found)
    cout << lfu.get(3) << endl; // returns 3
    cout << lfu.get(4) << endl; // returns 4
}

int main() {
    testCaches();
    return 0;
}
