#include "parser.hpp"

using namespace std;


Column::Column(const string& name, ColumnType type)
    : name(name), type(type) {}

QueryInfo SyntaxValidator::validateAndExtract(const string& query) {
    QueryInfo info;
    smatch matches;

    static const regex selectPattern(R"(SELECT\s+((\*)|(\w+)|([\w\s,]+))\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", regex_constants::icase);
    if (regex_match(query, matches, selectPattern)) {
        info.type = "SELECT";
        string columnPart = matches[1].str();
        info.tableName = matches[5].str();
        info.condition = matches.size() > 6 && matches[6].matched ? matches[6].str() : "";
        info.columns = (columnPart == "*") ? vector<string>{"*"} : splitAndTrim(columnPart);
        return info;
    }

    static const regex createIndexPattern(R"(CREATE\s+INDEX\s+(\w+)\s+ON\s+(\w+)\s*\((\w+)\))", regex_constants::icase);
    if (regex_match(query, matches, createIndexPattern)) {
        info.type = "CREATE_INDEX";
        info.indexName = matches[1].str();
        info.tableName = matches[2].str();
        info.columns = {matches[3].str()};
        return info;
    }

    static const regex insertPattern(R"(INSERT\s+INTO\s+(\w+)\s*\(([\w\s,]+)\)\s*VALUES\s*\(([\w\s',]+)\))", regex_constants::icase);
    if (regex_match(query, matches, insertPattern)) {
        info.type = "INSERT";
        info.tableName = matches[1].str();
        info.columns = splitAndTrim(matches[2].str());
        info.values = splitAndTrim(matches[3].str());
        return info;
    }

    static const regex deletePattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", regex_constants::icase);
    if (regex_match(query, matches, deletePattern)) {
        info.type = "DELETE";
        info.tableName = matches[1].str();
        info.condition = matches.size() > 2 && matches[2].matched ? matches[2].str() : "";
        return info;
    }

    static const regex updatePattern(R"(UPDATE\s+(\w+)\s+SET\s+([\w\s=,]+)(?:\s+WHERE\s+(.+))?)", regex_constants::icase);
    if (regex_match(query, matches, updatePattern)) {
        info.type = "UPDATE";
        info.tableName = matches[1].str();
        info.columns = splitAndTrim(matches[2].str());
        info.condition = matches.size() > 3 && matches[3].matched ? matches[3].str() : "";
        return info;
    }

    static const regex createTablePattern(R"(CREATE\s+TABLE\s+(\w+)\s*\(([\w\s,]+)\))", regex_constants::icase);
    if (regex_match(query, matches, createTablePattern)) {
        info.type = "CREATE";
        info.tableName = matches[1].str();
        info.columns = splitAndTrim(matches[2].str());
        return info;
    }

    info.type = "UNKNOWN";
    return info;
}

vector<string> SyntaxValidator::splitAndTrim(const string& str, char delimiter) {
    vector<string> result;
    stringstream ss(str);
    string item;
    while (getline(ss, item, delimiter)) {
        item.erase(0, item.find_first_not_of(" "));
        item.erase(item.find_last_not_of(" ") + 1);
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

vector<ExecutionStep> ExecutionPlanGenerator::generatePlan(const QueryInfo& queryInfo) {
     vector<ExecutionStep> plan;

        if (queryInfo.type == "SELECT") {
            plan.push_back({"Table Scan", queryInfo.tableName, "Sequential Scan of table"});
            if (!queryInfo.condition.empty()) {
                plan.push_back({"Filter", queryInfo.condition, "Applying WHERE clause filters"});
            }
            plan.push_back({"Projection", join(queryInfo.columns), "Selecting specific columns"});
        }
        else if (queryInfo.type == "CREATE_INDEX") {
            plan.push_back({"Create Index", queryInfo.indexName, "Creating new index"});
            
            
        }
        else if (queryInfo.type == "INSERT") {
            plan.push_back({"Insert", queryInfo.tableName, "Inserting new row"});
            
        }
        else if (queryInfo.type == "DELETE") {
            plan.push_back({"Delete", queryInfo.tableName, "Deleting rows"});
            if (!queryInfo.condition.empty()) {
                plan.push_back({"Filter", queryInfo.condition, "Applying WHERE clause filters"});
            } else {
                plan.push_back({"Warning", "Full table", "All rows will be deleted"});
            }
        }
        else if (queryInfo.type == "UPDATE") {
            plan.push_back({"Update", queryInfo.tableName, "Updating rows"});
            plan.push_back({"Set", join(queryInfo.columns), "Setting new values"});
            if (!queryInfo.condition.empty()) {
                plan.push_back({"Filter", queryInfo.condition, "Applying WHERE clause filters"});
            } else {
                plan.push_back({"Warning", "Full table", "All rows will be updated"});
            }
        }
        else if (queryInfo.type == "CREATE") {
            plan.push_back({"Create", queryInfo.tableName, "Creating new table"});
            
        }

        return plan;
    
}

string ExecutionPlanGenerator::join(const vector<string>& items, const string& delimiter) {
    string result;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) result += delimiter;
        result += items[i];
    }
    return result;
}


QueryAnalyzer::QueryAnalyzer() {
  
}



vector<ExecutionStep> QueryAnalyzer::analyze(const string& query) {
    queryInfo = syntaxValidator.validateAndExtract(query);

    if (queryInfo.type == "UNKNOWN") {
        cout << "Syntax Error: Invalid query format\n";
        return {};
    }

    auto plan = planGenerator.generatePlan(queryInfo);
    return plan;
}

const QueryInfo& QueryAnalyzer::getQueryInfo() const {
    return queryInfo;
}
