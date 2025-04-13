#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>
#include <sstream>
#include <iostream>

enum class ColumnType {
    INTEGER,
    VARCHAR,
    BOOLEAN,
    FLOAT,
    CHAR
};

class Column {
public:
    Column(const std::string& name, ColumnType type);
    std::string name;
    ColumnType type;
};

class Schema {
public:
    void addTable(const std::string& tableName, const std::vector<Column>& columns);
    void addIndex(const std::string& tableName, const std::string& indexName);
    bool hasTable(const std::string& tableName) const;
    bool hasColumn(const std::string& tableName, const std::string& columnName) const;

private:
    //std::unordered_map<std::string, std::vector<Column>> tables;
    //std::unordered_map<std::string, std::vector<std::string>> indexes;
};

struct QueryInfo {
    std::string type;
    std::string tableName;
    std::vector<std::string> columns;
    std::string condition;
    std::vector<std::string> values;
    std::string indexName;
};

class SyntaxValidator {
public:
    QueryInfo validateAndExtract(const std::string& query);

private:
    std::vector<std::string> splitAndTrim(const std::string& str, char delimiter = ',');
};

// class SemanticCheck {
// public:
//     SemanticCheck(const Schema& schema);
//     bool validate(const QueryInfo& queryInfo);
//     std::string getLastError() const;

// private:
//     // Schema schema;
//     std::string lastError;
// };

struct ExecutionStep {
    std::string operation;
    std::string target;
    std::string details;
};

class ExecutionPlanGenerator {
public:
    std::vector<ExecutionStep> generatePlan(const QueryInfo& queryInfo);

private:
    std::string join(const std::vector<std::string>& items, const std::string& delimiter = ", ");
};

class QueryAnalyzer {
public:
    QueryAnalyzer();
    std::vector<ExecutionStep> analyze(const std::string& query);
    const QueryInfo& getQueryInfo() const;

private:
    //const Schema& schema;
    SyntaxValidator syntaxValidator;
    // SemanticCheck semanticCheck;
    ExecutionPlanGenerator planGenerator;
    QueryInfo queryInfo;
};

#endif // PARSER_HPP
