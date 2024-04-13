
#include "app.hpp"
#include "book/book.hpp"
#include "fridge/fridge.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <fstream>

#define SDL_MAIN_HANDLED

int WinMain( int, char ** )
{
    auto app = AppI::Create();

    Catalog avon;
    Fridge edigarian( avon );
    Book newBraveWorld;
    std::ifstream loadFile( "fridge.json" );
    try
    {
        nlohmann::json j;
        loadFile >> j;
        avon.Load( j );
        edigarian.Load( j );
        newBraveWorld.Load( j );
    }
    catch ( ... )
    {
    }

    loadFile.close();
    bool running = true;
    while ( running )
    {
        running &= app->EventLoop();
        running &= app->BeginFrame();

        ImGui::Begin( "Lodówka" );
        edigarian.Render();
        ImGui::End();

        ImGui::Begin( "Katalog" );
        avon.Render();
        ImGui::End();

        ImGui::Begin( "Przepisy" );
        newBraveWorld.Render( avon );
        ImGui::End();

        running &= app->EndFrame();
    }
    std::ofstream saveFile( "fridge.json" );
    nlohmann::json j;
    avon.Save( j );
    edigarian.Save( j );
    newBraveWorld.Save( j );
    saveFile << j.dump( 4 );
    saveFile.close();
    app.reset();
    return 0;
}
