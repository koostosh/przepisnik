
#include "catalog.hpp"
#include "widgets.hpp"

#include <nlohmann/json.hpp>
#include <imgui.h>

void Catalog::Load( const nlohmann::json & j )
{
    decltype( m_itemKinds ) kinds;
    for ( auto & el : j[ "kinds" ] )
    {
        if ( el.is_null() )
            continue;
        Itemid_t id = el[ "id" ];
        kinds.emplace( id, ItemKind{ el[ "name" ], el[ "measurement" ] } );
    }
    // parsing successful
    m_itemKinds = std::move( kinds );
}

void Catalog::Save( nlohmann::json & j ) const
{
    for ( auto & kind : m_itemKinds )
    {
        j[ "kinds" ][ kind.first ] = { {"name", kind.second.name}, {"measurement", kind.second.measurement}, {"id", kind.first} };
    }
}

Itemid_t Catalog::AddItemKind( std::string name )
{
    Itemid_t newId;
    if ( m_itemKinds.crbegin() == m_itemKinds.crend() )
        newId = 0;
    else
        newId = m_itemKinds.crbegin()->first + 1;

    m_itemKinds.emplace( newId, ItemKind{ name } );

    return newId;
}

void Catalog::Render()
{
    using namespace ImGui;
    if ( BeginTable( "##catalogTable", 2 ) )
    {
        for ( auto & kind : m_itemKinds )
        {
            PushID( kind.first );
            TableNextRow();
            TableNextColumn();
            Text( kind.second.name.c_str() );
            TableNextColumn();
            if ( Widgets::SButton( "...##edit" ) )
                OpenPopup( "##editPopup" );

            if ( BeginPopup( "##editPopup" ) )
            {
                Widgets::SelectorRender( kind.second.measurement );
                EndPopup();
            }
            PopID();
        }
        EndTable();
    }
}
