
#pragma once

#include <functional>
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

struct ItemQuantityControlerI
{
    virtual ~ItemQuantityControlerI()
    {
    }

    virtual Itemquantity_t Get( Itemid_t id ) const = 0;
    virtual void TryRemove( Itemid_t id, Itemquantity_t amount ) = 0;
};
