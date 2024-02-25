
#include "fridge.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace Widgets
{

    constexpr ImVec2 btSize{ 20,20 };

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

    bool ItemAddPopup::Render( Fridge & parent )
    {
        using namespace ImGui;
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
                    for ( const auto & pair : parent.GetItemKinds() )
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
            if ( Button( "+d", btSize ) )
                expiration += std::chrono::days( 1 );
            SameLine();
            if ( Button( "+m", btSize ) )
            {
                ymd += std::chrono::months( 1 );
                expiration = ymd;
            }
            SameLine();
            if ( Button( "+y", btSize ) )
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
}