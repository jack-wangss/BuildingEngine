#include "pch.h"

#include <Runtime/Engine.h>
#include <Editor/Editor.h>

int main(int argc, char** argv)
{
    std::filesystem::path executablePath(argv[0]);
    std::filesystem::path configFilePath = executablePath.parent_path() / "BuildingEngine.ini";

    BE::Engine* engine = new BE::Engine();
    engine->StartEngine(configFilePath.generic_string());
    std::cout << "hello engine" << std::endl;

    BE::Editor* editor = new BE::Editor();
    editor->Initialize(engine);
    editor->Run();
    editor->Clear();

    engine->ShutdownEngine();
    return 0;
}