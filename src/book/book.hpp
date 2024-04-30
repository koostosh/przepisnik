
#pragma once

#include "catalog/catalog.hpp"

#include <nlohmann/json_fwd.hpp>

#include <vector>


struct Recipe
{
    std::vector<std::pair<Itemid_t, Itemquantity_t>> ingredients;
    std::string name;
    std::string seasoning;
    std::string instructions;
};

class RecipeDisplayCtx
{
public:
    enum class returned_t
    {
        stayOpen,
        close,
        saveCopy,
        saveOverwrite,
        reopen,
        remove
    };

    RecipeDisplayCtx( const Recipe & r, size_t idx, const Catalog & ing, const itemQuantityGetter_t & qGetter );
    returned_t Render( const itemNameGetter_t & ing );

    const size_t m_idx;
    Recipe getRecipeCopy() const
    {
        return m_r;
    };
private:
    void nameChanged();
    std::vector<std::string> m_ingredients;
    std::string m_windowName;
    Recipe m_r;
    Itemquantity_t m_countToBeAdded = 0;
    Widgets::ItemKindCombo m_ikc;
    bool edit = false;
};

class Book
{
public:
    Book()
    {
    };

    void Render( const Catalog & ing, const itemQuantityGetter_t & qGetter );

    void Load( const nlohmann::json & j );
    void Save( nlohmann::json & j ) const;
private:
    std::unique_ptr<RecipeDisplayCtx> m_displayed;
    std::vector<Recipe> m_recipes;
};