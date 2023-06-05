
#include "Tsk.hpp"

void Tsk::onSetup() {
    attachWindow(&s_splashWindow);
}

void Tsk::onUpdate() {
    switch (m_context.m_windowMode) {
        case TskWindowMode::Splash:
            if (b::time() > 4 && s_splashWindow) {
                m_context.m_windowMode = TskWindowMode::Main;
                attachWindow(&s_mainWindow);
                detachWindow(&s_splashWindow);
            }
            break;

        case TskWindowMode::Main:
            if (!s_mainWindow->isOpen()) {
                stopApplication();
            }
            break;

        default:
            break;
    }
}

void Tsk::onCleanup() {

}