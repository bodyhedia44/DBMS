#include "Table.hpp"
#include "page.hpp"
#include "fstream"
#include <iostream>
Page* Table::Create_page() {
    page_count++;
    Page* page = new Page(4096-3*sizeof(int),{(page_count-1)*81+1,(page_count) * 81});
    page->serialize(page_count,"test",table_name);
    page->pageId = page_count;
    
}

Page* Table::Get_page(int page_id) {
   Page* pg = new Page(4096,{0,0});
  Page::deserialize(pg,page_id,"test",table_name);
    return pg;
}

void Table::Update_page(int page_id, Page* page) {
    
    
}

void Table::Delete_page(int page_id) {
    
    
}

