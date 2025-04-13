#ifndef BUFFER_PAGE_HPP
#define BUFFER_PAGE_HPP

#include <iomanip>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cstring>

#define PAGE_SIZE 4096  


enum PageType {
    INDEX_PAGE,
    DATA_PAGE
};


class Buffer_Page {
public:
    int pageId;             
    PageType type;          
    bool referenceBit;      
    bool dirtyBit;          
    int pinCount;           
    char data[PAGE_SIZE];   

    
    Buffer_Page(int id, PageType t);

    
    void writeToDisk();

    
    static Buffer_Page* readFromDisk(int pageId, PageType type);
};


class BufferPool {
private:
    int bufferSize;
    std::vector<Buffer_Page*> bufferPool;  
    std::unordered_map<int, Buffer_Page*> pageTable;  
    int clockHand;  

public:
    
    BufferPool(int size);

    
    ~BufferPool();

    
    Buffer_Page* requestPage(int pageId, PageType type, bool isWrite = false);

    
    void releasePage(int pageId);

    
    void displayBufferPool() const;

    
    int getNewPageId();

private:
    
    Buffer_Page* replacePage(int pageId, PageType type, bool isWrite);
};

#endif 