
#include "fridge.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>

#include <optional>
#include <ranges>
#include <regex>

const char * measurementFormatter( const ItemKind & ik )
{
    switch ( ik.measurement )
    {
        case ItemKind::measurement_t::pcs:
            return "szt.";
        case ItemKind::measurement_t::grams:
            return "g";
        case ItemKind::measurement_t::custom:
        default:
            return "";
    }
}

Fridge::Fridge()
{
}

void Fridge::Render()
{
    using namespace ImGui;
    bool checkMatch = false;
    static std::regex r;
    static std::string searcher;

    {
        bool changed = InputText( "##searcher", &searcher );
        checkMatch = !searcher.empty();
        if ( changed )
        {
            try
            {
                r = std::regex( searcher, std::regex_constants::icase );
            }
            catch ( ... )
            {
                checkMatch = false;
            }
        }
    }

    constexpr ImVec2 btSize{ 20,20 };
    SameLine();
    if ( Button( "+##add", btSize ) )
        m_itemAddPopup = { searcher, true };
    SameLine();
    if ( Button( "s##sort", btSize ) )
    {
        m_sorting = static_cast< sorting_t >( ( static_cast< std::underlying_type_t<sorting_t> >( m_sorting ) + 1 ) % 4 );
        Sort();
    }

    std::optional<Itemid_t> previous = std::nullopt;
    auto removeItr = m_contents.end();
    if ( BeginTable( "##fridgeTable", 4 ) )
    {
        for ( auto itr = m_contents.begin(); itr != m_contents.end(); ++itr )
        {
            auto & itemKind = m_itemKinds[ itr->id ];
            if ( checkMatch && !std::regex_search( itemKind.name, r ) )
                continue;

            PushID( &*itr );
            bool isHeader = previous != itr->id;
            TableNextRow();
            TableNextColumn();
            if ( isHeader )
                Text( itemKind.name.c_str() );
            TableNextColumn();
            InputScalar( "##quantity", ImGuiDataType_U32, &itr->quantity );
            SameLine();
            Text( measurementFormatter( itemKind ) );
            TableNextColumn();
            if ( itr->expiration.ok() )
                Text( "%i-%i-%i", itr->expiration.day(), itr->expiration.month(), itr->expiration.year() );

            TableNextColumn();

            if ( Button( "-##remove", btSize ) )
                removeItr = itr;
            if ( isHeader )
            {
                SameLine();
                if ( Button( "+##add", btSize ) )
                    m_itemAddPopup = { itemKind.name };
                SameLine();
                if ( Button( "...##edit", btSize ) )
                    OpenPopup( "##editPopup" );

                if ( BeginPopup( "##editPopup" ) )
                {
                    if ( Selectable( "pcs", itemKind.measurement == ItemKind::measurement_t::pcs ) )
                        itemKind.measurement = ItemKind::measurement_t::pcs;
                    if ( Selectable( "grams", itemKind.measurement == ItemKind::measurement_t::grams ) )
                        itemKind.measurement = ItemKind::measurement_t::grams;
                    if ( Selectable( "custom", itemKind.measurement == ItemKind::measurement_t::custom ) )
                        itemKind.measurement = ItemKind::measurement_t::custom;
                    EndPopup();
                }
            }

            PopID();

            previous = itr->id;
        }
        EndTable();
        if ( removeItr != m_contents.end() )
            m_contents.erase( removeItr );
    }

    m_itemAddPopup.Render( *this );
}

void Fridge::AddItemByName( const std::string & name, std::chrono::year_month_day ymd )
{
    auto finder = std::ranges::find_if( m_itemKinds, [&]( const auto & pair )
    {
        if ( pair.second.name.size() != name.size() )
            return false;
        for ( size_t i = 0; i < name.size(); ++i )
        {
            if ( std::tolower( name[ i ], std::locale() ) != std::tolower( pair.second.name[ i ], std::locale() ) )
                return false;
        }
        return true;
    } );
    if ( finder != m_itemKinds.end() )
        AddItem( finder->first, ymd );
    else
        AddItem( AddItemKind( name ), ymd );
}

void Fridge::AddItem( Itemid_t type, std::chrono::year_month_day ymd )
{
    m_contents.emplace_back( type, 0u, ymd );
    Sort();
}

void Fridge::Sort()
{
    switch ( m_sorting )
    {
        default:
        case Fridge::sorting_t::byId:
            std::ranges::sort( m_contents, {}, &Item::id );
            break;
        case Fridge::sorting_t::byName:
            std::ranges::sort( m_contents, [&]( const Item & lhs, const Item & rhs )
            {
                return std::tie( m_itemKinds[ lhs.id ].name, lhs.expiration ) < std::tie( m_itemKinds[ rhs.id ].name, rhs.expiration );
            } );
            break;
        case Fridge::sorting_t::byClosestDate:
            std::ranges::sort( m_contents, {}, &Item::expiration );
            break;
        case Fridge::sorting_t::byClosestDateGrouped:
        {
            std::map<Itemid_t, std::chrono::year_month_day> sortKey;
            for ( const auto & item : m_contents )
            {
                if ( !sortKey.emplace( item.id, item.expiration ).second )
                    sortKey[ item.id ] = std::min( sortKey[ item.id ], item.expiration );
            }
            std::ranges::sort( m_contents, [&]( const Item & lhs, const Item & rhs )
            {
                return std::tie( sortKey[ lhs.id ], lhs.id, lhs.expiration ) < std::tie( sortKey[ rhs.id ], rhs.id, rhs.expiration );
            } );
            break;
        }
    }
}

Itemid_t Fridge::AddItemKind( std::string name )
{
    Itemid_t newId;
    if ( m_itemKinds.crbegin() == m_itemKinds.crend() )
        newId = 0;
    else
        newId = m_itemKinds.crbegin()->first + 1;

    m_itemKinds[ newId ].name = name;

    return newId;
}

std::ostream & operator<<( std::ostream & os, const Fridge & fr )
{
    nlohmann::json j;
    for ( auto & kind : fr.m_itemKinds )
    {
        j[ "kinds" ][ kind.first ] = { {"name", kind.second.name}, {"measurement", kind.second.measurement} };
    }
    for ( auto & item : fr.m_contents )
    {
        j[ "kinds" ][ item.id ][ "items" ].push_back( { {"quantity", item.quantity}, {"expiration", std::chrono::sys_days( item.expiration ).time_since_epoch().count()} } );
    }
    return os << j.dump(4);
}

std::istream & operator>>( std::istream & os, Fridge & fr )
{
    try
    {
        nlohmann::json j;
        os >> j;
        Itemid_t id = 0;
        decltype( fr.m_itemKinds ) kinds;
        decltype( fr.m_contents ) contents;
        for ( auto & el : j[ "kinds" ] )
        {
            if ( el.is_null() )
                continue;
            kinds[ id ].name = el[ "name" ];
            kinds[ id ].measurement = el[ "measurement" ];
            for ( auto & eli : el[ "items" ] )
                contents.emplace_back( id, eli[ "quantity" ], std::chrono::sys_days( std::chrono::days( eli[ "expiration" ] ) ) );
            ++id;
        }
        // parsing successful
        fr.m_itemKinds = std::move( kinds );
        fr.m_contents = std::move( contents );
    }
    catch ( ... )
    {
        // TODO: inform user the file is damaged
    }
    return os;
}
