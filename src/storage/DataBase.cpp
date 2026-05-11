#include "DataBase.hpp"
#include <iostream>
#include <arpa/inet.h>

DataBase::DataBase(const std::string& name) : dbname(name) {}

bool DataBase::createDatabase() {
    try {
        if (fs::exists(dbname)) {
            return true;
        }
        if (fs::create_directory(dbname)) {
            return true;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    std::cerr << "Failed to create database directory: " << dbname << std::endl;
    return false;
}

bool DataBase::tableExists(const std::string& tableName) {
    fs::path tablePath = fs::path(dbname) / (tableName + ".HAD");
    return fs::exists(tablePath);
}

bool DataBase::createTable(const std::string& tableName, const std::map<std::string, std::string>& schema) {
    if (tableExists(tableName)) {
        std::cerr << "Table already exists: " << tableName << std::endl;
        return false;
    }
    uint32_t size;
    serializeSchema(schema, dbname, tableName, size, 0);
    return true;
}

bool DataBase::deleteTable(const std::string& tableName) {
    fs::path tablePath = fs::path(dbname) / (tableName + ".HAD");

    if (!fs::exists(tablePath)) {
        std::cerr << "Table does not exist: " << tableName << std::endl;
        return false;
    }

    try {
        if (fs::remove(tablePath)) {
            std::cout << "Table deleted: " << tableName << std::endl;
            return true;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error deleting table: " << e.what() << std::endl;
    }
    return false;
}

Table* DataBase::getTable(const std::string& tableName) {
    if (tableExists(tableName)) {
        return deserializeSchema(dbname, tableName);
    }
    std::cerr << "Table not found: " << tableName << std::endl;
    return nullptr;
}

bool DataBase::serializeSchema(const std::map<std::string, std::string>& schema,
                               const std::string& dbName, const std::string& fileName,
                               uint32_t& size, uint32_t page_count) {
    try {
        size = schema.size();
        std::string filePath = dbName + "/" + fileName + ".HAD";
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) {
            throw std::ios_base::failure("Error: Could not open file for writing: " + filePath);
        }

        uint32_t size_network = htonl(size);
        uint32_t page_count_network = htonl(page_count);

        outFile.write(reinterpret_cast<const char*>(&size_network), sizeof(size_network));
        outFile.write(reinterpret_cast<const char*>(&page_count_network), sizeof(page_count_network));

        for (const auto& pair : schema) {
            uint32_t keySize = pair.first.size();
            uint32_t valueSize = pair.second.size();
            uint32_t keySize_network = htonl(keySize);
            uint32_t valueSize_network = htonl(valueSize);

            outFile.write(reinterpret_cast<const char*>(&keySize_network), sizeof(keySize_network));
            outFile.write(reinterpret_cast<const char*>(&valueSize_network), sizeof(valueSize_network));
            outFile.write(pair.first.c_str(), keySize);
            outFile.write(pair.second.c_str(), valueSize);
        }

        outFile.close();
        return true;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "Error serializing schema: " << e.what() << std::endl;
        return false;
    }
}

Table* DataBase::deserializeSchema(const std::string& dbName, const std::string& fileName) {
    std::string filePath = dbName + "/" + fileName + ".HAD";
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::ios_base::failure("Error: Could not open file for reading: " + filePath);
    }

    uint32_t size_network;
    inFile.read(reinterpret_cast<char*>(&size_network), sizeof(size_network));
    uint32_t size = ntohl(size_network);

    uint32_t page_count_network;
    inFile.read(reinterpret_cast<char*>(&page_count_network), sizeof(page_count_network));
    uint32_t page_count = ntohl(page_count_network);

    if (inFile.fail()) {
        throw std::ios_base::failure("Error: Failed to read schema header.");
    }

    std::map<std::string, std::string> schema;
    for (uint32_t i = 0; i < size; ++i) {
        uint32_t keySize_network, valueSize_network;
        inFile.read(reinterpret_cast<char*>(&keySize_network), sizeof(keySize_network));
        inFile.read(reinterpret_cast<char*>(&valueSize_network), sizeof(valueSize_network));
        uint32_t keySize = ntohl(keySize_network);
        uint32_t valueSize = ntohl(valueSize_network);

        if (inFile.fail() || inFile.eof()) {
            throw std::ios_base::failure("Error: Premature end of file.");
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

    Table* table = new Table(fileName, dbName);
    table->page_count = page_count;
    table->schema = schema;
    table->size = size;
    inFile.close();
    return table;
}

bool DataBase::updatePageCount(const std::string& tableName, uint32_t page_count) {

    std::string filePath = dbname + "/" + tableName + ".HAD";
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file to update page count: " << filePath << std::endl;
        return false;
    }

    file.seekp(sizeof(uint32_t));
    uint32_t page_count_network = htonl(page_count);
    file.write(reinterpret_cast<const char*>(&page_count_network), sizeof(page_count_network));
    file.close();
    return !file.fail();
}
