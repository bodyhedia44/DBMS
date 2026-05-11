#include "Table.hpp"
#include "page.hpp"
#include <fstream>
#include <iostream>

Table::Table(const std::string& table_name, const std::string& db_name)
    : table_name(table_name), db_name(db_name), size(0) {}

Page* Table::Create_page() {
    page_count++;
    int header_size = 3 * sizeof(int);
    Page* page = new Page(PAGE_SIZE - header_size,
                          {static_cast<int>((page_count - 1) * 81 + 1),
                           static_cast<int>(page_count * 81)});
    page->pageId = page_count;
    page->serialize(page_count, db_name, table_name);
    return page;
}

Page* Table::Get_page(int page_id) {
    Page* pg = new Page(PAGE_SIZE, {0, 0});
    Page::deserialize(pg, page_id, db_name, table_name);
    return pg;
}

void Table::Update_page(int page_id, Page* page) {
    if (page) {
        page->serialize(page_id, db_name, table_name);
    }
}

void Table::Delete_page(int ) {

}

int Table::maxTuplesPerPage() const {
    int header_size = 3 * sizeof(int);
    return (PAGE_SIZE - header_size) / 50;
}

bool Table::isPageFull(Page* page) const {
    return page && page->isFull();
}
