
#include "widgets.hpp"

#include "textConstants.hpp"
#include "catalog/catalog.hpp"

#include <imgui.h>

namespace Widgets
{
    using namespace ImGui;

    template <>
    bool SelectorRender( ItemKind::measurement_t & value )
    {
        if ( Selectable( language::measurement_pcs.data(), value == ItemKind::measurement_t::pcs ) )
        {
            value = ItemKind::measurement_t::pcs;
            return true;
        }
        if ( Selectable( language::measurement_g.data(), value == ItemKind::measurement_t::grams ) )
        {
            value = ItemKind::measurement_t::grams;
            return true;
        }
        if ( Selectable( language::measurement_custom.data(), value == ItemKind::measurement_t::custom ) )
        {
            value = ItemKind::measurement_t::custom;
            return true;
        }
        return false;
    }

    bool ItemKindCombo::Render()
    {
        bool ret = false;
        if ( BeginCombo( "##itemKindCombo", m_c[ selected ].name.c_str() ) )
        {
            for ( const auto & pair : m_c.GetItemKinds() )
            {
                if ( Selectable( pair.second.name.c_str(), false ) )
                {
                    selected = pair.first;
                    ret = true;
                }
            }
            EndCombo();
        }
        return ret;
    }
}
