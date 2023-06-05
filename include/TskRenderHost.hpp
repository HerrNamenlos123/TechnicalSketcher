
#include "TskDocument.hpp"

class TskRenderHost {
public:
    TskRenderHost() = default;

    void render(const TskDocument& document, b::BaseWindow* window);

private:
    void renderGrid();

    class Perspective {
    public:
        Perspective() = default;

    private:
        ImVec2 m_position;
        float m_zoom = 1.0f;
    } m_perspective;
};