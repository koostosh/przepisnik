
#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

typedef uint32_t Itemid_t;
struct Item
{
    typedef uint32_t Itemquantity_t;
    Itemid_t id;
    Itemquantity_t quantity;
    std::chrono::year_month_day expiration;
};

struct ItemKind
{
    std::string name;
    enum class measurement_t
    {
        pcs, grams, custom
    } measurement;
};

class Fridge
{
public:
    Fridge();

    void Render();
private:
    void AddItem( Itemid_t type, std::chrono::year_month_day ymd = {} );
    void Sort();
    Itemid_t AddItemKind(std::string name);

    std::vector<Item> m_contents;
    std::map<Itemid_t, ItemKind> m_itemKinds;
};
