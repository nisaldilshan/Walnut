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
	ExampleLayer(Walnut::Application* app) 
		: appPtr(app)
	{
	}

	virtual void OnAttach() override
	{}

	virtual void OnDetach() override
	{}

	virtual void OnUpdate(float ts) override
	{
		const auto& mainImage = appPtr->MainImageRef();
		if (mainImage) {
			MainImageRender();
		}
	}

	virtual void OnUIRender() override // This will be called only if UseImGui is set to true
	{
		const auto& mainImage = appPtr->MainImageRef();
		assert(!mainImage);
		ImGuiRender();
	}

	void MainImageRender()
    {
        Walnut::Timer timer;

		static uint32_t windowWidth = 0;
		static uint32_t windowHeight = 0;

		auto& mainImage = appPtr->MainImageRef();
        if (windowWidth != mainImage->GetWidth() ||
            windowHeight != mainImage->GetHeight())
        {
			windowWidth = mainImage->GetWidth();
			windowHeight = mainImage->GetHeight();
            delete[] m_imageData;
            m_imageData = new uint32_t[windowWidth * windowHeight];
        }

        for (size_t i = 0; i < windowWidth * windowHeight; i++)
        {
            m_imageData[i] = Walnut::Random::UInt();
            m_imageData[i] |= 0xff000000; // remove randomnes from alpha channel
        }
		mainImage->SetData(m_imageData);

        m_lastRenderTime = timer.ElapsedMillis();
    }

	void ImGuiRender()
	{
		Walnut::Timer timer;

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
		const uint32_t viewportWidth = ImGui::GetContentRegionAvail().x;
        const uint32_t viewportHeight = ImGui::GetContentRegionAvail().y;
        if (m_image) {
            ImGui::Image(m_image->GetHandle(), {(float)m_image->GetWidth(),(float)m_image->GetWidth()});
		}
		ImGui::End();
        ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();

		if (!m_image ||
            viewportWidth != m_image->GetWidth() ||
            viewportHeight != m_image->GetHeight())
        {
			m_image.reset();
			m_image = std::make_shared<Walnut::Image>(viewportWidth, viewportHeight, Walnut::ImageFormat::RGBA);
            delete[] m_imageData;
            m_imageData = new uint32_t[viewportWidth * viewportHeight];
        }

        for (size_t i = 0; i < viewportWidth * viewportHeight; i++)
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
    float m_lastRenderTime = 0.0f;
	Walnut::Application* appPtr = nullptr;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";
	spec.Width = 1280;
	spec.Height = 720;
	spec.UseImGui = false;

	Walnut::Application* app = new Walnut::Application(spec);
	auto exampleLayer = std::make_shared<ExampleLayer>(app);
	app->PushLayer(exampleLayer);
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
extern "C" int SDL_main(int argc, char *argv[])
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