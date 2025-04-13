#include "DataBase.hpp"
#include "ExcuetionEngine.hpp"
#include <iostream>
#include <cstring>
#include <vector>
#include "parser.hpp"
#include <iomanip>
#include <string>
using namespace std;
void printTuples(const std::vector<Tuple>& tuples);
std::pair<std::string, std::string> extractFirstAndLast(const std::string& input);

int main(int argc, char* argv[]){

    DataBase db (argv[1]);
    db.createDatabase();
    ExecutionEngine Eg(db);
    QueryAnalyzer analyzer = QueryAnalyzer();
     while(true)
    {
        string query;
        cout<<"Enter your query: ";
        getline(cin, query);
        if(query=="exit")
        {
            break;
        }
    //const string query =  "CREATE TABLE new_table (id INT, name VARCHAR)";
    //const string query = "INSERT INTO new_table (name, age) VALUES ('Alice', 25)";
    //const string query = "SELECT * FROM new_table";
    //const string query = "SELECT * FROM new_table WHERE id = 1";
    //const string query = "DELETE FROM new_table WHERE id = 0";
    
    auto initialPlan = analyzer.analyze(query);
    auto queryInfo = analyzer.getQueryInfo();
    //std::cout<<queryInfo.condition.size()<<std::endl;

    // std::cout<<extractFirstAndLast(queryInfo.condition).first<<std::endl;
    // std::cout<<extractFirstAndLast(queryInfo.condition).second<<std::endl;
    cout << "\nOptimized Execution Plan:\n";
    for (const auto& step : initialPlan) {
        cout << "  Operation: " << step.operation 
                    << ", Target: " << step.target 
                    << ", Details: " << step.details << "\n";
    }
    cout << "\n";
    std::vector<string> col= queryInfo.columns;
    std::vector<string> val= queryInfo.values;
    if(queryInfo.type == "CREATE"){
        std::map<std::string, std::string> mp;
        for(int i = 0;i<col.size();i++){
            std::string input = col[i];
            size_t spacePos = input.find(' '); // Find the position of the first space

            if (spacePos != std::string::npos) {
            // Split the string into two parts
                std::string first = input.substr(0, spacePos); // From start to the space
                std::string second = input.substr(spacePos + 1); // From after the space to the end
                mp[first] = second;
            }

        }
        Eg.Create_table(queryInfo.tableName, mp);
    }

    else if(queryInfo.type == "INSERT"){
        std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes;
        for(int i = 0;i<val.size();i++){
            attributes.push_back({col[i],{i,val[i]}});
        }
        Eg.insert(queryInfo.tableName,attributes);
    }

    else if(queryInfo.type == "SELECT"){
        if (queryInfo.condition.size() == 0){
            std::vector<Tuple> r =Eg.select(queryInfo.tableName,{" "," "});
            printTuples(r);
        }
        else{
            std::pair p =extractFirstAndLast(queryInfo.condition);
            std::vector<Tuple> r =Eg.select(queryInfo.tableName,{p.first,p.second});
            printTuples(r);
        }
    } else if(queryInfo.type == "DELETE"){
        if (queryInfo.condition.size() == 0){
            Eg.deleteRecord(queryInfo.tableName,{" "," "});

        }
        else{
                Eg.deleteRecord(queryInfo.tableName,{" "," "});
        }
    }

    }
   


    return 0;
}

void printTuples(const std::vector<Tuple>& tuples) {
    if (tuples.empty()) {
        std::cout << "No tuples to display.\n";
        return;
    }

    // Print table header
    std::cout << std::left << std::setw(15) << "Key" << std::setw(15) << "Value" << "\n";
    std::cout << std::setfill('-') << std::setw(30) << "" << std::setfill(' ') << "\n";

    // Iterate over each tuple and print its attributes
    for ( auto tuple : tuples) {
        std::string s = "id";
        if(tuple.get_attribute(s).size()!=0){
        for (const auto& attr : tuple.attributes) {
            
            std::cout << std::left << std::setw(15) << attr.first << std::setw(15) ;
            std::cout<<attr.second.second<< "\n";
        }
        }
        std::cout << std::setfill('-') << std::setw(30) << "" << std::setfill(' ') << "\n";
    }
}


std::pair<std::string, std::string> extractFirstAndLast(const std::string& input) {
    // Find the first space
    size_t firstSpace = input.find(' ');
    if (firstSpace == std::string::npos) {
        throw std::invalid_argument("No spaces found in the string.");
    }

    // Find the last space
    size_t lastSpace = input.rfind(' ');
    if (lastSpace == std::string::npos || lastSpace == firstSpace) {
        throw std::invalid_argument("Only one space found in the string.");
    }

    // Extract the first substring (before the first space)
    std::string first = input.substr(0, firstSpace);

    // Extract the last substring (after the last space)
    std::string last = input.substr(lastSpace + 1);

    return {first, last};
}
// int main(int argc, char* argv[]) {
//     // DataBase db (argv[1]);
//     // // db.createDatabase();
//     // ExecutionEngine Eg(db);
//     // std::map<std::string, std::string> mp;
//     // mp["name"] = "Alice";
//     // mp["age"] = "10";
  

//     // // Eg.Create_table("table",mp);
//     // std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes = {
//     //     {"name", {1, "Alice"}},       // Key: "Name", Value: {1, "Alice"}
//     //     {"age", {2, "30"}},           // Key: "Age", Value: {2, "30"}
//     // };
// //    db.getTable("table")->Get_page(1)->insert_tuple(attributes);
//     // std::vector<Tuple> v = Eg.select_("table",{"name","Alice"});
//     // std::cout<<v.size()<<std::endl;






//         DataBase db = DataBase( argv[1]);
//         std::map<std::string, std::string> mp;
        
  
  
//         mp["one"] = "Alice";
//         mp["rwo"] = "Addlice";
//         mp["ddd"] = "Addlice";
//         mp["oddne"] = "Aliddce";
//         db.createDatabase();
//         db.createTable("table",mp);
     
//         Table* my_table = db.getTable("table");   
//         Page* page = my_table->Create_page();
//         std::cout << my_table->page_count<< std::endl;
//     //     Page* ll = my_table->Get_page(1);
//     //      std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes = {
//     //     {"Name", {1, "Alice"}},       // Key: "Name", Value: {1, "Alice"}
//     //     {"Age", {2, "30"}},           // Key: "Age", Value: {2, "30"}
//     //     {"City", {3, "New York"}},    // Key: "City", Value: {3, "New York"}
//     //     {"Occupation", {4, "Engineer"}}, // Key: "Occupation", Value: {4, "Engineer"}
//     //     {"Hobby", {5, "Reading"}}     // Key: "Hobby", Value: {5, "Reading"}
//     // };
//     //     ll->insert_tuple(attributes);
//     //     ll->serialize(1,"test","table");
//     //     std::vector<Tuple> res=ll->get_tuple({"Name","Alice"});
//     //     std::cout<<res.size()<<std::endl;
//     //     std::cout<<ll->tuples.size()<<std::endl;
//     //     std::cout<<ll->tuples[0].get_attribute("Name")<<std::endl;
        
//     //     std::cout<<ll->tuples[0].get_attribute("City")<<std::endl;




//     // } catch (const std::ios_base::failure& e) {
//     //     std::cerr << "Error: " << e.what() << std::endl;
//     // }

//     return 0;
// }
