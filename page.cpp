#include "page.hpp"
#include "tuple.hpp"
#include <fstream>
#include <iostream>
#include <utility>
#include <cstring> 
#include <arpa/inet.h>
#include <memory>
Page::Page(int free,std::pair<int,int>)
        : freespace(free),
          ids_Range({0, 0})
          {
        
        std::memset(PageData, 0, PAGE_SIZE);
    }
    #pragma pack(push, 1)
bool Page::insert_tuple(const std::vector<std::pair<std::string, std::pair<int, std::string>>>& attributes){
    Tuple* t = new Tuple();
    
    if (freespace >= 50){
        freespace -= 50;
        for (const auto& attr : attributes) {
        t->add_attribute(attr.first, attr.second.second);
    }
    t->add_attribute("id",std::to_string(tuples.size()+ ids_Range.first));
    }
    tuples.push_back(*t);
}

bool Page::del_tuple(const std::pair<std::string, std::string>& attribute){
        if(!(attribute.first == " " && attribute.second == " ")){

            tuples.clear();
            std::cout<<"All - >"<<tuples.size()<<std::endl;
        }
        else{
            std::vector<Tuple> res= get_tuple(attribute);
            for(int i = 0;i<res.size();i++){
                for(int j = 0;j<tuples.size();j++){
                    if(tuples[j].get_attribute(attribute.first) == res[i].get_attribute(attribute.first)){
                        tuples.erase(tuples.begin()+i);
                    }
                }
            }
        std::cout<<" ->"<<tuples.size()<<std::endl;
        }
}


bool Page::serialize(int page_id, const std::string& dbName, const std::string& tableName) {
    std::string filePath = dbName + "/" + tableName + ".HAD";
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filePath << std::endl;
        return false;
    }

    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    file.seekp(offset);

    
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    
    file.seekp(file_size);

    
    if (offset + PAGE_SIZE > file_size) {
        size_t bytes_to_write =PAGE_SIZE- file_size;
        char padding[bytes_to_write];
        memset(padding, 0, bytes_to_write);
        file.write(padding, bytes_to_write);
    }
    file.seekp(offset);

    
    int fs_network = htonl(freespace);
    
    file.write(reinterpret_cast<const char*>(&fs_network), sizeof(int));
    if (file.fail()) {
        std::cerr << "Error: Could not write freespace to file: " << filePath << std::endl;
        file.close();
        return false;
    }

    int id_first_network = htonl(ids_Range.first);
    file.write(reinterpret_cast<const char*>(&id_first_network), sizeof(int));
    if (file.fail()) {
        std::cerr << "Error: Could not write ids_Range.first to file: " << filePath << std::endl;
        file.close();
        return false;
    }

    int id_second_network = htonl(ids_Range.second);
    file.write(reinterpret_cast<const char*>(&id_second_network), sizeof(int));
    if (file.fail()) {
        std::cerr << "Error: Could not write ids_Range.second to file: " << filePath << std::endl;
        file.close();
        return false;
    }

    const int dsize = PAGE_SIZE - 3 * sizeof(int);
    std::string pageData(dsize, '\0'); 
    size_t dataOffset = 0;

    for ( auto& tuple : tuples) {
        std::string tupleData = tuple.Serialize();
        if (dataOffset + TUPLE_SIZE > dsize) {
            std::cerr << "Error: Page size exceeded." << std::endl;
            return false;
        }
        std::memcpy(&pageData[dataOffset], tupleData.data(), TUPLE_SIZE);
        dataOffset += TUPLE_SIZE;
    }
    
    
    file.write(pageData.data(), dsize - tuples.size() * 50);

    
    size_t data_written = 3 * sizeof(int) + dataOffset;
    if (data_written < PAGE_SIZE) {
        char padding[PAGE_SIZE - data_written];
        memset(padding, 0, PAGE_SIZE - data_written);
        file.write(padding, PAGE_SIZE - data_written);
    }

    file.close();
    return true;
}  

Page* Page::deserialize(Page* page,int page_id, const std::string& dbName, const std::string& tableName) {
        std::string filePath = dbName + "/" + tableName + ".HAD";
        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for reading: " << filePath << std::endl;
            return nullptr;
        }

        size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
        file.seekg(offset);
        if (file.fail()) {
            std::cerr << "Error: Could not seek to offset " << offset << " in file: " << filePath << std::endl;
            file.close();
            return nullptr;
        }

        size_t file_size = 0;
        file.seekg(0, std::ios::end);
        file_size = file.tellg();
        
        file.seekg(offset);

        if (file_size < offset + PAGE_SIZE) {
            std::cerr << "File is smaller than expected for page " << page_id << std::endl;
            file.close();
            return nullptr;
        }

        int freespace_network;
        if (!file.read(reinterpret_cast<char*>(&freespace_network), sizeof(int))) {
            std::cerr << "Error: Could not read freespace from file: " << filePath << std::endl;
            file.close();
            return nullptr;
        }
        int freespace = ntohl(freespace_network);

        int id_first_network;
        if (!file.read(reinterpret_cast<char*>(&id_first_network), sizeof(int))) {
            std::cerr << "Error: Could not read ids_Range.first from file: " << filePath << std::endl;
            file.close();
            return nullptr;
        }
        int id_first = ntohl(id_first_network);

        int id_second_network;
        if (!file.read(reinterpret_cast<char*>(&id_second_network), sizeof(int))) {
            std::cerr << "Error: Could not read ids_Range.second from file: " << filePath << std::endl;
            file.close();
            return nullptr;
        }
        int id_second = ntohl(id_second_network);

        const size_t used_space = PAGE_SIZE - freespace;
        const size_t tuples_size = used_space - 3 * sizeof(int);    
        

       const int dsize = PAGE_SIZE - 3 * sizeof(int);
    std::string pageData(dsize, '\0');
    file.read(&pageData[0], dsize);

    
    page->tuples.clear();
    
    size_t dataOffset = 0;
    while (dataOffset + TUPLE_SIZE <= tuples_size) {
        std::string tupleData(&pageData[dataOffset], TUPLE_SIZE);
        Tuple tuple;
        tuple.Deserialize(tupleData);
        page->tuples.push_back(tuple);
        dataOffset += TUPLE_SIZE;
    }
        page->freespace = freespace;
        page->pageId = page_id;
        page->ids_Range={id_first,id_second};
        file.close();
        return page;
    }

std::vector<Tuple> Page::get_tuple(const std::pair<std::string, std::string>& attribute){
        std::vector<Tuple> results;
        if(!(attribute.first == " " && attribute.second == " ")){
            for(auto tuple:tuples){
                if(tuple.get_attribute(attribute.first) == attribute.second){
                    results.push_back(tuple);
                }
            }
            
            return results;
        }
        else{
           
            return tuples;
        }
    }