#ifndef PAGE_HPP
#define PAGE_HPP

#include <vector>
#include <string>
#include <map>
#include <utility>
#include "tuple.hpp"

#define PAGE_SIZE 4096

class Page {
public:
    int pageId;
    int freespace;
    std::vector<Tuple> tuples;
    std::pair<int, int> ids_Range;

    Page(int free, std::pair<int, int> range);

    bool isFull() const;
    bool insert_tuple(const std::vector<std::pair<std::string, std::pair<int, std::string>>>& attributes);
    std::vector<Tuple> get_tuple(const std::pair<std::string, std::string>& attribute) const;
    bool update_tuple(const std::pair<std::string, std::string>& condition,
                      const std::vector<std::pair<std::string, std::string>>& updates);
    bool del_tuple(const std::pair<std::string, std::string>& attribute);
    bool serialize(int page_id, const std::string& dbName, const std::string& tableName);
    static Page* deserialize(Page* page, int page_id, const std::string& dbName, const std::string& tableName);

private:
    static const size_t TUPLE_SIZE = 50;
};

#endif
