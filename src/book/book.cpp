
#include "book.hpp"

#include "textConstants.hpp"
#include "widgets.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>

#include <format>
#include <optional>

void Book::Render( const Catalog & ing, const itemQuantityGetter_t & qGetter )
{
    if ( Widgets::SButton( "+##add" ) )
    {
        m_recipes.emplace_back( Recipe{ {}, language::newRecipeName.data(), {}, {} } );
    }
    using namespace ImGui;

    for ( size_t i = 0; auto & recipe : m_recipes )
    {
        PushID( i );
        if ( Selectable( recipe.name.c_str(), false ) )
            m_displayed = std::make_unique<RecipeDisplayCtx>( recipe, i, ing, qGetter );
        PopID();
        i++;
    }

    if ( m_displayed )
    {
        auto returned = m_displayed->Render( ing );
        switch ( returned )
        {
            case RecipeDisplayCtx::returned_t::stayOpen:
                break;
            case RecipeDisplayCtx::returned_t::close:
                m_displayed.reset();
                break;
            case RecipeDisplayCtx::returned_t::saveCopy:
                m_recipes.emplace_back( m_displayed->getRecipeCopy() );
                break;
            case RecipeDisplayCtx::returned_t::saveOverwrite:
                m_recipes[ m_displayed->m_idx ] = m_displayed->getRecipeCopy();
                break;
            case RecipeDisplayCtx::returned_t::reopen:
                if ( m_displayed->m_idx < m_recipes.size() )
                    m_displayed = std::make_unique<RecipeDisplayCtx>( m_recipes[ m_displayed->m_idx ], m_displayed->m_idx, ing, qGetter );
                break;
            case RecipeDisplayCtx::returned_t::remove:
                m_recipes.erase( m_recipes.begin() + m_displayed->m_idx );
                m_displayed.reset();
                break;
        }
    }
}

void Book::Load( const nlohmann::json & j )
{
    decltype( m_recipes ) recipes;
    if ( j.contains( "recipes" ) )
        for ( auto & el : j[ "recipes" ] )
        {
            if ( el.is_null() )
                continue;
            auto & r = recipes.emplace_back();
            r.name = el[ "name" ];
            r.instructions = el[ "instructions" ];
            r.seasoning = el[ "seasoning" ];
            if ( el.contains( "ingredients" ) )
                for ( auto & ing : el[ "ingredients" ] )
                    r.ingredients.emplace_back( ing[ "id" ].get<Itemid_t>(), ing[ "quantity" ].get<Itemquantity_t>() );
        }
    // parsing successful
    m_recipes = std::move( recipes );
}

void Book::Save( nlohmann::json & j ) const
{
    for ( const auto & recipe : m_recipes )
    {
        nlohmann::json jr;
        jr[ "name" ] = recipe.name;
        jr[ "instructions" ] = recipe.instructions;
        jr[ "seasoning" ] = recipe.seasoning;
        for ( auto & ing : recipe.ingredients )
        {
            jr[ "ingredients" ].push_back( { {"id", ing.first }, {"quantity", ing.second} } );
        }

        j[ "recipes" ].push_back( std::move( jr ) );
    }
}

RecipeDisplayCtx::RecipeDisplayCtx( const Recipe & r, size_t idx, const Catalog & ing, const itemQuantityGetter_t & qGetter ) : m_r( r ), m_idx( idx ), m_ikc( ing )
{
    nameChanged();
    m_ingredients.reserve( r.ingredients.size() );
    for ( auto & element : r.ingredients )
        m_ingredients.emplace_back( std::format( "{}: {}/{}", ing[ element.first ].name, qGetter( element.first ), element.second ) );
}

RecipeDisplayCtx::returned_t RecipeDisplayCtx::Render( const itemNameGetter_t & ing )
{
    using namespace ImGui;
    bool stayOpen = true;
    returned_t ret = returned_t::stayOpen;
    if ( Begin( m_windowName.c_str(), &stayOpen ) )
    {
        if ( BeginTable( "##ingList", 2 ) )
        {
            std::optional<size_t> remove;
            for ( size_t i = 0; i < m_ingredients.size(); ++i )
            {
                PushID( i );
                TableNextRow();
                TableNextColumn();
                Text( m_ingredients[ i ].c_str() );
                if ( edit )
                {
                    TableNextColumn();
                    if ( Widgets::SButton( "-##deleteIngredient" ) )
                    {
                        remove = i;
                    }
                }
                PopID();
            }
            if ( remove )
            {
                m_ingredients.erase( m_ingredients.begin() + remove.value() );
                m_r.ingredients.erase( m_r.ingredients.begin() + remove.value() );
            }

            if ( edit )
            {
                TableNextRow();
                TableNextColumn();
                m_ikc.Render();
                TableNextColumn();
                InputScalar( "##itemcountSelector", ImGuiDataType_U32, &m_countToBeAdded );
                SameLine();
                if ( Widgets::SButton( "+##addIngredient" ) )
                {
                    m_ingredients.emplace_back( std::format( "{}: {}/{}", ing( m_ikc.selected ), 0u, m_countToBeAdded ) );
                    m_r.ingredients.emplace_back( m_ikc.selected, m_countToBeAdded );
                }
            }

            EndTable();
        }

        if ( edit )
        {
            constexpr ImVec2 textSize( -10, 150 );
            InputTextMultiline( "##instructionText", &m_r.instructions, textSize );
            InputTextMultiline( "##seasoningText", &m_r.seasoning, textSize );
            if ( InputText( "##name", &m_r.name ) )
                nameChanged();
            if ( Button( language::b_save.data() ) )
            {
                ret = returned_t::saveOverwrite;
            }

            if ( Button( language::b_saveCopy.data() ) )
            {
                ret = returned_t::saveCopy;
            }

            if ( Button( language::b_removeRecipe.data() ) )
            {
                ret = returned_t::remove;
            }

            if ( Button( language::b_discard.data() ) )
            {
                ret = returned_t::reopen;
            }
        }
        else
        {
            TextWrapped( m_r.instructions.c_str() );
            TextWrapped( m_r.seasoning.c_str() );
            if ( Button( language::b_edit.data() ) )
                edit = true;
        }
    }
    End();
    return stayOpen ? ret : returned_t::close;
}

void RecipeDisplayCtx::nameChanged()
{
    m_windowName = "Przepis: " + m_r.name + "###recipe";
}
