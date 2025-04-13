#include "ExcuetionEngine.hpp"
#include <iostream>
#include <algorithm>
ExecutionEngine::ExecutionEngine(DataBase Db):Db(Db){

}




// Create a new table
bool ExecutionEngine::Create_table(const std::string& tableName, const std::map<std::string,std::string> schema) {
    Db.createTable(tableName,schema);
    Db.getTable(tableName)->Create_page();
    std::cout << "Table '" << tableName << "' created.\n";
    return true;
}

// Insert a record into a table
bool ExecutionEngine::insert(const std::string& tableName,const std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes) {
    
   Page* p =Db.getTable(tableName)->Get_page(1);

   p->insert_tuple(attributes);
   p->serialize(1,Db.dbname,tableName);

    return true;
}

// Update records in a table
bool ExecutionEngine::update(const std::string& tableName, const std::string& conditionKey, const std::string& conditionValue,
                             const std::string& updateKey, const std::string& updateValue) {
    if (!tableExists(tableName)) {
        std::cerr << "Table '" << tableName << "' does not exist.\n";
        return false;
    }
    bool updated = false;
    for (auto& record : tables[tableName]) {
        if (record[conditionKey] == conditionValue) {
            record[updateKey] = updateValue;
            updated = true;
        }
    }
    if (updated) {
        std::cout << "Records updated in table '" << tableName << "'.\n";
    } else {
        std::cout << "No records matched the condition in table '" << tableName << "'.\n";
    }
    return updated;
}

// Delete records from a table
bool ExecutionEngine::deleteRecord(std::string& tableName,const std::pair<std::string, std::string>& attribute) {
   Page* page = Db.getTable(tableName)->Get_page(1);
   page->del_tuple(attribute);
    page->serialize(1,Db.dbname,tableName);

    std::cout<<"Records Is Deleted Successfully"<<std::endl;
}

// Select records from a table
std::vector<Tuple> ExecutionEngine::select(std::string& tableName,const std::pair<std::string, std::string>& attribute){
    
    std::vector<Tuple> res= Db.getTable(tableName)->Get_page(1)->get_tuple(attribute);

    return res;
   
}

// Helper method to check if a table exists
bool ExecutionEngine::tableExists(const std::string& tableName) const {
    return tables.find(tableName) != tables.end();
}

// Helper method to check if a database exists
bool ExecutionEngine::databaseExists(const std::string& dbName) const {
    return !currentDatabase.empty() && currentDatabase == dbName;
}