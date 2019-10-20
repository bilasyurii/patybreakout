#include "Paty.h"

int main() {
    srand(time(NULL));
    Storage::loadConfiguration("res/config/user.cnf");

    Storage::loadResources("res/config/battle.res",
                           Storage::texturesGUI, Storage::spritesGUI, Storage::fontsGUI);
    MasterRenderer::init();
    StateMachine::prepare();

    while(MasterRenderer::window.isOpen())
        StateMachine::work();

    return 0;
}
