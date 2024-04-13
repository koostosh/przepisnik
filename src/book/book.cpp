
#include "book.hpp"

#include "textConstants.hpp"
#include "widgets.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>

#include <format>
#include <optional>

void Book::Render( const Catalog & ing )
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
            m_displayed = std::make_unique<RecipleDisplayCtx>( recipe, i, ing );
        PopID();
        i++;
    }

    if ( m_displayed )
    {
        auto returned = m_displayed->Render( ing );
        switch ( returned )
        {
            case RecipleDisplayCtx::returned_t::close:
                m_displayed.reset();
                break;
            case RecipleDisplayCtx::returned_t::saveCopy:
                m_recipes.emplace_back( m_displayed->getRecipeCopy() );
                break;
            case RecipleDisplayCtx::returned_t::saveOverwrite:
                m_recipes[ m_displayed->m_idx ] = m_displayed->getRecipeCopy();
                break;
            case RecipleDisplayCtx::returned_t::reopen:
                if ( m_displayed->m_idx < m_recipes.size() )
                    m_displayed = std::make_unique<RecipleDisplayCtx>( m_recipes[ m_displayed->m_idx ], m_displayed->m_idx, ing );
                break;
        }
    }
}

void Book::Load( const nlohmann::json & j )
{
    decltype( m_recipes ) recipes;
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

RecipleDisplayCtx::RecipleDisplayCtx( const Recipe & r, size_t idx, const Catalog & ing ) : m_r( r ), m_idx( idx ), m_ikc( ing )
{
    nameChanged();
    m_ingredients.reserve( r.ingredients.size() );
    for ( auto & element : r.ingredients )
        m_ingredients.emplace_back( std::format( "{}: {}/{}", ing[ element.first ].name, 0u, element.second ) );
}

RecipleDisplayCtx::returned_t RecipleDisplayCtx::Render( const itemNameGetter_t & ing )
{
    using namespace ImGui;
    bool stayOpen;
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

        constexpr ImVec2 textSize( 0, 150 );
        InputTextMultiline( "##seasoningText", &m_r.seasoning, textSize, edit ? 0 : ImGuiInputTextFlags_ReadOnly );
        InputTextMultiline( "##instructionText", &m_r.instructions, textSize, edit ? 0 : ImGuiInputTextFlags_ReadOnly );
        if ( edit )
        {
            if ( InputText( "##name", &m_r.name ) )
                nameChanged();
            if ( Button( language::b_save.data() ) )
            {
                ret = returned_t::saveCopy;
            }

            if ( Button( language::b_saveCopy.data() ) )
            {
                ret = returned_t::saveOverwrite;
            }

            if ( Button( language::b_discard.data() ) )
            {
                ret = returned_t::reopen;
            }
        }
        else
        {
            if ( Button( language::b_edit.data() ) )
                edit = true;
        }
    }
    End();
    return stayOpen ? ret : returned_t::close;
}

void RecipleDisplayCtx::nameChanged()
{
    m_windowName = "Przepis: " + m_r.name + "###recipe";
}
