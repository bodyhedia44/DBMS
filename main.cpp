#include "DataBase.hpp"
#include "ExcuetionEngine.hpp"
#include <iostream>
#include <cstring>
int main(int argc, char* argv[]) {
    // DataBase db (argv[1]);
    // //db.createDatabase();
    // ExecutionEngine Eg(db);
    // std::map<std::string, std::string> mp;
    // mp["name"] = "Alice";
    // mp["age"] = "10";
  

    // //Eg.Create_table("table",mp);
    // std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes = {
    //     {"Name", {1, "Alice"}},       // Key: "Name", Value: {1, "Alice"}
    //     {"Age", {2, "30"}},           // Key: "Age", Value: {2, "30"}
    // };
    //     {"City", {3, "New York"}},    // Key: "City", Value: {3, "New York"}
    //     {"Occupation", {4, "Engineer"}}, // Key: "Occupation", Value: {4, "Engineer"}
    //     {"Hobby", {5, "Reading"}}     // Key: "Hobby", Value: {5, "Reading"}
    // };
    //Eg.insert("table", attributes);
    // std::string table_name = "table";
    // std::vector<Tuple> t = Eg.select(table_name,{"Name","Alice"});
    // std::cout<<t.size()<<std::endl;


    // try {
    //     // // Example usage: DB name is "MyDatabase", and file name is "DataFile"
    //     // FileHandler& writer = FileHandler::getInstance("MyDatabase", "DataFile", std::ios::out);
        
    //     // // Writing data to the file
    //     // writer.writeData("This is a test line.\n");
    //     // writer.writeData("Here's another line in the file.\n");
        
    //     // // Flush the output buffer to ensure data is written to disk
    //     // writer.flush();

    //     // std::cout << "Data successfully written to: " << writer.getFilePath() << std::endl;

    //     // // Close the current write mode and reopen in read mode
    //     // writer.reopen(std::ios::in);

    //     // // Reading the data line by line using the new readLine() method
    //     // std::string line;
    //     // std::cout << "Reading data from the file:" << std::endl;
    //     // while (writer.readLine(line)) {
    //     //     std::cout << line << std::endl;
    //     // }

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
        // std::cout << my_table->page_count<< std::endl;
        Page* ll = my_table->Get_page(1);
         std::vector<std::pair<std::string, std::pair<int, std::string>>> attributes = {
        {"Name", {1, "Alice"}},       // Key: "Name", Value: {1, "Alice"}
        {"Age", {2, "30"}},           // Key: "Age", Value: {2, "30"}
        {"City", {3, "New York"}},    // Key: "City", Value: {3, "New York"}
        {"Occupation", {4, "Engineer"}}, // Key: "Occupation", Value: {4, "Engineer"}
        {"Hobby", {5, "Reading"}}     // Key: "Hobby", Value: {5, "Reading"}
    };
        ll->insert_tuple(attributes);
        ll->serialize(1,"test","table");
        std::vector<Tuple> res=ll->get_tuple({"Name","Alice"});
        std::cout<<res.size()<<std::endl;
        std::cout<<ll->tuples.size()<<std::endl;
        std::cout<<ll->tuples[0].get_attribute("Name")<<std::endl;
        
        std::cout<<ll->tuples[0].get_attribute("City")<<std::endl;




    // } catch (const std::ios_base::failure& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    // }

    return 0;
}
