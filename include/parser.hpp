#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>
#include <sstream>
#include <iostream>

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
    SyntaxValidator syntaxValidator;
    ExecutionPlanGenerator planGenerator;
    QueryInfo queryInfo;
};

#endif
