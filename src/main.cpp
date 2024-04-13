
#include "app.hpp"
#include "book/book.hpp"
#include "fridge/fridge.hpp"
#include "textConstants.hpp"

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
    std::ifstream loadFile( otherConstants::mainFilename );
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

        ImGui::SetNextWindowSize( { 300,300 }, ImGuiCond_FirstUseEver );
        ImGui::Begin( language::fridgeWindowName.data() );
        edigarian.Render();
        ImGui::End();

        ImGui::SetNextWindowSize( { 300,300 }, ImGuiCond_FirstUseEver );
        ImGui::Begin( language::catalogWindowName.data() );
        avon.Render();
        ImGui::End();

        ImGui::SetNextWindowSize( { 300,300 }, ImGuiCond_FirstUseEver );
        ImGui::Begin( language::bookWindowName.data() );
        newBraveWorld.Render( avon );
        ImGui::End();

        running &= app->EndFrame();
    }
    std::ofstream saveFile( otherConstants::mainFilename );
    nlohmann::json j;
    avon.Save( j );
    edigarian.Save( j );
    newBraveWorld.Save( j );
    saveFile << j.dump( 4 );
    saveFile.close();
    app.reset();
    return 0;
}
