#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <vector>
#include <string>
#include <utility>
#include <iostream>

class Tuple {
public:
    enum AttributeType {
        TYPE_STRING = 1,
        TYPE_INT = 2,
        
    };

    
    std::vector<std::pair<std::string, std::pair<AttributeType, std::string>>> attributes;

    
    void add_attribute(const std::string& key, const std::string& value);
    std::string get_attribute(const std::string& key);
    void update_attribute(const std::vector<std::pair<std::string, std::string>>& attrs);
    std::string Serialize(); 
    void Deserialize(const std::string& data); 

private:
    static const size_t TUPLE_SIZE = 50; 
    
};

#endif 