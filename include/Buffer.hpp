#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "page.hpp"

#define BUFFER_POOL_SIZE 10

struct Frame {
    Page* page;
    std::string dbName;
    std::string tableName;
    int pageId;
    bool referenceBit;
    bool dirtyBit;
    int pinCount;

    Frame();
    void reset();
};

class BufferPool {
public:
    BufferPool(int size = BUFFER_POOL_SIZE);
    ~BufferPool();

    Page* fetchPage(int pageId, const std::string& dbName, const std::string& tableName);

    void markDirty(int pageId, const std::string& dbName, const std::string& tableName);

    void unpinPage(int pageId, const std::string& dbName, const std::string& tableName);

    void flushPage(int pageId, const std::string& dbName, const std::string& tableName);

    void flushAll();

    void displayBufferPool() const;

private:
    int poolSize;
    std::vector<Frame> frames;
    int clockHand;

    static std::string makeKey(int pageId, const std::string& dbName, const std::string& tableName);

    std::unordered_map<std::string, int> pageTable;

    int findVictim();

    void writeToDisk(int frameIdx);

    void loadFromDisk(int frameIdx, int pageId, const std::string& dbName, const std::string& tableName);
};

#endif
