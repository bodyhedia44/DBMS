#ifndef TABLE_HPP
#define TABLE_HPP

#include <string>
#include <map>
#include <vector>
#include <utility>

// Forward declaration of the Page class
class Page;

class Table {
public:
    // Attributes
    std::map<std::string, std::string> schema ;  // Reference to schema
    //std::vector<std::map<int, std::pair<int, int>>> directory;  // Vector of pages
    std::string table_name;
    uint32_t size;
    uint32_t page_count = 0;

    // Constructor
    Table(const std::string table_name) : table_name(table_name){};

    // Methods
    Page* Create_page(); // Method to create a new page
    Page* Get_page(int page_id);  // Get a page by page_id
    void Update_page(int page_id, Page* page);  // Update a page
    void Delete_page(int page_id);  // Delete a page
    bool serializeDirectory(const std::string& dbName, const std::string& fileName);

private:
    // Additional private members can be added if needed
};

#endif // TABLE_HPP
