
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

Fridge::Fridge( Catalog & catalog ) : m_catalogRef( catalog )
{
}

void Fridge::Render()
{
    using namespace ImGui;
    bool checkMatch = false;
    static std::regex r;
    static std::string searcher;
    const auto expirationWarning = std::chrono::floor<std::chrono::days>( std::chrono::system_clock::now() ) + std::chrono::days( 2 );

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

    SameLine();
    if ( Widgets::SButton( "+##add" ) )
        m_itemAddPopup = { searcher, true };
    SameLine();
    if ( Widgets::SelectorRender( m_sorting ) )
    {
        Sort();
    }

    std::optional<Itemid_t> previous = std::nullopt;
    auto removeItr = m_contents.end();
    if ( BeginTable( "##fridgeTable", 4 ) )
    {
        for ( auto itr = m_contents.begin(); itr != m_contents.end(); ++itr )
        {
            auto & itemKind = m_catalogRef[ itr->id ];
            if ( checkMatch && !std::regex_search( itemKind.name, r ) )
                continue;

            PushID( &*itr );
            bool isHeader = previous != itr->id;
            TableNextRow();
            if ( itr->expiration <= expirationWarning )
                TableSetBgColor( ImGuiTableBgTarget_RowBg0, ImColor( 120, 20, 40, 128 ) );
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

            if ( Widgets::SButton( "-##remove" ) )
                removeItr = itr;
            if ( isHeader )
            {
                SameLine();
                if ( Widgets::SButton( "+##add" ) )
                    m_itemAddPopup = { itemKind.name };
                SameLine();
            }

            PopID();

            previous = itr->id;
        }
        EndTable();
        if ( removeItr != m_contents.end() )
            m_contents.erase( removeItr );
    }

    m_itemAddPopup.Render( *this, m_catalogRef );
}

void Fridge::AddItemByName( const std::string & name, std::chrono::year_month_day ymd )
{
    auto & kinds = m_catalogRef.GetItemKinds();
    auto finder = std::ranges::find_if( kinds, [&]( const auto & pair )
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
    if ( finder != kinds.end() )
        AddItem( finder->first, ymd );
    else
        AddItem( m_catalogRef.AddItemKind( name ), ymd );
}

void Fridge::Load( const nlohmann::json & j )
{
    decltype( m_contents ) contents;
    if ( j.contains( "kinds" ) )
        for ( auto & el : j[ "kinds" ] )
        {
            if ( el.is_null() )
                continue;
            Itemid_t id = el[ "id" ];
            if ( el.contains( "items" ) )
                for ( auto & eli : el[ "items" ] )
                    contents.emplace_back( id, eli[ "quantity" ], std::chrono::sys_days( std::chrono::days( eli[ "expiration" ] ) ) );
        }
    // parsing successful
    m_contents = std::move( contents );
}

void Fridge::Save( nlohmann::json & j ) const
{
    for ( auto & item : m_contents )
    {
        j[ "kinds" ][ item.id ][ "items" ].push_back( { {"quantity", item.quantity}, {"expiration", std::chrono::sys_days( item.expiration ).time_since_epoch().count()} } );
    }
}

Itemquantity_t Fridge::Get( Itemid_t id ) const
{
    return std::accumulate( m_contents.begin(), m_contents.end(), Itemquantity_t( 0 ), [id]( auto q, const Item & item )
    {
        return q + ( item.id == id ? item.quantity : 0 );
    } );
}

void Fridge::TryRemove( Itemid_t id, Itemquantity_t amount)
{
    std::vector<std::pair<std::chrono::year_month_day, size_t>> positions;
    for ( size_t i = 0; i < m_contents.size(); ++i )
    {
        if ( m_contents[ i ].id == id )
            positions.emplace_back( m_contents[ i ].expiration, i );
    }
    std::ranges::sort( positions, {}, &std::pair<std::chrono::year_month_day, size_t>::first );
    for ( auto [ymd, i] : positions )
    {
        if ( m_contents[ i ].quantity >= amount )
        {
            m_contents[ i ].quantity -= amount;
            break;
        }
        else
        {
            amount -= m_contents[ i ].quantity;
            m_contents[ i ].quantity = 0;
        }
    }
    std::erase_if( m_contents, [id](const auto & item)
    {
        return item.quantity == 0 && item.id == id;
    } );
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
                return std::tie( m_catalogRef[ lhs.id ].name, lhs.expiration ) < std::tie( m_catalogRef[ rhs.id ].name, rhs.expiration );
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
