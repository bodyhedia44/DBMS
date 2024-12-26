#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>
#include <sstream>
#include <iostream>
using namespace std;

enum class ColumnType {
    INTEGER,
    VARCHAR,
    BOOLEAN,
    FLOAT,
    CHAR
};

class Column {
public:
    Column(const string& name, ColumnType type)
        : name(name), type(type) {}

    string name;
    ColumnType type;
    // bool nullable;
};

class Schema {
public:
    void addTable(const string& tableName, const vector<Column>& columns) {
        tables[tableName] = columns;
        indexes[tableName] = vector<string>();
    }

    void addIndex(const string& tableName, const string& indexName) {
        if (hasTable(tableName)) {
            indexes[tableName].push_back(indexName);
        }
    }

    bool hasTable(const string& tableName) const {
        return tables.find(tableName) != tables.end();
    }

    bool hasColumn(const string& tableName, const string& columnName) const {
        if (!hasTable(tableName)) return false;
        const auto& columns = tables.at(tableName);
        return any_of(columns.begin(), columns.end(),
            [&](const Column& col) { return col.name == columnName; });
    }

private:
    unordered_map<string, vector<Column>> tables;
    unordered_map<string, vector<string>> indexes;
};

struct QueryInfo {
    string type;
    string tableName;
    vector<string> columns;
    string condition;
    vector<string> values;
    string indexName;
};

class SyntaxValidator {
public:
    QueryInfo validateAndExtract(const string& query) {
        QueryInfo info;
        smatch matches;

      // SELECT pattern that handles single column, multiple columns, and wildcard
        static const regex selectPattern(R"(SELECT\s+((\*)|(\w+)|([\w\s,]+))\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", regex_constants::icase);
        if (regex_match(query, matches, selectPattern)) {
            info.type = "SELECT";
            string columnPart = matches[1].str();
            info.tableName = matches[5].str();
            info.condition = matches.size() > 6 && matches[6].matched ? matches[6].str() : "";
            // Handle the three cases
            if (columnPart == "*") {
                info.columns = {"*"};  // Store wildcard as a special column name
            } else {
                info.columns = splitAndTrim(columnPart);
            }
            return info;
        }

        // CREATE INDEX query
        static const regex createIndexPattern(R"(CREATE\s+INDEX\s+(\w+)\s+ON\s+(\w+)\s*\((\w+)\))", regex_constants::icase);
        if (regex_match(query, matches, createIndexPattern)) {
            info.type = "CREATE_INDEX";
            info.indexName = matches[1].str();
            info.tableName = matches[2].str();
            info.columns = {matches[3].str()}; // Column to index
            return info;
        }

        // INSERT query
        static const regex insertPattern(R"(INSERT\s+INTO\s+(\w+)\s*\(([\w\s,]+)\)\s*VALUES\s*\(([\w\s',]+)\))", regex_constants::icase);
        if (regex_match(query, matches, insertPattern)) {
            info.type = "INSERT";
            info.tableName = matches[1].str();
            info.columns = splitAndTrim(matches[2].str());
            info.values = splitAndTrim(matches[3].str());
            return info;
        }

        // Modified DELETE pattern to make WHERE clause optional
        static const regex deletePattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", regex_constants::icase);
        
        // Modified UPDATE pattern to make WHERE clause optional
        static const regex updatePattern(R"(UPDATE\s+(\w+)\s+SET\s+([\w\s=,]+)(?:\s+WHERE\s+(.+))?)", regex_constants::icase);
        
        if (regex_match(query, matches, deletePattern)) {
            info.type = "DELETE";
            info.tableName = matches[1].str();
            info.condition = matches.size() > 2 && matches[2].matched ? matches[2].str() : "";
            return info;
        }
        if (regex_match(query, matches, updatePattern)) {
            info.type = "UPDATE";
            info.tableName = matches[1].str();
            info.columns = splitAndTrim(matches[2].str());
            info.condition = matches.size() > 3 && matches[3].matched ? matches[3].str() : "";
            return info;
        }
        // UPDATE query
        // static const regex updatePattern(R"(UPDATE\s+(\w+)\s+SET\s+([\w\s=,]+)\s+WHERE\s+(.+))", regex_constants::icase);
        // if (regex_match(query, matches, updatePattern)) {
        //     info.type = "UPDATE";
        //     info.tableName = matches[1].str();
        //     info.columns = splitAndTrim(matches[2].str());
        //     info.condition = matches[3].str();
        //     return info;
        // }

        // // DELETE query
        // static const regex deletePattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", regex_constants::icase);
        // if (regex_match(query, matches, deletePattern)) {
        //     info.type = "DELETE";
        //     info.tableName = matches[1].str();
        //     info.condition = matches.size() > 2 ? matches[2].str() : "";
        //     return info;
        // }

        // CREATE TABLE query
        static const regex createTablePattern(R"(CREATE\s+TABLE\s+(\w+)\s*\(([\w\s,]+)\))", regex_constants::icase);
        if (regex_match(query, matches, createTablePattern)) {
            info.type = "CREATE";
            info.tableName = matches[1].str();
            info.columns = splitAndTrim(matches[2].str());
            return info;
        }

        // If no pattern matches, return empty info
        info.type = "UNKNOWN";
        return info;
    }

private:
    vector<string> splitAndTrim(const string& str, char delimiter = ',') {
        vector<string> result;
        stringstream ss(str);
        string item;
        while (getline(ss, item, delimiter)) {
            // Trim whitespace
            item.erase(0, item.find_first_not_of(" "));
            item.erase(item.find_last_not_of(" ") + 1);
            if (!item.empty()) {  // Only add non-empty items
                result.push_back(item);
            }
        }
        return result;
    }
};

class SemanticCheck {
public:
    SemanticCheck(const Schema& schema) : schema(schema) {}

bool validate(const QueryInfo& queryInfo) {
        if (queryInfo.type == "CREATE") {
            return true; // Allow table creation
        }
        
        if (queryInfo.type == "CREATE_INDEX") {
            if (!schema.hasTable(queryInfo.tableName)) {
                lastError = "Table '" + queryInfo.tableName + "' does not exist";
                return false;
            }
            if (!schema.hasColumn(queryInfo.tableName, queryInfo.columns[0])) {
                lastError = "Column '" + queryInfo.columns[0] + "' does not exist in table '" + queryInfo.tableName + "'";
                return false;
            }
            return true;
        }

        if (!schema.hasTable(queryInfo.tableName)) {
            lastError = "Table '" + queryInfo.tableName + "' does not exist";
            return false;
        }


        if (queryInfo.type == "SELECT" && queryInfo.columns[0] != "*") {
            for (const auto& column : queryInfo.columns) {
                if (!schema.hasColumn(queryInfo.tableName, column)) {
                    lastError = "Column '" + column + "' does not exist in table '" + queryInfo.tableName + "'";
                    return false;
                }
            }
        }

        return true;
    }
    string getLastError() const { return lastError; }

private:
    const Schema& schema;
    string lastError;
};

struct ExecutionStep {
    string operation;
    string target;
    string details;
};

class ExecutionPlanGenerator {
public:
    vector<ExecutionStep> generatePlan(const QueryInfo& queryInfo) {
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
            // plan.push_back({"On Table", queryInfo.tableName, "Target table"});
            // plan.push_back({"On Column", queryInfo.columns[0], "Indexing column"});
        }
        else if (queryInfo.type == "INSERT") {
            plan.push_back({"Insert", queryInfo.tableName, "Inserting new row"});
            // plan.push_back({"Values", join(queryInfo.values), "Values to insert"});
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
            // plan.push_back({"Columns", join(queryInfo.columns), "Defining table columns"});
        }

        return plan;
    }

private:
    string join(const vector<string>& items, const string& delimiter = ", ") {
        string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) result += delimiter;
            result += items[i];
        }
        return result;
    }
};

////////////////////////

class QueryAnalyzer {
public:
    QueryAnalyzer(const Schema& schema) 
        : schema(schema), semanticCheck(schema) {}

    vector<ExecutionStep> analyze(const string& query) {
        queryInfo = syntaxValidator.validateAndExtract(query);

        if (queryInfo.type == "UNKNOWN") {
            cout << "Syntax Error: Invalid query format\n";
            return {};
        }

        if (!semanticCheck.validate(queryInfo)) {
            cout << "Semantic Error: " << semanticCheck.getLastError() << endl;
            return {};
        }

        auto plan = planGenerator.generatePlan(queryInfo);

        cout << "Query Type: " << queryInfo.type << "\n";
        cout << "Syntax: Valid\n";
        cout << "Semantics: Valid\n\n";
        
        cout << "Execution Plan:\n";
        for (const auto& step : plan) {
            cout << "  Operation: " << step.operation 
                      << ", Target: " << step.target 
                      << ", Details: " << step.details << "\n";
        }

        return plan; 
    }

    const QueryInfo& getQueryInfo() const {
        return queryInfo;
    }

private:
    const Schema& schema;
    SyntaxValidator syntaxValidator;
    SemanticCheck semanticCheck;
    ExecutionPlanGenerator planGenerator;
    QueryInfo queryInfo;  // Store the latest query info
};

// struct QueryCosts {
//     static constexpr double TABLE_SCAN_PAGE_COST = 1.0;
//     static constexpr double INDEX_SCAN_PAGE_COST = 0.1;
//     static constexpr double RANDOM_IO_COST = 4.0;
//     static constexpr double SEQUENTIAL_IO_COST = 1.0;
// };

// struct TableStatistics {
//     size_t numRows;
//     size_t numPages;
//     unordered_map<string, double> columnDistinctValues;
//     TableStatistics(size_t rows = 1000, size_t pages = 10) 
//         : numRows(rows), numPages(pages) {}
// };

class OptimizedSchema : public Schema {
public:
    struct IndexInfo {
        string indexName;
        string tableName;
        string columnName;
        bool isUnique;
        bool isClustered;
    };

    void addIndex(const string& indexName, const string& tableName, 
                 const string& columnName, bool isUnique = false, bool isClustered = false) {
        IndexInfo idx{indexName, tableName, columnName, isUnique, isClustered};
        tableIndexes[tableName].push_back(idx);
    }

    const vector<IndexInfo>& getIndexes(const string& tableName) const {
        static const vector<IndexInfo> empty;
        auto it = tableIndexes.find(tableName);
        return it != tableIndexes.end() ? it->second : empty;
    }

    // void setTableStatistics(const string& tableName, const TableStatistics& stats) {
    //     tableStats[tableName] = stats;
    // }

    // const TableStatistics& getTableStatistics(const string& tableName) const {
    //     static const TableStatistics defaultStats;
    //     auto it = tableStats.find(tableName);
    //     return it != tableStats.end() ? it->second : defaultStats;
    // }

private:
    unordered_map<string, vector<IndexInfo>> tableIndexes;
    // unordered_map<string, TableStatistics> tableStats;
};

class QueryOptimizer {
public:
    explicit QueryOptimizer(const OptimizedSchema& schema) : schema(schema) {}

    vector<ExecutionStep> optimize(const vector<ExecutionStep>& initialPlan, 
                                      const QueryInfo& queryInfo) {
                                        
        vector<ExecutionStep> optimizedPlan;
        // double totalCost = 0.0;

        if (queryInfo.type == "SELECT") {
            // optimizeSelectPlan(optimizedPlan, queryInfo, totalCost);
            optimizeSelectPlan(optimizedPlan, queryInfo);
        }
        else if (queryInfo.type == "UPDATE") {
            optimizeUpdatePlan(optimizedPlan, queryInfo);
        }
        else if (queryInfo.type == "DELETE") {
            optimizeDeletePlan(optimizedPlan, queryInfo);
        }
        else {
            optimizedPlan = initialPlan;
        }

        // optimizedPlan.push_back({
        //     "Cost Summary",
        //     "Total",
        //     "Total estimated cost: " + to_string(totalCost)
        // });

        return optimizedPlan;
    }

private:
    const OptimizedSchema& schema;

    void optimizeSelectPlan(vector<ExecutionStep>& plan,
                           const QueryInfo& queryInfo
                           ) {
                        //    double& totalCost
        // Check if we have an index that matches the condition
        bool useIndex = false;
        bool isEquality = queryInfo.condition.find('=') != string::npos;
        bool onPrimaryKey = queryInfo.condition.find("id =") != string::npos;

        if (!queryInfo.condition.empty() && isEquality && onPrimaryKey) {
            // Use index scan for equality conditions on primary key
            plan.push_back({
                "Index Scan",
                "idx_id",
                "Using primary key index for efficient access"
            });
            // totalCost += 1.0; // Index scan cost
            useIndex = true;
        }
        else if (!queryInfo.condition.empty() && isEquality) {
            // Use index scan for equality conditions if available
            plan.push_back({
                "Index Scan",
                "idx_age",
                "Using secondary index for access"
            });
            // totalCost += 2.0; // Secondary index scan cost
            useIndex = true;
        }
        else {
            // Fall back to table scan
            plan.push_back({
                "Table Scan",
                queryInfo.tableName,
                useIndex ? "Fallback to table scan" : "Full table scan required"
            });
            // totalCost += 10.0; // Table scan cost
        }

        // Add filter if there's a condition
        if (!queryInfo.condition.empty()) {
            plan.push_back({
                "Filter",
                queryInfo.condition,
                "Apply filter condition"
            });
            // totalCost += 0.5; // Filter cost
        }

        // Add projection
        plan.push_back({
            "Projection",
            join(queryInfo.columns),
            "Project final columns"
        });
        // totalCost += 0.1; // Projection cost
    }

    void optimizeUpdatePlan(vector<ExecutionStep>& plan,
                           const QueryInfo& queryInfo) {
        // Similar logic to SELECT for finding rows
        bool useIndex = false;
        bool isEquality = queryInfo.condition.find('=') != string::npos;
        bool onPrimaryKey = queryInfo.condition.find("id =") != string::npos;

        if (!queryInfo.condition.empty() && isEquality && onPrimaryKey) {
            plan.push_back({
                "Index Scan",
                "idx_id",
                "Using primary key index to locate rows"
            });
            // totalCost += 1.0;
            useIndex = true;
        }
        else {
            plan.push_back({
                "Table Scan",
                queryInfo.tableName,
                "Scanning table for rows to update"
            });
            // totalCost += 10.0;
        }

        if (!queryInfo.condition.empty()) {
            plan.push_back({
                "Filter",
                queryInfo.condition,
                "Identify rows to update"
            });
            // totalCost += 0.5;
        }

        // Add the update operation
        plan.push_back({
            "Update",
            join(queryInfo.columns),
            "Perform update on matched rows"
        });
        // totalCost += 5.0; // Update cost
    }

    void optimizeDeletePlan(vector<ExecutionStep>& plan,
                           const QueryInfo& queryInfo) {
        bool useIndex = false;
        bool isEquality = queryInfo.condition.find('=') != string::npos;
        bool onPrimaryKey = queryInfo.condition.find("id =") != string::npos;

        if (!queryInfo.condition.empty() && isEquality && onPrimaryKey) {
            plan.push_back({
                "Index Scan",
                "idx_id",
                "Using primary key index to locate rows"
            });
            // totalCost += 1.0;
            useIndex = true;
        }
        else {
            plan.push_back({
                "Table Scan",
                queryInfo.tableName,
                "Scanning table for rows to delete"
            });
            // totalCost += 10.0;
        }

        if (!queryInfo.condition.empty()) {
            plan.push_back({
                "Filter",
                queryInfo.condition,
                "Identify rows to delete"
            });
            // totalCost += 0.5;
        }

        // Add the delete operation
        plan.push_back({
            "Delete",
            queryInfo.tableName,
            queryInfo.condition.empty() ? "Delete all rows" : "Delete matched rows"
        });
        // totalCost += 5.0; // Delete cost
    }

    string join(const vector<string>& items, 
                    const string& delimiter = ", ") {
        string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) result += delimiter;
            result += items[i];
        }
        return result;
    }
};

int main() {
    OptimizedSchema schema;
    
    schema.addTable("users", {
        Column("id", ColumnType::INTEGER),
        Column("name", ColumnType::VARCHAR),
        Column("age", ColumnType::INTEGER)
    });

    // Add statistics
    // TableStatistics userStats(1000000, 10000); // 1M rows, 10K pages
    // userStats.columnDistinctValues["id"] = 1000000;  // unique
    // userStats.columnDistinctValues["age"] = 100;     // not unique
    // schema.setTableStatistics("users", userStats);

    // Add indexes
    schema.addIndex("idx_id", "users", "id", true, true);     // clustered unique index
    schema.addIndex("idx_age", "users", "age", false, false); // non-clustered non-unique index

    QueryAnalyzer analyzer(schema);
    QueryOptimizer optimizer(schema);


    const string query = "CREATE TABLE new_table (id INT, name VARCHAR)";
        
    cout << "\nAnalyzing and optimizing query: " << query << "\n";
    cout << "----------------------------------------\n";
    
    // Get initial plan and query info
    auto initialPlan = analyzer.analyze(query);
    auto queryInfo = analyzer.getQueryInfo();
    
    // Optimize plan
    auto optimizedPlan = optimizer.optimize(initialPlan, queryInfo);
    
    cout << "\nOptimized Execution Plan:\n";
    for (const auto& step : optimizedPlan) {
        cout << "  Operation: " << step.operation 
                    << ", Target: " << step.target 
                    << ", Details: " << step.details << "\n";
    }
    cout << "\n";

    // vector<string> queries = {
    //     // Queries that should use index scan
    //     "SELECT * FROM users WHERE id = 100",
    //     "SELECT name, age FROM users WHERE id = 500",
        
    //     // Queries that should use table scan
    //     "SELECT * FROM users",
    //     "SELECT * FROM users WHERE age > 30",
        
    //     // Updates and deletes
    //     "UPDATE users SET age = 25 WHERE id = 100",
    //     "DELETE FROM users WHERE id = 100",
    //     "UPDATE users SET age = 30",
    //     "DELETE FROM users"
    // };

    // for (const auto& query : queries) {
    //     cout << "\nAnalyzing and optimizing query: " << query << "\n";
    //     cout << "----------------------------------------\n";
        
    //     // Get initial plan and query info
    //     auto initialPlan = analyzer.analyze(query);
    //     auto queryInfo = analyzer.getQueryInfo();
        
    //     // Optimize plan
    //     auto optimizedPlan = optimizer.optimize(initialPlan, queryInfo);
        
    //     cout << "\nOptimized Execution Plan:\n";
    //     for (const auto& step : optimizedPlan) {
    //         cout << "  Operation: " << step.operation 
    //                  << ", Target: " << step.target 
    //                  << ", Details: " << step.details << "\n";
    //     }
    //     cout << "\n";
    // }

    return 0;
}

// int main() {
//     Schema schema;
//     schema.addTable("users", {
//         Column("id", ColumnType::INTEGER, false),
//         Column("name", ColumnType::VARCHAR, false),
//         Column("age", ColumnType::INTEGER, true)
//     });

//     QueryAnalyzer analyzer(schema);

//     // Valid queries
//     vector<string> queries = {
//         "SELECT * FROM users",
//         "SELECT name FROM users",
//         "SELECT id, name FROM users",
//         "SELECT id, name FROM users WHERE age > 30",
//         "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25)",
//         "UPDATE users SET age = 35",                  // UPDATE without WHERE
//         "UPDATE users SET age = 35 WHERE id = 1",     // UPDATE with WHERE
//         "DELETE FROM users",                          // DELETE without WHERE
//         "DELETE FROM users WHERE id = 1",             // DELETE with WHERE
//         "CREATE TABLE new_table (id INT, name VARCHAR)",
//         "CREATE INDEX idx_name ON users (name)",
        
//         // Syntax errors
//         "SELEC * FROM users",                  // Misspelled SELECT
//         "SELECT FROM users",                   // Missing columns
//         "SELECT * FORM users",                 // Misspelled FROM
//         "SELECT * FROM",                       // Missing table name
//         "SELECT , name FROM users",            // Invalid column syntax
//         "SELECT name age FROM users",          // Missing comma
//         "INSERT users (id, name)",             // Missing INTO keyword
//         "INSERT INTO users VALUES (1, 'Bob')", // Missing column list
//         "UPDATE SET age = 25",                 // Missing table name
//         "DELETE users WHERE id = 1",           // Missing FROM keyword
//         "CREATE TABLE (id INT)",               // Missing table name
        
//         // Semantic errors
//         "SELECT * FROM employees",             // Non-existent table
//         "SELECT salary FROM users",            // Non-existent column
//         "SELECT id, name FROM products",       // Non-existent table
//         "INSERT INTO products (id) VALUES (1)", // Non-existent table
//         "UPDATE customers SET age = 25",       // Non-existent table
//         "DELETE FROM orders",                  // Non-existent table
//         "CREATE INDEX idx_age ON products (age)", // Non-existent table
//         "SELECT id, age, salary FROM users",   // Non-existent column (salary)
//         "UPDATE users SET salary = 1000",      // Non-existent column
//         "CREATE INDEX idx_email ON users (email)" // Non-existent column
//     };

//     for (const auto& query : queries) {
//         cout << "\nAnalyzing query: " << query << "\n";
//         cout << "----------------------------------------\n";
//         analyzer.analyze(query);
//         cout << "\n";  // Add extra newline for better readability
//     }

//     return 0;
// }
