#include "ExecutionEngine.hpp"
#include <iostream>
#include <algorithm>

ExecutionEngine::ExecutionEngine(DataBase& db, BufferPool& pool)
    : Db(db), bufferPool(pool) {}

bool ExecutionEngine::Create_table(const std::string& tableName,
                                   const std::map<std::string, std::string>& schema) {
    if (!Db.createTable(tableName, schema)) {
        return false;
    }

    Table* table = Db.getTable(tableName);
    if (!table) return false;
    Page* page = table->Create_page();

    Db.updatePageCount(tableName, table->page_count);
    delete page;
    delete table;
    std::cout << "Table '" << tableName << "' created.\n";
    return true;
}

bool ExecutionEngine::insert(const std::string& tableName,
                             const std::vector<std::pair<std::string, std::pair<int, std::string>>>& attributes) {
    Table* table = Db.getTable(tableName);
    if (!table) {
        std::cerr << "Table '" << tableName << "' does not exist.\n";
        return false;
    }

    for (uint32_t pid = 1; pid <= table->page_count; pid++) {
        Page* p = bufferPool.fetchPage(pid, Db.dbname, tableName);
        if (!p) continue;

        if (!p->isFull()) {
            if (p->insert_tuple(attributes)) {
                bufferPool.markDirty(pid, Db.dbname, tableName);
                bufferPool.unpinPage(pid, Db.dbname, tableName);
                bufferPool.flushPage(pid, Db.dbname, tableName);
                delete table;
                return true;
            }
        }
        bufferPool.unpinPage(pid, Db.dbname, tableName);
    }

    Page* newPage = table->Create_page();
    uint32_t newPid = table->page_count;
    Db.updatePageCount(tableName, newPid);

    Page* p = bufferPool.fetchPage(newPid, Db.dbname, tableName);
    if (!p) {
        delete newPage;
        delete table;
        return false;
    }

    if (!p->insert_tuple(attributes)) {
        std::cerr << "Failed to insert tuple into new page.\n";
        bufferPool.unpinPage(newPid, Db.dbname, tableName);
        delete newPage;
        delete table;
        return false;
    }

    bufferPool.markDirty(newPid, Db.dbname, tableName);
    bufferPool.unpinPage(newPid, Db.dbname, tableName);
    bufferPool.flushPage(newPid, Db.dbname, tableName);
    delete newPage;
    delete table;
    return true;
}

bool ExecutionEngine::update(const std::string& tableName,
                             const std::pair<std::string, std::string>& condition,
                             const std::vector<std::pair<std::string, std::string>>& updates) {
    Table* table = Db.getTable(tableName);
    if (!table) {
        std::cerr << "Table '" << tableName << "' does not exist.\n";
        return false;
    }

    bool anyUpdated = false;

    for (uint32_t pid = 1; pid <= table->page_count; pid++) {
        Page* p = bufferPool.fetchPage(pid, Db.dbname, tableName);
        if (!p) continue;

        if (p->update_tuple(condition, updates)) {
            bufferPool.markDirty(pid, Db.dbname, tableName);
            bufferPool.flushPage(pid, Db.dbname, tableName);
            anyUpdated = true;
        }
        bufferPool.unpinPage(pid, Db.dbname, tableName);
    }

    if (anyUpdated) {
        std::cout << "Records updated in table '" << tableName << "'.\n";
    } else {
        std::cout << "No records matched the condition in table '" << tableName << "'.\n";
    }

    delete table;
    return anyUpdated;
}

bool ExecutionEngine::deleteRecord(const std::string& tableName,
                                   const std::pair<std::string, std::string>& attribute) {
    Table* table = Db.getTable(tableName);
    if (!table) {
        std::cerr << "Table '" << tableName << "' does not exist.\n";
        return false;
    }

    for (uint32_t pid = 1; pid <= table->page_count; pid++) {
        Page* p = bufferPool.fetchPage(pid, Db.dbname, tableName);
        if (!p) continue;

        p->del_tuple(attribute);
        bufferPool.markDirty(pid, Db.dbname, tableName);
        bufferPool.flushPage(pid, Db.dbname, tableName);
        bufferPool.unpinPage(pid, Db.dbname, tableName);
    }

    std::cout << "Delete operation completed on table '" << tableName << "'.\n";
    delete table;
    return true;
}

std::vector<Tuple> ExecutionEngine::select(const std::string& tableName,
                                           const std::pair<std::string, std::string>& attribute) {
    Table* table = Db.getTable(tableName);
    if (!table) {
        std::cerr << "Table '" << tableName << "' does not exist.\n";
        return {};
    }

    std::vector<Tuple> results;

    for (uint32_t pid = 1; pid <= table->page_count; pid++) {
        Page* p = bufferPool.fetchPage(pid, Db.dbname, tableName);
        if (!p) continue;

        auto pageTuples = p->get_tuple(attribute);
        results.insert(results.end(), pageTuples.begin(), pageTuples.end());
        bufferPool.unpinPage(pid, Db.dbname, tableName);
    }

    delete table;
    return results;
}
