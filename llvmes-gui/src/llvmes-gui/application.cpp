#include "llvmes-gui/application.h"

#include <iostream>

#include "llvmes-gui/imgui/imgui_layer.h"
#include "llvmes-gui/log.h"

namespace llvmes {
namespace gui {

Application* Application::s_application = nullptr;

Application::Application(int width, int height, const std::string& program_name)
    : m_running(false)
{
    LLVMES_ASSERT(s_application == nullptr, "An Application already exists");

    // Attempts to create a window, can fail and expects a try-catch block
    // on some level of the program
    m_window = std::unique_ptr<Window>(new Window(width, height, program_name));

    // Init the log library
    Log::Init();

    // The window was successfully created, proceed to init the rest
    //
    // Set the global static application pointer
    s_application = this;

    // Set the event handler to fill the event queue in this application object
    m_window->event_handler = [this](Event* e) { m_event_queue.push(e); };

    // Initialize ImGui and set styles etc
    ImGuiLayer::Create();
    m_running = true;
}

Application::~Application()
{
    while (!m_event_queue.empty()) {
        delete m_event_queue.front();
        m_event_queue.pop();
    }
    s_application = nullptr;
    ImGuiLayer::Destroy();
}

void Application::OnImGui()
{
}

void Application::OnUpdate()
{
}

void Application::OnEvent(Event& e)
{
}

void Application::Run()
{
    while (m_running) {
        m_window->Clear();

        while (!m_event_queue.empty()) {
            // Fetch next event
            Event& e = *m_event_queue.front();

            // Process events
            if (e.GetEventType() == EventType::WindowCloseEvent)
                m_running = false;
            OnEvent(e);
            ImGuiLayer::ProcessEvents(e);

            // Delete it
            m_event_queue.pop();
        }

        OnUpdate();

        ImGuiLayer::Begin();
        OnImGui();
        ImGuiLayer::End();

        m_window->Update();
    }
}

void Application::Terminate()
{
    m_running = false;
}

Application& Application::Get()
{
    LLVMES_ASSERT(s_application != nullptr, "An application does not exist");
    return *s_application;
}

}  // namespace gui
}  // namespace llvmes