#ifndef EXECUTION_ENGINE_HPP
#define EXECUTION_ENGINE_HPP

#include <string>
#include <vector>
#include <map>
#include "DataBase.hpp"
#include "Buffer.hpp"

class ExecutionEngine {
public:
    DataBase& Db;
    BufferPool& bufferPool;

    ExecutionEngine(DataBase& db, BufferPool& pool);

    bool Create_table(const std::string& tableName, const std::map<std::string, std::string>& schema);
    bool insert(const std::string& tableName,
                const std::vector<std::pair<std::string, std::pair<int, std::string>>>& attributes);
    bool update(const std::string& tableName,
                const std::pair<std::string, std::string>& condition,
                const std::vector<std::pair<std::string, std::string>>& updates);
    bool deleteRecord(const std::string& tableName, const std::pair<std::string, std::string>& attribute);
    std::vector<Tuple> select(const std::string& tableName, const std::pair<std::string, std::string>& attribute);
};

#endif
