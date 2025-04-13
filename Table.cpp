#include "Table.hpp"
#include "page.hpp"
#include "fstream"
#include <iostream>
Page* Table::Create_page() {
    page_count++;
    // Create a new page and add it to the directory
    // Implementation depends on how pages are stored
    // For now, increment page_count
    Page* page = new Page(4096-3*sizeof(int),{(page_count-1)*81+1,(page_count) * 81});
    page->serialize(page_count,"test",table_name);
    page->pageId = page_count;
    
}

Page* Table::Get_page(int page_id) {
    // Retrieve the page with the given page_id
    // Implementation depends on storage mechanism
    
   Page* pg = new Page(4096,{0,0});
  Page::deserialize(pg,page_id,"test",table_name);
    return pg;
}

void Table::Update_page(int page_id, Page* page) {
    // Update the page with the given page_id
    // Implementation depends on storage mechanism
}

void Table::Delete_page(int page_id) {
    // Delete the page with the given page_id
    // Implementation depends on storage mechanism
}

// bool Table::serializeDirectory(const std::string& dbName, const std::string& fileName) {
//     try {
//         // Construct the full file path
//         std::string filePath = dbName + "/" + fileName + ".HAD";

//         // Open the file in binary write mode
//         std::ofstream outFile(filePath, std::ios::binary | std::ios::out);
//         if (!outFile.is_open()) {
//             throw std::ios_base::failure("Error: Could not open file for writing: " + filePath);
//         }

//         // Skip the first 4096 bytes
//         outFile.seekp(PAGE_SIZE);

//         // Serialize each page starting after the first page
//         for (int i = 0; i < page_count; ++i) {
//             Page* page = Get_page(i);
//             if (page && page->serialize()) {
//                 // Write the page data to the file
//                 outFile.write(*(page->PageData), PAGE_SIZE);
//             } else {
//                 // Handle serialization error
//                 outFile.close();
//                 return false;
//             }
//         }

//         // Close the file
//         outFile.close();
//         std::cout << "Directory serialized successfully to " << filePath << std::endl;
//         return true;
//     } catch (const std::ios_base::failure& e) {
//         std::cerr << "Error serializing directory: " << e.what() << std::endl;
//         return false;
//     }
// }