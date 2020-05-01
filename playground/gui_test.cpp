#include <imgui.h>
#include <llvmes-gui/application.h>
#include <llvmes-gui/log.h>

#include <iostream>

using namespace llvmes;

class Debugger : public gui::Application {
    bool p_open = true;

   public:
    Debugger() : Application(1200, 800, "Test UI") {}

    void OnImGui() override
    {
        ImGui::ShowDemoWindow();
    }

    void OnEvent(gui::Event& e) override
    {
        if (e.GetEventType() == gui::EventType::KeyPressEvent) {
            auto& ev = (gui::KeyPressEvent&)e;
            LLVMES_INFO("Key down: {}", ev.GetKeyCode());

            if (ev.GetKeyCode() == LLVMES_KEY_G) {
                LLVMES_TRACE("G key down!");
            }
        }
        else if (e.GetEventType() == gui::EventType::MouseMoveEvent) {
            auto& ev = (gui::MouseMoveEvent&)e;
            auto pair = ev.GetPosition();
            LLVMES_TRACE("Mouse position: {} {}", pair.first, pair.second);
        }
    }

    void OnUpdate() override {}
};

int main()
try {
    Debugger ui;
    ui.Run();
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
}