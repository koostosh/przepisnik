
#pragma once

namespace Widgets
{
    bool SButton( const char * label );
    // type based selector for explicit instatiation
    template<typename T>
    bool SelectorRender( T & value );
}