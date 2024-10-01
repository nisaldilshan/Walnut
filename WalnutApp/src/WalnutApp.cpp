#include <Walnut/Application.h>
#if defined(__ANDROID__) || defined(__iOS__)
#else
#include <Walnut/EntryPoint.h>
#endif
#include <Walnut/Random.h>
#include <Walnut/Timer.h>
#include <Walnut/RenderingBackend.h>
#include <Walnut/Image.h>

#include <imgui.h>

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
	}

	virtual void OnDetach() override
	{

	}

	virtual void OnUpdate(float ts) override
	{
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", m_lastRenderTime);

		static int sleepAmountMs = 10;
        auto sleepAmountChanged = ImGui::SliderInt("SleepTime (ms)", &sleepAmountMs, 0, 50);
		if (sleepAmountChanged)
		{
			Walnut::Application::Get().SetSleepAmount(std::chrono::milliseconds(sleepAmountMs));
		}
		ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
		m_viewportWidth = ImGui::GetContentRegionAvail().x;
        m_viewportHeight = ImGui::GetContentRegionAvail().y;
        if (m_image)
            ImGui::Image(m_image->GetDescriptorSet(), {(float)m_image->GetWidth(),(float)m_image->GetWidth()});
		ImGui::End();
        ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();
        Render();
	}

	void Render()
    {
        Walnut::Timer timer;

        if (!m_image ||
            m_viewportWidth != m_image->GetWidth() ||
            m_viewportHeight != m_image->GetHeight())
        {
			m_image.reset();
			m_image = std::make_shared<Walnut::Image>(m_viewportWidth, m_viewportHeight, Walnut::ImageFormat::RGBA);
            delete[] m_imageData;
            m_imageData = new uint32_t[m_viewportWidth * m_viewportHeight];
        }

        for (size_t i = 0; i < m_viewportWidth * m_viewportHeight; i++)
        {
            m_imageData[i] = Walnut::Random::UInt();
            m_imageData[i] |= 0xff000000; // remove randomnes from alpha channel
        }

        m_image->SetData(m_imageData);

        m_lastRenderTime = timer.ElapsedMillis();
        
    }
private:
    std::shared_ptr<Walnut::Image> m_image;
    uint32_t* m_imageData = nullptr;
    uint32_t m_viewportWidth = 0;
    uint32_t m_viewportHeight = 0;
    float m_lastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";
	// spec.Width = 320;
	// spec.Height = 480;

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		assert(ImGui::GetCurrentContext()); // This will fail in windows if walnut shared lib is used
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});

	return app;
}


#if defined(__ANDROID__) || defined(__iOS__)
int SDL_main(int argc, char *argv[])
{
    //Walnut::Application* app = Walnut::CreateApplication(argc, argv); // This is a function declared in Application.h
    //ImGuiIO& io = ImGui::GetIO();
    //io.FontGlobalScale = 2.2 * io.FontGlobalScale;
	static bool started = false;
	if (!started)
	{
		Walnut::Application* app = Walnut::CreateApplication(argc, argv);
		started = true;
		app->Run();
		delete app;
	}
    
    
    return 0;
}
#endif