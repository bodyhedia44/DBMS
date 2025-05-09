#ifndef EXECUTION_ENGINE_HPP
#define EXECUTION_ENGINE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "DataBase.hpp"
class ExecutionEngine {
public:
    
    DataBase Db;
    ExecutionEngine(DataBase Db);

    bool Create_table(const std::string& tableName, const std::map<std::string,std::string> schema);

bool insert(const std::string& tableName,const std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes) ;
bool update(const std::string& tableName, const std::string& conditionKey, const std::string& conditionValue,
                const std::string& updateKey, const std::string& updateValue);
    bool deleteRecord(std::string& tableName,const std::pair<std::string, std::string>& attribute);
    std::vector<Tuple> select(std::string& tableName,const std::pair<std::string, std::string>& attribute);

private:
    
    bool tableExists(const std::string& tableName) const;
    bool databaseExists(const std::string& dbName) const;

    
    std::string currentDatabase;
    std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> tables;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> tableSchemas;
};

#endif 