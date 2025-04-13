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
    
    // std::map<std::string, Table*> tables;   // Store tables within the database

public:
    std::string dbname;
    // Constructor
    DataBase(const std::string& name);

    // Method to create a new database
    bool createDatabase();

    // Check if a table exists
    bool tableExists(const std::string& tableName);

    // Create a new table
    bool createTable(const std::string& tableName, const std::map<std::string, std::string>& schema);

    // Delete a table
    bool deleteTable(const std::string& tableName);
    Table* deserializeSchema(const std::string& dbName, const std::string& fileName);
    bool serializeSchema(const std::map<std::string, std::string>& schema, const std::string& dbName, const std::string& fileName,uint32_t& size, uint32_t page_count) ;
    // Get a table by name
    Table* getTable(const std::string tableName);
};

#endif // DATABASE_HPP
