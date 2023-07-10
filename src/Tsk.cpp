
#include "Tsk.hpp"

void Tsk::onSetup() {
    attachWindow(splashWindow);
}

void Tsk::onUpdate() {
    switch (context.windowMode) {
        case TskWindowMode::Splash:
            if (b::time() > 0.3) {
                context.windowMode = TskWindowMode::Main;
                attachWindow(mainWindow);
                detachWindow(splashWindow);
            }
            break;

        case TskWindowMode::Main:
            if (!mainWindow.isOpen()) {
                stopApplication();
            }
            break;

        default:
            break;
    }
}

void Tsk::onRender() {

}

void Tsk::onExit() {

}