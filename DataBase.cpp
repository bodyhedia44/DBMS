// File: database.cpp
#include "DataBase.hpp"
#include <iostream>
#include <arpa/inet.h> 
DataBase::DataBase(const std::string& name) : dbname(name) {}

bool DataBase::createDatabase() {
    if (fs::create_directory(dbname)) {

        return true;
    }
    //std::cerr << "Failed to create database: " << dbname << std::endl;
    return false;
}

bool DataBase::tableExists(const std::string& tableName) {
    // Define the directory where tables are stored

    // Construct the full path to the table
    std::filesystem::path tablePath = std::filesystem::path(dbname) / (tableName+".HAD");

    // Check if the table exists as a file or directory
    return std::filesystem::exists(tablePath);
}

bool DataBase::createTable(const std::string& tableName, const std::map<std::string, std::string>& schema) {
    if (tableExists(tableName)) {
        std::cerr << "Table already exists: " << tableName << std::endl;
        return false;
    }
    uint32_t size;

    serializeSchema(schema,dbname,tableName,size,0);
    Table* table= new Table(tableName);
    table->schema = schema;
    table->size = size;
    //std::cout << "Table created: " << tableName << std::endl;
    return true;
}

bool DataBase::deleteTable(const std::string& tableName) {
    // Define the directory where tables are stored
    const std::string dbDirectory = "dbname";

    // Construct the full path to the table directory
    std::filesystem::path tablePath = std::filesystem::path(dbDirectory) / tableName;

    // Check if the directory exists
    if (!std::filesystem::exists(tablePath)) {
        std::cerr << "Table does not exist: " << tableName << std::endl;
        return false;
    }

    // Try to delete the directory (only if it's empty)
    try {
        if (std::filesystem::is_directory(tablePath) && std::filesystem::remove(tablePath)) {
            std::cout << "Table directory deleted: " << tableName << std::endl;
            return true;
        } else {
            std::cerr << "Failed to delete table directory. Ensure it is empty: " << tableName << std::endl;
            return false;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error deleting table directory: " << e.what() << std::endl;
        return false;
    }
}

Table* DataBase::getTable(const std::string tableName ) {
    if (tableExists(tableName)) {
        
        
        
        Table* my = deserializeSchema(dbname,tableName);
        //std::cout <<"offset size "<< my->size<<std::endl;
          for (const auto& [key, value] : my->schema) {
        //std::cout << key << ": " << value << std::endl;
    }
        return my;
    }
    std::cerr << "Table not found: " << tableName << std::endl;
    return nullptr;
}


bool DataBase::serializeSchema(const std::map<std::string, std::string>& schema, const std::string& dbName, const std::string& fileName, uint32_t& size, uint32_t page_count) {
    try {
        size = schema.size(); // Ensure size is set to the schema size
        std::string filePath = dbName + "/" + fileName + ".HAD";
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) {
            throw std::ios_base::failure("Error: Could not open file for writing: " + filePath);
        }

        uint32_t size_network = htonl(size);
        uint32_t page_count_network = htonl(page_count);
        

        outFile.write(reinterpret_cast<const char*>(&size_network), sizeof(size_network));
        if (outFile.fail()) {
            throw std::ios_base::failure("Error: Failed to write schema size.");
        }

        outFile.write(reinterpret_cast<const char*>(&page_count_network), sizeof(page_count_network));
        if (outFile.fail()) {
            throw std::ios_base::failure("Error: Failed to write page count.");
        }

        for (const auto& pair : schema) {
            uint32_t keySize = pair.first.size();
            uint32_t valueSize = pair.second.size();

            uint32_t keySize_network = htonl(keySize);
            uint32_t valueSize_network = htonl(valueSize);

            outFile.write(reinterpret_cast<const char*>(&keySize_network), sizeof(keySize_network));
            outFile.write(reinterpret_cast<const char*>(&valueSize_network), sizeof(valueSize_network));
            if (outFile.fail()) {
                throw std::ios_base::failure("Error: Failed to write key/value sizes.");
            }

            outFile.write(pair.first.c_str(), keySize);
            outFile.write(pair.second.c_str(), valueSize);
            if (outFile.fail()) {
                throw std::ios_base::failure("Error: Failed to write key or value.");
            }
        }

        outFile.close();
        //std::cout << "Schema serialized successfully to " << filePath << std::endl;
        return true;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "Error serializing schema: " << e.what() << std::endl;
        return false;
    }
}
Table* DataBase::deserializeSchema(const std::string& dbName, const std::string& fileName) {
    std::map<std::string, std::string> schema;
    std::string filePath = dbName + "/" + fileName + ".HAD";

    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::ios_base::failure("Error: Could not open file for reading: " + filePath);
    }

    uint32_t size_network;
    inFile.read(reinterpret_cast<char*>(&size_network), sizeof(size_network));
    uint32_t size = ntohl(size_network);
    if (inFile.fail()) {
        throw std::ios_base::failure("Error: Failed to read schema size.");
    }

    uint32_t page_count_network;
    inFile.read(reinterpret_cast<char*>(&page_count_network), sizeof(page_count_network));
    uint32_t page_count = ntohl(page_count_network);
    if (inFile.fail()) {
        throw std::ios_base::failure("Error: Failed to read page count.");
    }

    // std::cout << "Size: " << size << std::endl;
    // std::cout << "Page count: " << page_count << std::endl;

    for (uint32_t i = 0; i < size; ++i) {
        uint32_t keySize_network, valueSize_network;
        inFile.read(reinterpret_cast<char*>(&keySize_network), sizeof(keySize_network));
        inFile.read(reinterpret_cast<char*>(&valueSize_network), sizeof(valueSize_network));
        uint32_t keySize = ntohl(keySize_network);
        uint32_t valueSize = ntohl(valueSize_network);
        if (inFile.fail()) {
            throw std::ios_base::failure("Error: Failed to read key/value sizes.");
        }

        //std::cout << "keySize: " << keySize << ", valueSize: " << valueSize << std::endl;

        if (inFile.eof()) {
            throw std::ios_base::failure("Error: Premature end of file while reading key/value sizes.");
        }

        std::string key(keySize, '\0');
        inFile.read(&key[0], keySize);
        std::string value(valueSize, '\0');
        inFile.read(&value[0], valueSize);
        if (inFile.fail()) {
            throw std::ios_base::failure("Error: Failed to read key or value.");
        }

        schema[key] = value;
    }

    Table* table = new Table(fileName);
    table->page_count = page_count;
    table->schema = schema;
    table->size = size;
    inFile.close();
   // std::cout << "Schema deserialized successfully from " << filePath << std::endl;
    return table;
}
// Deserialize the schema from a .HAD file using FileHandler
// Table* DataBase::deserializeSchema(const std::string& dbName, const std::string& fileName) {
//     std::map<std::string, std::string> schema;
//     std::string filePath = dbName + "/" + fileName + ".HAD";

//     std::ifstream inFile(filePath, std::ios::binary);
//     if (!inFile.is_open()) {
//         throw std::ios_base::failure("Error: Could not open file for reading: " + filePath);
//     }

//     uint32_t size_network;
//     inFile.read(reinterpret_cast<char*>(&size_network), sizeof(size_network));
//     uint32_t size = ntohl(size_network);
//     std::cout << size << std::endl;
//     if (inFile.fail()) {
//         throw std::ios_base::failure("Error: Failed to read schema size.");
//     }

//     uint32_t page_count_network;
//     inFile.read(reinterpret_cast<char*>(&page_count_network), sizeof(page_count_network));
//     uint32_t page_count = ntohl(page_count_network);
//     std::cout << page_count << std::endl;
//     if (inFile.fail()) {
//         throw std::ios_base::failure("Error: Failed to read page count.");
//     }

//     for (uint32_t i = 0; i < size; ++i) {
//         uint32_t keySize_network, valueSize_network;
//         inFile.read(reinterpret_cast<char*>(&keySize_network), sizeof(keySize_network));
//         inFile.read(reinterpret_cast<char*>(&valueSize_network), sizeof(valueSize_network));
//         uint32_t keySize = ntohl(keySize_network);
//         std::cout << keySize  << std::endl;
//         uint32_t valueSize = ntohl(valueSize_network);
//         std::cout << valueSize  << std::endl;
//         if (inFile.fail()) {
//             throw std::ios_base::failure("Error: Failed to read key/value sizes.");
//         }

//         std::string key(keySize, '\0');
//         inFile.read(&key[0], keySize);
//         std::string value(valueSize, '\0');
//         inFile.read(&value[0], valueSize);
//         if (inFile.fail()) {
//             throw std::ios_base::failure("Error: Failed to read key or value.");
//         }

//         schema[key] = value;
//     }

//     Table* table = new Table(fileName);
//     table->page_count = page_count;
//     table->schema = schema;
//     table->size = size;
//     inFile.close();
//     std::cout << "Schema deserialized successfully from " << filePath << std::endl;
//     return table;
// }