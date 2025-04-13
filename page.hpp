#ifndef PAGE_HPP
#define PAGE_HPP

#include <vector>
#include <string>
#include <map>
#include <utility>
#define PAGE_SIZE 4096
#include "tuple.hpp"


class Page {
public:
    
    int pageId;
    int freespace;  
     std::vector<Tuple> tuples; 
    std::pair<int,int> ids_Range;
    char* PageData[PAGE_SIZE];
    
    bool insert_tuple(const std::vector<std::pair<std::string, std::pair<int, std::string>>>& attributes);
    std::vector<Tuple> get_tuple(const std::pair<std::string, std::string>& attribute);
    bool update_tuple( std::pair<std::string, std::string>& attribute);
    bool del_tuple(const std::pair<std::string, std::string>& attribute);
    bool serialize(int page_id, const std::string& dbName, const std::string& tableName);
    static Page* deserialize(Page* page,int page_id, const std::string& dbName, const std::string& tableName);
    Page(int free,std::pair<int,int>);
private:
    static const size_t TUPLE_SIZE = 50; 
    
};

#endif 
