
#pragma once
// notice: this file should be encoded as utf-8 without signature

#include <string_view>

namespace language
{
    constexpr std::string_view bigWindowName{ "Przepiśnik" };
    constexpr std::string_view fridgeWindowName{ "Lodówka" };
    constexpr std::string_view catalogWindowName{ "Katalog składników" };
    constexpr std::string_view bookWindowName{ "Przepisy" };

    constexpr std::string_view b_edit{ "Edytuj" };
    constexpr std::string_view b_add{ "Dodaj" };
    constexpr std::string_view b_discard{ "Anuluj" };
    constexpr std::string_view b_save{ "Zapisz" };
    constexpr std::string_view b_saveCopy{ "Zapisz jako kopię" };

    constexpr std::string_view newRecipeName{ "Nowy przepis" };
    constexpr std::string_view selectIngredient{ "Wybierz składnik" };

    constexpr std::string_view measurement_pcs{ "sztuki" };
    constexpr std::string_view measurement_g{ "gramy" };
    constexpr std::string_view measurement_custom{ "inne" };

    constexpr std::string_view sort_id{ "sortowanie: po id" };
    constexpr std::string_view sort_name{ "sortowanie: po nazwie" };
    constexpr std::string_view sort_date{ "sortowanie: po dacie przydatności" };
    constexpr std::string_view sort_dateGrouped{ "sortowanie: po dacie przydatności (grupowane)" };
}

namespace otherConstants
{
    constexpr std::string_view mainFilename{ "Asiula.json" };
}
