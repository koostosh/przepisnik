
#include "fridge.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

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
    auto now_ymd = std::chrono::floor<std::chrono::days>( std::chrono::system_clock::now() );
    m_contents.emplace_back( 1, 2, now_ymd );
    m_contents.emplace_back( 1, 12, now_ymd + std::chrono::days( 2 ) );
    m_contents.emplace_back( 2, 3, now_ymd );
    m_contents.emplace_back( 3, 250, now_ymd );
    m_itemKinds[ 1 ] = { "JAJO", ItemKind::measurement_t::pcs };
    m_itemKinds[ 2 ] = { "NIE JAJO", ItemKind::measurement_t::pcs };
    m_itemKinds[ 3 ] = { "ser", ItemKind::measurement_t::grams };
}

void Fridge::Render()
{
    using namespace ImGui;
    bool checkMatch = false;
    static std::regex r;

    {
        static std::string searcher;
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
    {
        static std::chrono::sys_days itemAddExpiration;
        if ( Button( "+##add", btSize ) )
        {
            OpenPopup( "addItem" );
            itemAddExpiration = std::chrono::floor<std::chrono::days>( std::chrono::system_clock::now() );
        }

        if ( BeginPopup( "addItem" ) )
        {
            static std::string itemAddName;
            InputText( "##itemAddName", &itemAddName );
            SameLine();
            if ( BeginCombo( "##itemNameCombo", "select item", ImGuiComboFlags_NoPreview ) )
            {
                for ( const auto & pair : m_itemKinds )
                {
                    if ( Selectable( pair.second.name.c_str(), false ) )
                    {
                        itemAddName = pair.second.name;
                    }
                }
                EndCombo();
            }

            std::chrono::year_month_day ymd = itemAddExpiration;
            Text( "%i-%i-%i", ymd.day(), ymd.month(), ymd.year() );
            SameLine();
            if ( Button( "+d", btSize ) )
                itemAddExpiration += std::chrono::days( 1 );
            SameLine();
            if ( Button( "+m", btSize ) )
            {
                ymd += std::chrono::months( 1 );
                itemAddExpiration = ymd;
            }
            SameLine();
            if ( Button( "+y", btSize ) )
            {
                ymd += std::chrono::years( 1 );
                itemAddExpiration = ymd;
            }

            if ( Button( "Dodaj" ) )
            {
                auto finder = std::ranges::find( m_itemKinds, itemAddName, []( const auto & pair )
                {
                    return pair.second.name;
                } );
                if ( finder != m_itemKinds.end() )
                    AddItem( finder->first, ymd );
                else
                    AddItem( AddItemKind( itemAddName ), ymd );
            }

            EndPopup();
        }
    }

    std::optional<Itemid_t> previous = std::nullopt;
    std::optional<Itemid_t> addEntry = std::nullopt;
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
                    addEntry = itr->id;
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
        if ( addEntry )
            AddItem( addEntry.value() );
    }
}

void Fridge::AddItem( Itemid_t type, std::chrono::year_month_day ymd )
{
    m_contents.emplace_back( type, 0u, ymd );
    Sort();
}

void Fridge::Sort()
{
    std::ranges::sort( m_contents, []( const Item & lhs, const Item & rhs )
    {
        return lhs.id < rhs.id;
    } );
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
