#ifndef TABLE_HPP
#define TABLE_HPP

#include <string>
#include <map>
#include <vector>
#include <utility>

class Page;

class Table {
public:
    std::map<std::string, std::string> schema;
    std::string table_name;
    std::string db_name;
    uint32_t size;
    uint32_t page_count = 0;

    Table(const std::string& table_name, const std::string& db_name = "");

    Page* Create_page();
    Page* Get_page(int page_id);
    void Update_page(int page_id, Page* page);
    void Delete_page(int page_id);

    int maxTuplesPerPage() const;
    bool isPageFull(Page* page) const;
};

#endif
