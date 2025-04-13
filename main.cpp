#include "DataBase.hpp"
#include "ExcuetionEngine.hpp"
#include <iostream>
#include <cstring>
int main(int argc, char* argv[]) {
        DataBase db = DataBase( argv[1]);
        std::map<std::string, std::string> mp;
        
  
  
        mp["one"] = "Alice";
        mp["rwo"] = "Addlice";
        mp["ddd"] = "Addlice";
        mp["oddne"] = "Aliddce";
        db.createDatabase();
        db.createTable("table",mp);
     
        Table* my_table = db.getTable("table");   
        Page* page = my_table->Create_page();
        
        Page* ll = my_table->Get_page(1);
         std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes = {
        {"Name", {1, "Alice"}},       
        {"Age", {2, "30"}},           
        {"City", {3, "New York"}},    
        {"Occupation", {4, "Engineer"}}, 
        {"Hobby", {5, "Reading"}}     
    };
        ll->insert_tuple(attributes);
        ll->serialize(1,"test","table");
        std::vector<Tuple> res=ll->get_tuple({"Name","Alice"});
        std::cout<<res.size()<<std::endl;
        std::cout<<ll->tuples.size()<<std::endl;
        std::cout<<ll->tuples[0].get_attribute("Name")<<std::endl;
        
        std::cout<<ll->tuples[0].get_attribute("City")<<std::endl;

    return 0;
}
