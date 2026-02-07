#pragma once

#include <string>

// Unique identifier for all entities
class Entity
{
    unsigned int m_id;
    std::string m_name;
    static unsigned int id_count;   // defaults to 0 (invalid), need to init 1

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

    ~Entity()
    {
    }

    operator unsigned int() { return m_id; } // enables automatic casting to int

    unsigned int id() { return m_id; }
    std::string name() { return m_name; }
    std::string name_id() { return m_name + " (ID=" + std::to_string(m_id) + ")"; }
};