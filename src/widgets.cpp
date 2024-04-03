
#include "widgets.hpp"

#include "catalog/catalog.hpp"

#include <imgui.h>

namespace Widgets
{
    using namespace ImGui;
    constexpr ImVec2 c_btSize{ 20, 20 };
    bool SButton( const char * label )
    {
        return Button( label, c_btSize );
    }

    template <>
    bool SelectorRender( ItemKind::measurement_t & value )
    {
        if ( Selectable( "pcs", value == ItemKind::measurement_t::pcs ) )
        {
            value = ItemKind::measurement_t::pcs;
            return true;
        }
        if ( Selectable( "grams", value == ItemKind::measurement_t::grams ) )
        {
            value = ItemKind::measurement_t::grams;
            return true;
        }
        if ( Selectable( "custom", value == ItemKind::measurement_t::custom ) )
        {
            value = ItemKind::measurement_t::custom;
            return true;
        }
        return false;
    }
}
