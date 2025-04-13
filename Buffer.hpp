#ifndef BUFFER_PAGE_HPP
#define BUFFER_PAGE_HPP

#include <iomanip>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cstring>

#define PAGE_SIZE 4096  // 4 KB page size

// Enum to distinguish between different page types
enum PageType {
    INDEX_PAGE,
    DATA_PAGE
};

// Buffer_Page class to represent pages in memory
class Buffer_Page {
public:
    int pageId;             // Page ID
    PageType type;          // Type of page (INDEX_PAGE or DATA_PAGE)
    bool referenceBit;      // Reference bit for the Clock algorithm
    bool dirtyBit;          // Dirty bit to track if the page has been modified
    int pinCount;           // Pin count to track the number of users of the page
    char data[PAGE_SIZE];   // Actual page data

    // Constructor
    Buffer_Page(int id, PageType t);

    // Function to simulate writing page to disk (if dirty)
    void writeToDisk();

    // Function to simulate reading page from disk
    static Buffer_Page* readFromDisk(int pageId, PageType type);
};

// Buffer pool class to manage pages in memory
class BufferPool {
private:
    int bufferSize;
    std::vector<Buffer_Page*> bufferPool;  // Circular buffer
    std::unordered_map<int, Buffer_Page*> pageTable;  // Map to track pages in memory
    int clockHand;  // Pointer for the clock algorithm

public:
    // Constructor
    BufferPool(int size);

    // Destructor
    ~BufferPool();

    // Function to request a page
    Buffer_Page* requestPage(int pageId, PageType type, bool isWrite = false);

    // Function to release a page (decrement pin count)
    void releasePage(int pageId);

    // Function to display the buffer pool state (for debugging)
    void displayBufferPool() const;

    // Function to get a new page ID
    int getNewPageId();

private:
    // Function to replace a page using the Clock Algorithm
    Buffer_Page* replacePage(int pageId, PageType type, bool isWrite);
};

#endif // BUFFER_PAGE_HPP