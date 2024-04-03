
#pragma once

#include "catalog/catalog.hpp"
#include "widgets.hpp"

#include <chrono>
#include <vector>

struct Item
{
    Itemid_t id;
    Itemquantity_t quantity;
    std::chrono::year_month_day expiration;
};

class Fridge;

namespace Widgets
{
    struct ItemAddPopup
    {
        ItemAddPopup();
        ItemAddPopup( const std::string & hint, bool modif = false );
        bool Render( Fridge & parent, const Catalog & catalog );
    private:
        std::string name;
        std::chrono::sys_days expiration;
        bool modifiable;
        bool open;
    };
}

class Fridge
{
public:
    Fridge(Catalog & catalog);

    void Render();
    void AddItemByName( const std::string & name, std::chrono::year_month_day ymd );

    void Load( const nlohmann::json & j );
    void Save( nlohmann::json & j ) const;

    enum class sorting_t
    {
        byId,
        byName,
        byClosestDate,
        byClosestDateGrouped
    };
private:
    void AddItem( Itemid_t type, std::chrono::year_month_day ymd );
    void Sort();

    sorting_t m_sorting = sorting_t::byId;

    std::vector<Item> m_contents;
    Widgets::ItemAddPopup m_itemAddPopup;
    Catalog & m_catalogRef;
};
