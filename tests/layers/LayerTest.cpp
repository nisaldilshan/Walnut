// Pushes and pops layers while the application runs and checks the order of OnAttach, OnDetach and the
// layers' destructors:
//  - a layer pushed before Run() is attached at once
//  - a layer popped and its replacement pushed in the same frame: the old one is detached and destroyed
//    before the new one is attached
//  - the same from OnUIRender, popping the top layer
//  - popping a null layer does nothing
//  - a layer pushed and popped in the same frame is dropped, it is never attached or detached
//  - a layer pushed before PopLayer() in the same frame is not the one that is popped
//  - at shutdown, layers detach bottom first
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
		else if (m_Frame == 3)
		{
			// Must not pop D
			m_Log.push_back("frame 3");
			m_App.PopLayer(std::shared_ptr<Walnut::Layer>());
		}
		else if (m_Frame == 4)
		{
			// E is popped before it is attached, so it is dropped and destroyed without OnAttach or OnDetach
			m_Log.push_back("frame 4");
			auto e = m_App.PushLayer<RecordingLayer>("E", m_Log);
			m_App.PopLayer(e);
		}
		else if (m_Frame == 5)
		{
			// PopLayer() removes D, the top attached layer, not F
			m_Log.push_back("frame 5");
			m_App.PushLayer<RecordingLayer>("F", m_Log);
			m_App.PopLayer();
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
	bool attachedBeforeRun = false;
	{
		Walnut::Application app(spec);
		auto controller = app.PushLayer<ControllerLayer>(app, log);
		controller->A = app.PushLayer<RecordingLayer>("A", log);
		app.PushLayer<RecordingLayer>("B", log);
		attachedBeforeRun = log == EventLog{ "attach A", "attach B" };
		app.Run();
	}

	const EventLog expected = {
		"attach A", "attach B",
		"frame 1",
		"detach A", "destroy A", "attach C",
		"frame 2",
		"detach C", "destroy C", "attach D",
		"frame 3",
		"frame 4", "destroy E",
		"frame 5",
		"detach D", "destroy D", "attach F",
		"frame 6",
		"detach B", "detach F", "destroy B", "destroy F",
	};

	bool pass = attachedBeforeRun && log == expected;
	if (!pass)
	{
		if (!attachedBeforeRun)
			std::printf("layers pushed before Run() were not attached at once\n");
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
