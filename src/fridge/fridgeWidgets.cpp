
#include "fridge.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace Widgets
{
    using namespace ImGui;

    ItemAddPopup::ItemAddPopup() : open(false)
    {
    }

    ItemAddPopup::ItemAddPopup( const std::string & hint, bool modif ) :
        name( hint ),
        expiration( std::chrono::floor<std::chrono::days>( std::chrono::system_clock::now() ) ),
        open(true)
    {
        modifiable = name.empty() || modif;
    }

    bool ItemAddPopup::Render( Fridge & parent, const Catalog & catalog )
    {
        if ( open )
        {
            OpenPopup( "addItem" );
            open = false;
        }

        if ( BeginPopup( "addItem" ) )
        {
            if ( modifiable )
            {
                InputText( "##itemAddName", &name );
                SameLine();
                if ( BeginCombo( "##itemNameCombo", "select item", ImGuiComboFlags_NoPreview ) )
                {
                    for ( const auto & pair : catalog.GetItemKinds() )
                    {
                        if ( Selectable( pair.second.name.c_str(), false ) )
                        {
                            name = pair.second.name;
                        }
                    }
                    EndCombo();
                }
            }
            else
            {
                BeginDisabled();
                InputText( "##itemAddName", &name );
                EndDisabled();
            }

            std::chrono::year_month_day ymd = expiration;
            Text( "%i-%i-%i", ymd.day(), ymd.month(), ymd.year() );
            SameLine();
            if ( SButton( "+d" ) )
                expiration += std::chrono::days( 1 );
            SameLine();
            if ( SButton( "+m" ) )
            {
                ymd += std::chrono::months( 1 );
                expiration = ymd;
            }
            SameLine();
            if ( SButton( "+y" ) )
            {
                ymd += std::chrono::years( 1 );
                expiration = ymd;
            }

            if ( Button( "Dodaj" ) )
            {
                parent.AddItemByName( name, ymd );
                CloseCurrentPopup();
            }

            EndPopup();
        }
        return false;
    }

    template <>
    bool SelectorRender( Fridge::sorting_t & value)
    {
        const bool used = SButton( "s" );
        if (used)
        {
            value = static_cast< Fridge::sorting_t >( ( static_cast< std::underlying_type_t<Fridge::sorting_t> >( value ) + 1 ) % 4 );
        }
        if ( IsItemHovered() )
        {
            switch ( value )
            {
                case Fridge::sorting_t::byId:
                    SetTooltip( "sortowanie: po id");
                    break;
                case Fridge::sorting_t::byName:
                    SetTooltip( "sortowanie: po nazwie" );
                    break;
                case Fridge::sorting_t::byClosestDate:
                    SetTooltip( "sortowanie: po dacie przydatności" );
                    break;
                case Fridge::sorting_t::byClosestDateGrouped:
                    SetTooltip( "sortowanie: po dacie przydatności (grupowane)" );
                    break;
            }

        }
        return used;
    }

}
