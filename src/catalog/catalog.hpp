
#pragma once

#include <nlohmann/json_fwd.hpp>

#include <cstdint>
#include <map>
#include <string>

typedef uint32_t Itemid_t;

struct ItemKind
{
    std::string name;
    enum class measurement_t
    {
        pcs, grams, custom
    } measurement;
};

class Catalog
{
public:
    Catalog()
    {};

    void Render();

    void Load( const nlohmann::json & j );
    void Save( nlohmann::json & j );

    Itemid_t AddItemKind( std::string name );
    const ItemKind & operator[]( Itemid_t id )
    {
        static ItemKind empty{};
        auto itr = m_itemKinds.find( id );
        if ( itr != m_itemKinds.end() )
            return itr->second;
        return empty;
    }
    const auto & GetItemKinds() const
    {
        return m_itemKinds;
    }
private:
    std::map<Itemid_t, ItemKind> m_itemKinds;
};

