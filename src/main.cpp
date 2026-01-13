#include "System/Launcher.hpp"

int main(int, char**) {
    Launcher launcher({
        .WindowWidth = 1500u,
        .WindowHeight = 850u,
        .Fps = 60u,
        .WindowTitle = "Graph Visualiser",
        .WindowStyle = sf::Style::Default,
        .Fullscreen = false
    });

    if (!launcher.LoadResources("Resources/")) [[unlikely]] {
        return 1;
    }

    while (launcher.isRunning()) {
        launcher.HandleEvents();
        launcher.Update();
        launcher.Render();
    }

    return 0;
}