#pragma once

#include <string>

// Unique identifier for all entities
class Entity {
    unsigned int m_id;
    std::string m_name;
    static unsigned int id_count; // defaults to 0 (invalid), need to init 1

  public:
    Entity(std::string name = "Entity") {
        // ensure that each entity gets a unique ID
        m_id = id_count++; // assign and increment
        m_name = name.empty() ? "Entity" : name;
    }

    /*
    Entity(Entity& e)
    {
        m_id = e.m_id;
        m_name = e.m_name;
    }
    */

    ~Entity() {}

    operator unsigned int() const { return m_id; } // enables automatic casting to int

    // Required for using entity as key in map
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
    bool operator==(const Entity &other) const { return m_id == other.m_id; }

    unsigned int id() const { return m_id; }
    std::string name() const { return m_name; }
    std::string name_id() const { return m_name + " (ID=" + std::to_string(m_id) + ")"; }
};

// Required for using entity as key in map
// https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
namespace std {
template <> struct hash<Entity> {
    std::size_t operator()(const Entity &e) const noexcept { return std::hash<unsigned int>{}(e.id()); }
};
}