#include <SGE/defines.hpp>

#include "../../common.hpp"
#include "app.hpp"

int main(int argc, char** argv) {
    ExampleConfig config;

    if (!ParseCommandLineArguments(argc, argv, config)) {
        return 1;
    }

    App app(config);
    if (app.Init()) {
        app.Run();
    }

    return 0;
}