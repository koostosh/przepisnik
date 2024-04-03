
#include "widgets.hpp"

#include <imgui.h>

namespace Widgets
{
    using namespace ImGui;
    constexpr ImVec2 c_btSize{ 20, 20 };
    bool SButton( const char * label )
    {
        return Button( label, c_btSize );
    }
}
