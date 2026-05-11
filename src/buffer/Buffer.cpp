#include "Buffer.hpp"
#include "page.hpp"
#include <iostream>
#include <iomanip>

Frame::Frame()
    : page(nullptr), pageId(-1), referenceBit(false), dirtyBit(false), pinCount(0) {}

void Frame::reset() {
    delete page;
    page = nullptr;
    dbName.clear();
    tableName.clear();
    pageId = -1;
    referenceBit = false;
    dirtyBit = false;
    pinCount = 0;
}

BufferPool::BufferPool(int size) : poolSize(size), clockHand(0) {
    frames.resize(poolSize);
}

BufferPool::~BufferPool() {
    flushAll();
    for (auto& frame : frames) {
        delete frame.page;
        frame.page = nullptr;
    }
}

std::string BufferPool::makeKey(int pageId, const std::string& dbName, const std::string& tableName) {
    return dbName + "/" + tableName + ":" + std::to_string(pageId);
}

Page* BufferPool::fetchPage(int pageId, const std::string& dbName, const std::string& tableName) {
    std::string key = makeKey(pageId, dbName, tableName);

    auto it = pageTable.find(key);
    if (it != pageTable.end()) {
        Frame& frame = frames[it->second];
        frame.referenceBit = true;
        frame.pinCount++;
        return frame.page;
    }

    int frameIdx = findVictim();
    if (frameIdx == -1) {
        std::cerr << "BufferPool: All pages are pinned, cannot evict.\n";
        return nullptr;
    }

    Frame& victim = frames[frameIdx];
    if (victim.page != nullptr) {
        writeToDisk(frameIdx);
        std::string oldKey = makeKey(victim.pageId, victim.dbName, victim.tableName);
        pageTable.erase(oldKey);
        victim.reset();
    }

    loadFromDisk(frameIdx, pageId, dbName, tableName);
    pageTable[key] = frameIdx;

    Frame& frame = frames[frameIdx];
    frame.referenceBit = true;
    frame.pinCount = 1;
    return frame.page;
}

void BufferPool::markDirty(int pageId, const std::string& dbName, const std::string& tableName) {
    std::string key = makeKey(pageId, dbName, tableName);
    auto it = pageTable.find(key);
    if (it != pageTable.end()) {
        frames[it->second].dirtyBit = true;
    }
}

void BufferPool::unpinPage(int pageId, const std::string& dbName, const std::string& tableName) {
    std::string key = makeKey(pageId, dbName, tableName);
    auto it = pageTable.find(key);
    if (it != pageTable.end()) {
        Frame& frame = frames[it->second];
        if (frame.pinCount > 0) {
            frame.pinCount--;
        }
    }
}

void BufferPool::flushPage(int pageId, const std::string& dbName, const std::string& tableName) {
    std::string key = makeKey(pageId, dbName, tableName);
    auto it = pageTable.find(key);
    if (it != pageTable.end()) {
        writeToDisk(it->second);
    }
}

void BufferPool::flushAll() {
    for (int i = 0; i < poolSize; i++) {
        if (frames[i].page != nullptr && frames[i].dirtyBit) {
            writeToDisk(i);
        }
    }
}

int BufferPool::findVictim() {

    int fullSweeps = 0;
    while (fullSweeps < 2) {
        Frame& frame = frames[clockHand];

        if (frame.page == nullptr) {
            int idx = clockHand;
            clockHand = (clockHand + 1) % poolSize;
            return idx;
        }

        if (frame.pinCount == 0) {
            if (!frame.referenceBit) {
                int idx = clockHand;
                clockHand = (clockHand + 1) % poolSize;
                return idx;
            } else {
                frame.referenceBit = false;
            }
        }

        clockHand = (clockHand + 1) % poolSize;
        if (clockHand == 0) {
            fullSweeps++;
        }
    }
    return -1;
}

void BufferPool::writeToDisk(int frameIdx) {
    Frame& frame = frames[frameIdx];
    if (frame.page != nullptr && frame.dirtyBit) {
        frame.page->serialize(frame.pageId, frame.dbName, frame.tableName);
        frame.dirtyBit = false;
    }
}

void BufferPool::loadFromDisk(int frameIdx, int pageId,
                              const std::string& dbName, const std::string& tableName) {
    Frame& frame = frames[frameIdx];
    frame.page = new Page(PAGE_SIZE, {0, 0});
    frame.dbName = dbName;
    frame.tableName = tableName;
    frame.pageId = pageId;
    frame.dirtyBit = false;
    frame.referenceBit = false;
    frame.pinCount = 0;

    Page::deserialize(frame.page, pageId, dbName, tableName);
}

void BufferPool::displayBufferPool() const {
    std::cout << "\n+-------+---------------------+--------+-------+-------+--------+\n";
    std::cout << "| Frame |        Page         | Pinned | Dirty | Ref   | Tuples |\n";
    std::cout << "+-------+---------------------+--------+-------+-------+--------+\n";

    for (int i = 0; i < poolSize; i++) {
        const Frame& f = frames[i];
        if (f.page == nullptr) {
            std::cout << "| " << std::setw(5) << i
                      << " | " << std::setw(19) << "(empty)"
                      << " | " << std::setw(6) << "-"
                      << " | " << std::setw(5) << "-"
                      << " | " << std::setw(5) << "-"
                      << " | " << std::setw(6) << "-"
                      << " |\n";
        } else {
            std::string pageDesc = f.tableName + ":" + std::to_string(f.pageId);
            std::cout << "| " << std::setw(5) << i
                      << " | " << std::setw(19) << pageDesc
                      << " | " << std::setw(6) << f.pinCount
                      << " | " << std::setw(5) << (f.dirtyBit ? "Y" : "N")
                      << " | " << std::setw(5) << (f.referenceBit ? "Y" : "N")
                      << " | " << std::setw(6) << f.page->tuples.size()
                      << " |\n";
        }
    }
    std::cout << "+-------+---------------------+--------+-------+-------+--------+\n";
}
