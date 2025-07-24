# LFU and LRU Cache Implementation in C++

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

An efficient implementation of Least Frequently Used (LFU) and Least Recently Used (LRU) cache algorithms using doubly linked lists and hash maps in C++.

## Features

- O(1) time complexity for both get and put operations
- Clean, modular C++ implementation
- Separate classes for LFU and LRU implementations
- Example usage provided
- Comprehensive documentation

## Implementation Details

### Data Structures Used

- **Doubly Linked List**: For maintaining access order (LRU) and frequency groups (LFU)
- **Hash Maps**: For O(1) access to cache elements

### Complexity Analysis

| Operation | LRU Cache | LFU Cache |
|-----------|-----------|-----------|
| get()     | O(1)      | O(1)      |
| put()     | O(1)      | O(1)      |

## How to Use

### Prerequisites

- C++ compiler (C++11 or later)
- CMake (optional, for building)

### Basic Usage

```cpp
#include "cache_impl.h"

int main() {
    // LRU Cache example
    LRUCache lruCache(2);
    lruCache.put(1, 1);
    lruCache.put(2, 2);
    cout << lruCache.get(1);    // returns 1
    
    // LFU Cache example
    LFUCache lfuCache(2);
    lfuCache.put(1, 1);
    lfuCache.put(2, 2);
    cout << lfuCache.get(1);    // returns 1
}
