#include "page.hpp"
#include "tuple.hpp"
#include <fstream>
#include <iostream>
#include <utility>
#include <cstring>
#include <arpa/inet.h>
#include <vector>

Page::Page(int free, std::pair<int, int> range)
    : pageId(0), freespace(free), ids_Range(range) {}

bool Page::isFull() const {
    return freespace < static_cast<int>(TUPLE_SIZE);
}

bool Page::insert_tuple(const std::vector<std::pair<std::string, std::pair<int, std::string>>>& attributes) {
    if (freespace < static_cast<int>(TUPLE_SIZE)) {
        std::cerr << "Error: Not enough free space on page to insert tuple.\n";
        return false;
    }

    Tuple t;
    for (const auto& attr : attributes) {
        t.add_attribute(attr.first, attr.second.second);
    }
    t.add_attribute("id", std::to_string(tuples.size() + ids_Range.first));

    freespace -= TUPLE_SIZE;
    tuples.push_back(t);
    return true;
}

bool Page::del_tuple(const std::pair<std::string, std::string>& attribute) {
    if (attribute.first == " " && attribute.second == " ") {
        tuples.clear();
        freespace = PAGE_SIZE - 3 * sizeof(int);
        return true;
    }

    size_t before = tuples.size();
    for (auto it = tuples.begin(); it != tuples.end();) {
        if (it->get_attribute(attribute.first) == attribute.second) {
            it = tuples.erase(it);
            freespace += TUPLE_SIZE;
        } else {
            ++it;
        }
    }
    return tuples.size() < before;
}

bool Page::update_tuple(const std::pair<std::string, std::string>& condition,
                        const std::vector<std::pair<std::string, std::string>>& updates) {
    bool updated = false;

    if (condition.first == " " && condition.second == " ") {

        for (auto& tuple : tuples) {
            tuple.update_attribute(updates);
            updated = true;
        }
    } else {
        for (auto& tuple : tuples) {
            if (tuple.get_attribute(condition.first) == condition.second) {
                tuple.update_attribute(updates);
                updated = true;
            }
        }
    }
    return updated;
}

std::vector<Tuple> Page::get_tuple(const std::pair<std::string, std::string>& attribute) const {
    if (attribute.first == " " && attribute.second == " ") {
        return tuples;
    }

    std::vector<Tuple> results;
    for (const auto& tuple : tuples) {
        if (tuple.get_attribute(attribute.first) == attribute.second) {
            results.push_back(tuple);
        }
    }
    return results;
}

bool Page::serialize(int page_id, const std::string& dbName, const std::string& tableName) {
    std::string filePath = dbName + "/" + tableName + ".HAD";
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filePath << std::endl;
        return false;
    }

    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();

    if (offset + PAGE_SIZE > file_size) {
        file.seekp(file_size);
        size_t bytes_to_write = (offset + PAGE_SIZE) - file_size;
        std::vector<char> padding(bytes_to_write, 0);
        file.write(padding.data(), bytes_to_write);
    }

    file.seekp(offset);

    int fs_network = htonl(freespace);
    file.write(reinterpret_cast<const char*>(&fs_network), sizeof(int));

    int id_first_network = htonl(ids_Range.first);
    file.write(reinterpret_cast<const char*>(&id_first_network), sizeof(int));

    int id_second_network = htonl(ids_Range.second);
    file.write(reinterpret_cast<const char*>(&id_second_network), sizeof(int));

    const int dsize = PAGE_SIZE - 3 * sizeof(int);
    std::string pageData(dsize, '\0');
    size_t dataOffset = 0;

    for (auto& tuple : tuples) {
        std::string tupleData = tuple.Serialize();
        if (dataOffset + TUPLE_SIZE > static_cast<size_t>(dsize)) {
            std::cerr << "Error: Page size exceeded." << std::endl;
            file.close();
            return false;
        }
        std::memcpy(&pageData[dataOffset], tupleData.data(), TUPLE_SIZE);
        dataOffset += TUPLE_SIZE;
    }

    file.write(pageData.data(), dsize);
    file.close();
    return true;
}

Page* Page::deserialize(Page* page, int page_id, const std::string& dbName, const std::string& tableName) {
    std::string filePath = dbName + "/" + tableName + ".HAD";
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << filePath << std::endl;
        return nullptr;
    }

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();

    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    if (file_size < offset + PAGE_SIZE) {
        std::cerr << "File is smaller than expected for page " << page_id << std::endl;
        file.close();
        return nullptr;
    }

    file.seekg(offset);

    int freespace_network;
    file.read(reinterpret_cast<char*>(&freespace_network), sizeof(int));
    int freespace = ntohl(freespace_network);

    int id_first_network;
    file.read(reinterpret_cast<char*>(&id_first_network), sizeof(int));
    int id_first = ntohl(id_first_network);

    int id_second_network;
    file.read(reinterpret_cast<char*>(&id_second_network), sizeof(int));
    int id_second = ntohl(id_second_network);

    if (file.fail()) {
        std::cerr << "Error: Could not read page header from file: " << filePath << std::endl;
        file.close();
        return nullptr;
    }

    const size_t used_space = PAGE_SIZE - freespace;
    const size_t tuples_size = used_space - 3 * sizeof(int);

    const int dsize = PAGE_SIZE - 3 * sizeof(int);
    std::string pageData(dsize, '\0');
    file.read(&pageData[0], dsize);

    page->tuples.clear();
    size_t dataOffset = 0;
    while (dataOffset + Page::TUPLE_SIZE <= tuples_size) {
        std::string tupleData(&pageData[dataOffset], Page::TUPLE_SIZE);
        Tuple tuple;
        tuple.Deserialize(tupleData);
        page->tuples.push_back(tuple);
        dataOffset += Page::TUPLE_SIZE;
    }

    page->freespace = freespace;
    page->pageId = page_id;
    page->ids_Range = {id_first, id_second};
    file.close();
    return page;
}
