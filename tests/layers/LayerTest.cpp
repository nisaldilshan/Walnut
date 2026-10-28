// Pushes and pops layers while the application runs and checks the order of OnAttach, OnDetach and the
// layers' destructors:
//  - a layer popped and its replacement pushed in the same frame: the old one is detached and destroyed
//    before the new one is attached
//  - the same from OnUIRender, popping the top layer
//  - a layer popped before it was ever attached
//  - at shutdown, layers detach top first
//
// Exit code: 0 when the order is the expected one, 1 otherwise.

#include <Walnut/Application.h>

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace {

using EventLog = std::vector<std::string>;

class RecordingLayer : public Walnut::Layer
{
public:
	RecordingLayer(std::string name, EventLog& log) : m_Name(std::move(name)), m_Log(log) {}
	~RecordingLayer() override { m_Log.push_back("destroy " + m_Name); }

	void OnAttach() override { m_Log.push_back("attach " + m_Name); }
	void OnDetach() override { m_Log.push_back("detach " + m_Name); }

private:
	std::string m_Name;
	EventLog& m_Log;
};

// The bottom layer, it changes the layers above it
class ControllerLayer : public Walnut::Layer
{
public:
	ControllerLayer(Walnut::Application& app, EventLog& log) : m_App(app), m_Log(log) {}

	std::shared_ptr<Walnut::Layer> A;

	void OnUpdate(float) override
	{
		m_Frame++;
		if (m_Frame == 1)
		{
			// Replace A by C
			m_Log.push_back("frame 1");
			m_App.PopLayer(A);
			A.reset();
			m_App.PushLayer<RecordingLayer>("C", m_Log);
		}
		else if (m_Frame == 4)
		{
			// E is popped before it was attached
			m_Log.push_back("frame 4");
			auto e = m_App.PushLayer<RecordingLayer>("E", m_Log);
			m_App.PopLayer(e);
		}
		else if (m_Frame == 6)
		{
			m_Log.push_back("frame 6");
			m_App.Close();
		}
	}

	void OnUIRender() override
	{
		if (m_Frame == 2)
		{
			// Replace the top layer, C, by D
			m_Log.push_back("frame 2");
			m_App.PopLayer();
			m_App.PushLayer<RecordingLayer>("D", m_Log);
		}
	}

private:
	Walnut::Application& m_App;
	EventLog& m_Log;
	int m_Frame = 0;
};

} // namespace

int main(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Layer Test";
	spec.Width = 400;
	spec.Height = 300;
	spec.EnableGraphicsValidation = true; // Vulkan: CTest fails the test on any validation message

	EventLog log;
	{
		Walnut::Application app(spec);
		auto controller = app.PushLayer<ControllerLayer>(app, log);
		controller->A = app.PushLayer<RecordingLayer>("A", log);
		app.PushLayer<RecordingLayer>("B", log);
		app.Run();
	}

	const EventLog expected = {
		"attach A", "attach B",
		"frame 1",
		"detach A", "destroy A", "attach C",
		"frame 2",
		"detach C", "destroy C", "attach D",
		"frame 4",
		"attach E", "detach E", "destroy E",
		"frame 6",
		"detach D", "destroy D", "detach B", "destroy B",
	};

	bool pass = log == expected;
	if (!pass)
	{
		std::printf("expected:\n");
		for (const auto& event : expected)
			std::printf("  %s\n", event.c_str());
		std::printf("got:\n");
		for (const auto& event : log)
			std::printf("  %s\n", event.c_str());
	}
	std::printf("%s\n", pass ? "PASS" : "FAILED");
	return pass ? 0 : 1;
}
