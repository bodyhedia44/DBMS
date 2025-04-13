#include "Buffer.hpp"


Buffer_Page::Buffer_Page(int id, PageType t) : pageId(id), type(t), referenceBit(true), dirtyBit(false), pinCount(0) {
    std::memset(data, 0, PAGE_SIZE);  
}


void Buffer_Page::writeToDisk() {
    if (dirtyBit) {
        std::ofstream file("page_" + std::to_string(pageId) + ".dat", std::ios::binary);
        file.write(data, PAGE_SIZE);
        file.close();
        dirtyBit = false;  
    }
}


Buffer_Page* Buffer_Page::readFromDisk(int pageId, PageType type) {
    Buffer_Page* page = new Buffer_Page(pageId, type);
    std::ifstream file("page_" + std::to_string(pageId) + ".dat", std::ios::binary);
    if (file.is_open()) {
        file.read(page->data, PAGE_SIZE);
        file.close();
    }
    return page;
}


BufferPool::BufferPool(int size) : bufferSize(size), clockHand(0) {
    bufferPool.resize(bufferSize, nullptr);
}


BufferPool::~BufferPool() {
    
    for (Buffer_Page* page : bufferPool) {
        if (page) {
            page->writeToDisk();
            delete page;
        }
    }
}


int BufferPool::getNewPageId() {
    std::cout << "from get page id";
    return 0;
}


Buffer_Page* BufferPool::requestPage(int pageId, PageType type, bool isWrite) {
    if (pageTable.find(pageId) != pageTable.end()) {
        
        Buffer_Page* page = pageTable[pageId];
        page->referenceBit = true;
        page->pinCount++;
        return page;
    } else {
        
        return replacePage(pageId, type, isWrite);
    }
}


void BufferPool::releasePage(int pageId) {
    if (pageTable.find(pageId) != pageTable.end()) {
        Buffer_Page* page = pageTable[pageId];
        page->pinCount = std::max(0, page->pinCount - 1);  
    }
}


Buffer_Page* BufferPool::replacePage(int pageId, PageType type, bool isWrite) {
    while (true) {
        Buffer_Page* page = bufferPool[clockHand];

        if (page == nullptr || (page->pinCount == 0 && !page->referenceBit)) {
            
            if (page != nullptr) {
                if (page->dirtyBit) {
                    page->writeToDisk();
                }
                pageTable.erase(page->pageId);
                delete page;
            }

            
            Buffer_Page* newPage = Buffer_Page::readFromDisk(pageId, type);
            newPage->pinCount++;
            if (isWrite) {
                newPage->dirtyBit = true;
            }

            
            bufferPool[clockHand] = newPage;
            pageTable[pageId] = newPage;

            
            clockHand = (clockHand + 1) % bufferSize;
            return newPage;
        } else if (page->pinCount == 0 && page->referenceBit) {
            
            page->referenceBit = false;
            clockHand = (clockHand + 1) % bufferSize;
        } else {
            
            clockHand = (clockHand + 1) % bufferSize;
        }
    }
}


void BufferPool::displayBufferPool() const {
    std::cout << "Buffer Pool State:\n";
    for (size_t i = 0; i < bufferPool.size(); i++) {
        const Buffer_Page* page = bufferPool[i];
        if (page) {
            std::cout << "Page ID: " << page->pageId 
                      << ", Type: " << (page->type == INDEX_PAGE ? "INDEX" : "DATA")
                      << ", Pin Count: " << page->pinCount 
                      << ", Dirty: " << page->dirtyBit 
                      << ", Reference: " << page->referenceBit 
                      << "\n";
        } else {
            std::cout << "Empty Slot\n";
        }
    }
}