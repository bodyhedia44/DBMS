#include "tuple.hpp"
#include <cstring>
#include <stdexcept>

void Tuple::add_attribute(const std::string& key, const std::string& value) {
    attributes.push_back(std::make_pair(key, std::make_pair(TYPE_STRING, value)));
}

std::string Tuple::get_attribute(const std::string& key) {
    for (const auto& attr : attributes) {
        if (attr.first == key) {
            return attr.second.second;
        }
    }
    return "";
}

void Tuple::update_attribute(const std::vector<std::pair<std::string, std::string>>& attrs) {
    for (const auto& attr : attrs) {
        bool found = false;
        for (auto& existing_attr : attributes) {
            if (existing_attr.first == attr.first) {
                existing_attr.second.second = attr.second;
                found = true;
                break;
            }
        }
        if (!found) {
            add_attribute(attr.first, attr.second);
        }
    }
}

std::string Tuple::Serialize() {
    std::string data(TUPLE_SIZE, '\0'); // Initialize with 50 null bytes
    size_t offset = 0;

    for (const auto& attr : attributes) {
        // Write key length (1 byte)
        if (offset + 1 > TUPLE_SIZE) break;
        uint8_t key_length = static_cast<uint8_t>(attr.first.size());
        data[offset++] = static_cast<char>(key_length);

        // Write key (truncate if necessary)
        size_t key_write_size = std::min(attr.first.size(), static_cast<size_t>(TUPLE_SIZE - offset));
        std::memcpy(&data[offset], attr.first.data(), key_write_size);
        offset += key_write_size;

        // Write type (1 byte)
        if (offset + 1 > TUPLE_SIZE) break;
        uint8_t type = static_cast<uint8_t>(attr.second.first);
        data[offset++] = static_cast<char>(type);

        // Write value length (1 byte)
        if (offset + 1 > TUPLE_SIZE) break;
        uint8_t value_length = static_cast<uint8_t>(attr.second.second.size());
        data[offset++] = static_cast<char>(value_length);

        // Write value (truncate if necessary)
        size_t value_write_size = std::min(attr.second.second.size(), static_cast<size_t>(TUPLE_SIZE - offset));
        std::memcpy(&data[offset], attr.second.second.data(), value_write_size);
        offset += value_write_size;
    }

    return data;
}

void Tuple::Deserialize(const std::string& data) {
    attributes.clear();
    size_t offset = 0;

    while (offset + 3 <= TUPLE_SIZE) { // At least 3 bytes needed for key length, type, and value length
        // Read key length (1 byte)
        uint8_t key_length = static_cast<uint8_t>(data[offset++]);

        // Read key
        if (offset + key_length > TUPLE_SIZE) break;
        std::string key(&data[offset], key_length);
        offset += key_length;

        // Read type (1 byte)
        uint8_t type = static_cast<uint8_t>(data[offset++]);

        // Read value length (1 byte)
        uint8_t value_length = static_cast<uint8_t>(data[offset++]);

        // Read value
        if (offset + value_length > TUPLE_SIZE) break;
        std::string value(&data[offset], value_length);
        offset += value_length;

        // Add the attribute
        attributes.push_back(std::make_pair(key, std::make_pair(static_cast<AttributeType>(type), value)));
    }
}