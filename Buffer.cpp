#include "Buffer.hpp"

// Buffer_Page constructor
Buffer_Page::Buffer_Page(int id, PageType t) : pageId(id), type(t), referenceBit(true), dirtyBit(false), pinCount(0) {
    std::memset(data, 0, PAGE_SIZE);  // Initialize data
}

// Function to simulate writing page to disk (if dirty)
void Buffer_Page::writeToDisk() {
    if (dirtyBit) {
        std::ofstream file("page_" + std::to_string(pageId) + ".dat", std::ios::binary);
        file.write(data, PAGE_SIZE);
        file.close();
        dirtyBit = false;  // Reset dirty bit after writing
    }
}

// Function to simulate reading page from disk
Buffer_Page* Buffer_Page::readFromDisk(int pageId, PageType type) {
    Buffer_Page* page = new Buffer_Page(pageId, type);
    std::ifstream file("page_" + std::to_string(pageId) + ".dat", std::ios::binary);
    if (file.is_open()) {
        file.read(page->data, PAGE_SIZE);
        file.close();
    }
    return page;
}

// BufferPool constructor
BufferPool::BufferPool(int size) : bufferSize(size), clockHand(0) {
    bufferPool.resize(bufferSize, nullptr);
}

// BufferPool destructor
BufferPool::~BufferPool() {
    // Clean up and write dirty pages to disk
    for (Buffer_Page* page : bufferPool) {
        if (page) {
            page->writeToDisk();
            delete page;
        }
    }
}

// Function to get a new page ID
int BufferPool::getNewPageId() {
    std::cout << "from get page id";
    return 0;
}

// Function to request a page
Buffer_Page* BufferPool::requestPage(int pageId, PageType type, bool isWrite) {
    if (pageTable.find(pageId) != pageTable.end()) {
        // Page is already in memory
        Buffer_Page* page = pageTable[pageId];
        page->referenceBit = true;
        page->pinCount++;
        return page;
    } else {
        // Page not in memory, load it from disk
        return replacePage(pageId, type, isWrite);
    }
}

// Function to release a page (decrement pin count)
void BufferPool::releasePage(int pageId) {
    if (pageTable.find(pageId) != pageTable.end()) {
        Buffer_Page* page = pageTable[pageId];
        page->pinCount = std::max(0, page->pinCount - 1);  // Decrement pin count
    }
}

// Function to replace a page using the Clock Algorithm
Buffer_Page* BufferPool::replacePage(int pageId, PageType type, bool isWrite) {
    while (true) {
        Buffer_Page* page = bufferPool[clockHand];

        if (page == nullptr || (page->pinCount == 0 && !page->referenceBit)) {
            // If empty slot or unpinned page with reference bit off, replace it
            if (page != nullptr) {
                if (page->dirtyBit) {
                    page->writeToDisk();
                }
                pageTable.erase(page->pageId);
                delete page;
            }

            // Load the new page from disk
            Buffer_Page* newPage = Buffer_Page::readFromDisk(pageId, type);
            newPage->pinCount++;
            if (isWrite) {
                newPage->dirtyBit = true;
            }

            // Insert the new page in the buffer
            bufferPool[clockHand] = newPage;
            pageTable[pageId] = newPage;

            // Move clock hand forward
            clockHand = (clockHand + 1) % bufferSize;
            return newPage;
        } else if (page->pinCount == 0 && page->referenceBit) {
            // Reset the reference bit and move clock hand
            page->referenceBit = false;
            clockHand = (clockHand + 1) % bufferSize;
        } else {
            // Move clock hand forward
            clockHand = (clockHand + 1) % bufferSize;
        }
    }
}

// Function to display the buffer pool state (for debugging)
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