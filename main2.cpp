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
    
    auto initialPlan = analyzer.analyze(query);
    auto queryInfo = analyzer.getQueryInfo();
    
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
            size_t spacePos = input.find(' '); 

            if (spacePos != std::string::npos) {
            
                std::string first = input.substr(0, spacePos); 
                std::string second = input.substr(spacePos + 1); 
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

    
    std::cout << std::left << std::setw(15) << "Key" << std::setw(15) << "Value" << "\n";
    std::cout << std::setfill('-') << std::setw(30) << "" << std::setfill(' ') << "\n";

    
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
    
    size_t firstSpace = input.find(' ');
    if (firstSpace == std::string::npos) {
        throw std::invalid_argument("No spaces found in the string.");
    }

    
    size_t lastSpace = input.rfind(' ');
    if (lastSpace == std::string::npos || lastSpace == firstSpace) {
        throw std::invalid_argument("Only one space found in the string.");
    }

    
    std::string first = input.substr(0, firstSpace);

    
    std::string last = input.substr(lastSpace + 1);

    return {first, last};
}







  

















        
  
  






     

















        











