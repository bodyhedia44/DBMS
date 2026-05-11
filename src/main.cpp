#include "DataBase.hpp"
#include "ExecutionEngine.hpp"
#include "Buffer.hpp"
#include "parser.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

void printTuples(const vector<Tuple>& tuples, const vector<string>& columns);
pair<string, string> parseCondition(const string& input);
vector<pair<string, string>> parseSetClause(const vector<string>& assignments);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <database_name>\n";
        return 1;
    }

    DataBase db(argv[1]);
    db.createDatabase();

    BufferPool pool(BUFFER_POOL_SIZE);
    ExecutionEngine engine(db, pool);
    QueryAnalyzer analyzer;

    cout << "DBMS Interactive Shell. Type 'exit' to quit.\n\n";

    while (true) {
        string query;
        cout << "sql> ";
        if (!getline(cin, query)) {
            cout << "\n";
            break;
        }

        size_t start = query.find_first_not_of(" \t");
        size_t end = query.find_last_not_of(" \t");
        if (start == string::npos) continue;
        query = query.substr(start, end - start + 1);

        if (query == "exit" || query == "quit") break;

        if (query == "buffer" || query == "BUFFER") {
            pool.displayBufferPool();
            continue;
        }

        auto plan = analyzer.analyze(query);
        auto queryInfo = analyzer.getQueryInfo();

        if (queryInfo.type == "UNKNOWN") continue;

        cout << "\nExecution Plan:\n";
        for (const auto& step : plan) {
            cout << "  " << step.operation << " -> " << step.target
                 << " (" << step.details << ")\n";
        }
        cout << "\n";

        const auto& col = queryInfo.columns;
        const auto& val = queryInfo.values;

        if (queryInfo.type == "CREATE") {
            map<string, string> mp;
            for (size_t i = 0; i < col.size(); i++) {
                const string& input = col[i];
                size_t spacePos = input.find(' ');
                if (spacePos != string::npos) {
                    mp[input.substr(0, spacePos)] = input.substr(spacePos + 1);
                }
            }
            engine.Create_table(queryInfo.tableName, mp);
        }

        else if (queryInfo.type == "INSERT") {
            vector<pair<string, pair<int, string>>> attributes;
            for (size_t i = 0; i < val.size(); i++) {
                attributes.push_back({col[i], {static_cast<int>(i), val[i]}});
            }
            engine.insert(queryInfo.tableName, attributes);
        }

        else if (queryInfo.type == "SELECT") {
            vector<Tuple> r;
            if (queryInfo.condition.empty()) {
                r = engine.select(queryInfo.tableName, {" ", " "});
            } else {
                auto cond = parseCondition(queryInfo.condition);
                r = engine.select(queryInfo.tableName, cond);
            }
            printTuples(r, col);
        }

        else if (queryInfo.type == "DELETE") {
            if (queryInfo.condition.empty()) {
                engine.deleteRecord(queryInfo.tableName, {" ", " "});
            } else {
                auto cond = parseCondition(queryInfo.condition);
                engine.deleteRecord(queryInfo.tableName, cond);
            }
        }

        else if (queryInfo.type == "UPDATE") {
            auto updates = parseSetClause(queryInfo.columns);
            if (updates.empty()) {
                cerr << "Error: Could not parse SET clause.\n";
                continue;
            }
            if (queryInfo.condition.empty()) {
                engine.update(queryInfo.tableName, {" ", " "}, updates);
            } else {
                auto cond = parseCondition(queryInfo.condition);
                engine.update(queryInfo.tableName, cond, updates);
            }
        }
    }

    cout << "Flushing buffer pool and shutting down...\n";
    pool.flushAll();
    return 0;
}

pair<string, string> parseCondition(const string& input) {

    size_t eqPos = input.find('=');
    if (eqPos == string::npos) {

        size_t firstSpace = input.find(' ');
        size_t lastSpace = input.rfind(' ');
        if (firstSpace == string::npos || lastSpace == firstSpace) {
            return {input, ""};
        }
        string col = input.substr(0, firstSpace);
        string val = input.substr(lastSpace + 1);
        return {col, val};
    }

    string col = input.substr(0, eqPos);
    string val = input.substr(eqPos + 1);

    col.erase(col.find_last_not_of(" \t") + 1);
    col.erase(0, col.find_first_not_of(" \t"));
    val.erase(val.find_last_not_of(" \t") + 1);
    val.erase(0, val.find_first_not_of(" \t'\""));
    val.erase(val.find_last_not_of("'\"") + 1);
    return {col, val};
}

vector<pair<string, string>> parseSetClause(const vector<string>& assignments) {
    vector<pair<string, string>> result;
    for (const auto& assign : assignments) {
        size_t eqPos = assign.find('=');
        if (eqPos == string::npos) continue;

        string col = assign.substr(0, eqPos);
        string val = assign.substr(eqPos + 1);

        col.erase(col.find_last_not_of(" \t") + 1);
        col.erase(0, col.find_first_not_of(" \t"));
        val.erase(val.find_last_not_of(" \t") + 1);
        val.erase(0, val.find_first_not_of(" \t'\""));
        val.erase(val.find_last_not_of("'\"") + 1);

        result.push_back({col, val});
    }
    return result;
}

void printTuples(const vector<Tuple>& tuples, const vector<string>& ) {
    if (tuples.empty()) {
        cout << "(0 rows)\n";
        return;
    }

    vector<string> headers;
    for (const auto& attr : tuples[0].attributes) {
        headers.push_back(attr.first);
    }

    if (headers.empty()) {
        cout << "(0 rows)\n";
        return;
    }

    vector<size_t> widths(headers.size());
    for (size_t i = 0; i < headers.size(); i++) {
        widths[i] = headers[i].size();
    }
    for (const auto& tuple : tuples) {
        for (size_t i = 0; i < headers.size(); i++) {

            for (const auto& attr : tuple.attributes) {
                if (attr.first == headers[i]) {
                    widths[i] = max(widths[i], attr.second.second.size());
                    break;
                }
            }
        }
    }

    for (auto& w : widths) w += 2;

    cout << " ";
    for (size_t i = 0; i < headers.size(); i++) {
        cout << left << setw(static_cast<int>(widths[i])) << headers[i];
        if (i + 1 < headers.size()) cout << "| ";
    }
    cout << "\n";

    for (size_t i = 0; i < headers.size(); i++) {
        cout << string(widths[i], '-');
        if (i + 1 < headers.size()) cout << "+-";
    }
    cout << "\n";

    for (const auto& tuple : tuples) {
        cout << " ";
        for (size_t i = 0; i < headers.size(); i++) {
            string val;
            for (const auto& attr : tuple.attributes) {
                if (attr.first == headers[i]) {
                    val = attr.second.second;
                    break;
                }
            }
            cout << left << setw(static_cast<int>(widths[i])) << val;
            if (i + 1 < headers.size()) cout << "| ";
        }
        cout << "\n";
    }

    cout << "(" << tuples.size() << " row" << (tuples.size() != 1 ? "s" : "") << ")\n";
}
