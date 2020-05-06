#include <imgui.h>
#include <llvmes-gui/application.h>
#include <llvmes-gui/draw.h>
#include <llvmes-gui/log.h>
#include <llvmes/common.h>

#include <iostream>

using namespace llvmes::gui;

class TestGUI : public Application {
    bool p_open = true;
    std::unique_ptr<Texture> texture;

   public:
    TestGUI() : Application(1200, 800, "Test UI")
    {
        texture = std::make_unique<Texture>(256, 240);
        auto data = std::vector<uint8_t>(256 * 240 * 4);
        int i, j;
        for (i = 0; i < 256; i++) {
            for (j = 0; j < 240; j++) {
                data[i * 240 * 4 + j * 4 + 0] = 255;
                data[i * 240 * 4 + j * 4 + 1] = 0;
                data[i * 240 * 4 + j * 4 + 2] = 0;
                data[i * 240 * 4 + j * 4 + 3] = 0;
            }
        }
        texture->SetData(data.data());
    }

    void OnImGui() override {}

    void OnEvent(Event& e) override
    {
        if (e.GetEventType() == EventType::KeyPressEvent) {
            auto& ev = (KeyPressEvent&)e;
            LLVMES_INFO("Key down: {}", ev.GetKeyCode());

            if (ev.GetKeyCode() == LLVMES_KEY_G) {
                LLVMES_TRACE("G key down!");
            }
        }
        else if (e.GetEventType() == EventType::MouseMoveEvent) {
            auto& ev = (MouseMoveEvent&)e;
            auto pair = ev.GetPosition();
            LLVMES_TRACE("Mouse position: {} {}", pair.first,
                         Application::GetWindow().GetHeight() - pair.second);
        }
    }

    void OnUpdate() override
    {
        Draw::Begin();
        Draw::UseColor(glm::vec4(1, 0, 0, 0));
        Draw::Rectangle(0, 0, 100, 100);
        Draw::UseColor(glm::vec4(1, 1, 0, 0));
        Draw::Rectangle(200, 200, 100, 100);
        Draw::End();
    }
};

int main()
try {
    TestGUI ui;
    ui.Run();
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
}