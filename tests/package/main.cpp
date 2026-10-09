#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>

#include <imgui.h>

class HelloLayer : public Walnut::Layer
{
public:
    void OnUIRender() override
    {
        ImGui::Begin("Hello");
        ImGui::Text("Hello from Walnut");
        ImGui::End();
    }
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "Walnut Package Test";

    auto* app = new Walnut::Application(spec);
    app->PushLayer<HelloLayer>();
    return app;
}
