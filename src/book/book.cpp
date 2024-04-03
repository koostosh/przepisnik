
#include "book.hpp"

#include "widgets.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <format>
#include <optional>

void Book::Render( const itemNameGetter_t & ing )
{
    if ( Widgets::SButton( "+##add" ) )
    {
        m_recipes.emplace_back( Recipe{ {}, "nowy przepis", {}, {} } );
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
}

void Book::Save( nlohmann::json & j ) const
{
}

RecipleDisplayCtx::RecipleDisplayCtx( const Recipe & r, size_t idx, const itemNameGetter_t & ing ) : m_r( r ), m_idx( idx )
{
    nameChanged();
    m_ingredients.reserve( r.ingredients.size() );
    for ( auto & element : r.ingredients )
        m_ingredients.emplace_back( std::format( "{}: {}/{}", ing( element.first ), 0u, element.second ) );
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
                //TODO: handle item selection and adding
                TableNextRow();
                TableNextColumn();
                InputScalar( "##itemidSelector", ImGuiDataType_U32, &m_itemToBeAdded );
                TableNextColumn();
                InputScalar( "##itemcountSelector", ImGuiDataType_U32, &m_countToBeAdded );
                SameLine();
                if ( Widgets::SButton( "+##addIngredient" ) )
                {
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
            if ( Button( "Save as copy" ) )
            {
                ret = returned_t::saveCopy;
            }

            if ( Button( "Save as overwrite" ) )
            {
                ret = returned_t::saveOverwrite;
            }

            if ( Button( "Discard changes" ) )
            {
                ret = returned_t::reopen;
            }
        }
        else
        {
            if ( Button( "Edit" ) )
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
