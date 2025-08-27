#include <SGE/defines.hpp>

#include "../../common.hpp"
#include "app.hpp"

int main(int argc, char** argv) {
    ExampleConfig config;

    if (!ParseCommandLineArguments(argc, argv, config)) {
        return 1;
    }

    if (App::Init(config)) {
        App::Run();
    }
    App::Destroy();

    return 0;
}