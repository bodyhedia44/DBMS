#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include "page.hpp"
#include "Table.hpp"

namespace fs = std::filesystem;

class DataBase {

private:
    
    

public:
    std::string dbname;
    
    DataBase(const std::string& name);
    bool createDatabase();
    bool tableExists(const std::string& tableName);
    bool createTable(const std::string& tableName, const std::map<std::string, std::string>& schema);
    bool deleteTable(const std::string& tableName);
    Table* deserializeSchema(const std::string& dbName, const std::string& fileName);
    bool serializeSchema(const std::map<std::string, std::string>& schema, const std::string& dbName, const std::string& fileName,uint32_t& size, uint32_t page_count) ;
    
    Table* getTable(const std::string tableName);
};

#endif 
