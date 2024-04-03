
#pragma once

#include <nlohmann/json_fwd.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <string>

typedef uint32_t Itemid_t;
typedef uint32_t Itemquantity_t;
typedef std::function<const std::string( Itemid_t )> itemNameGetter_t;

struct ItemKind
{
    std::string name;
    enum class measurement_t
    {
        pcs, grams, custom
    } measurement;
};

class Catalog;
namespace Widgets
{
    struct ItemKindCombo
    {
        ItemKindCombo( const Catalog & c ) : m_c( c )
        {
        };
        bool Render();
        Itemid_t selected = 0;
    private:
        const Catalog & m_c;
    };
}

class Catalog
{
public:
    Catalog()
    {};

    void Render();

    void Load( const nlohmann::json & j );
    void Save( nlohmann::json & j ) const;

    Itemid_t AddItemKind( std::string name );
    const ItemKind & operator[]( Itemid_t id ) const
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

    operator itemNameGetter_t() const
    {
        return [&]( auto id )
        {
            return this->operator[]( id ).name;
        };
    }
private:
    std::map<Itemid_t, ItemKind> m_itemKinds;
};

