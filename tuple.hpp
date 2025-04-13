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
        // Add more types as needed
    };

    // Attributes
    std::vector<std::pair<std::string, std::pair<AttributeType, std::string>>> attributes;

    // Methods
    void add_attribute(const std::string& key, const std::string& value);
    std::string get_attribute(const std::string& key);
    void update_attribute(const std::vector<std::pair<std::string, std::string>>& attrs);
    std::string Serialize(); // Returns a fixed-size 50-byte string
    void Deserialize(const std::string& data); // Deserializes from a 50-byte string

private:
    static const size_t TUPLE_SIZE = 50; // Fixed size of 50 bytes
    // Additional private members can be added if needed for serialization
};

#endif // TUPLE_HPP