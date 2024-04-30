
#pragma once

#include <string_view>

namespace language
{
    constexpr std::string_view bigWindowName{ "Przepi\xc5\x9b""nik" };
    constexpr std::string_view fridgeWindowName{ "Lod\xc3\xb3""wka" };
    constexpr std::string_view catalogWindowName{ "Katalog sk\xc5\x82""adnik\xc3\xb3""w" };
    constexpr std::string_view bookWindowName{ "Przepisy" };

    constexpr std::string_view b_edit{ "Edytuj" };
    constexpr std::string_view b_add{ "Dodaj" };
    constexpr std::string_view b_discard{ "Anuluj" };
    constexpr std::string_view b_save{ "Zapisz" };
    constexpr std::string_view b_saveCopy{ "Zapisz jako kopi\xc4\x99" };
    constexpr std::string_view b_removeRecipe{ "Usu\xc5\x84"" przepis" };

    constexpr std::string_view newRecipeName{ "Nowy przepis" };
    constexpr std::string_view selectIngredient{ "Wybierz sk\xc5\x82""adnik" };

    constexpr std::string_view measurement_pcs{ "sztuki" };
    constexpr std::string_view measurement_g{ "gramy" };
    constexpr std::string_view measurement_custom{ "inne" };

    constexpr std::string_view sort_id{ "sortowanie: po id" };
    constexpr std::string_view sort_name{ "sortowanie: po nazwie" };
    constexpr std::string_view sort_date{ "sortowanie: po dacie przydatno\xc5\x9b""ci" };
    constexpr std::string_view sort_dateGrouped{ "sortowanie: po dacie przydatno\xc5\x9b""ci (grupowane)" };
}

namespace otherConstants
{
    constexpr std::string_view mainFilename{ "Asiula.json" };
}
