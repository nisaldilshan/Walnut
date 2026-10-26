// Renders a known scene through Walnut, reads the frame back from the main window and checks its pixels:
//  - a Walnut::Image with four colored quadrants, drawn with ImGui::Image
//  - the same image after Image::SetData gave it four other colors
//  - a rectangle filled by ImGui itself
// The checks only sample solid areas, so they do not depend on fonts, the windowing system or the GPU.
// Every captured frame is also saved as render-test-<renderer>-<windowing>-<phase>.png.
//
// Exit code: 0 when all checks pass, 1 when one fails, 77 (skipped) when the renderer cannot read frames back.

#include <Walnut/Application.h>
#include <Walnut/Image.h>
#include <Walnut/RenderingBackend.h>

#include <imgui.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr int kExitPass = 0;
constexpr int kExitFail = 1;
constexpr int kExitSkip = 77;

struct Color { uint8_t r, g, b; };

constexpr Color kRed{ 255, 0, 0 }, kGreen{ 0, 255, 0 }, kBlue{ 0, 0, 255 }, kWhite{ 255, 255, 255 };
constexpr Color kYellow{ 255, 255, 0 }, kCyan{ 0, 255, 255 }, kMagenta{ 255, 0, 255 };

// Quadrant colors: top left, top right, bottom left, bottom right. Only 0 and 255 channels, which
// sRGB encoding and texture filtering leave unchanged.
using Quadrants = std::array<Color, 4>;
constexpr Quadrants kFirstImage{ kRed, kGreen, kBlue, kWhite };
constexpr Quadrants kSecondImage{ kYellow, kCyan, kMagenta, kRed };
constexpr Color kRectColor = kMagenta;

constexpr uint32_t kImageSize = 64;          // texels
constexpr float kImageDrawSize = 256.0f;     // ImGui units
constexpr float kRectSize = 64.0f;           // ImGui units
constexpr int kTolerance = 8;                // per channel

// Frames to wait before each capture, so windows have appeared and textures are uploaded.
constexpr int kFirstCaptureFrame = 5;
constexpr int kSecondImageFrame = 8;
constexpr int kSecondCaptureFrame = 12;
constexpr int kTimeoutFrame = 120;

const char* RendererName()
{
	switch (Walnut::RenderingBackend::GetBackend())
	{
		case Walnut::RenderingBackend::BACKEND::OpenGL: return "opengl";
		case Walnut::RenderingBackend::BACKEND::Vulkan: return "vulkan";
		case Walnut::RenderingBackend::BACKEND::WebGPU: return "webgpu";
		default: return "unknown";
	}
}

const char* WindowingName()
{
#ifdef USE_SDL
	return "sdl";
#else
	return "glfw";
#endif
}

std::vector<uint32_t> MakeImageData(const Quadrants& colors)
{
	std::vector<uint32_t> data(kImageSize * kImageSize);
	for (uint32_t y = 0; y < kImageSize; y++)
	{
		for (uint32_t x = 0; x < kImageSize; x++)
		{
			const Color& c = colors[(y < kImageSize / 2 ? 0 : 2) + (x < kImageSize / 2 ? 0 : 1)];
			data[y * kImageSize + x] = 0xff000000u | (uint32_t(c.b) << 16) | (uint32_t(c.g) << 8) | c.r; // RGBA in memory
		}
	}
	return data;
}

// A rectangle in framebuffer pixels
struct PixelRect { float x0, y0, x1, y1; };

class RenderTestLayer : public Walnut::Layer
{
public:
	explicit RenderTestLayer(Walnut::Application& app) : m_App(app) {}

	int ExitCode() const { return m_ExitCode; }

	void OnAttach() override
	{
		ImGui::GetIO().IniFilename = nullptr; // a saved layout from an earlier run must not change the scene
		m_Image = std::make_shared<Walnut::Image>(kImageSize, kImageSize, Walnut::ImageFormat::RGBA);
		m_Image->SetData(MakeImageData(kFirstImage).data());
	}

	void OnDetach() override
	{
		m_Image.reset(); // before the renderer shuts down
	}

	void OnUpdate(float) override
	{
		if (m_Frame == kSecondImageFrame)
			m_Image->SetData(MakeImageData(kSecondImage).data());
	}

	void OnUIRender() override
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::Begin("Render Test", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings);

		ImGui::Text("Walnut render test (%s, %s)", RendererName(), WindowingName());
		ImGui::Image(m_Image->GetHandle(), ImVec2(kImageDrawSize, kImageDrawSize));
		const PixelRect image = ToPixels(viewport, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

		const ImVec2 rectMin(ImGui::GetItemRectMax().x + 16.0f, ImGui::GetItemRectMin().y);
		const ImVec2 rectMax(rectMin.x + kRectSize, rectMin.y + kRectSize);
		ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, IM_COL32(kRectColor.r, kRectColor.g, kRectColor.b, 255));
		const PixelRect rect = ToPixels(viewport, rectMin, rectMax);

		ImGui::End();

		if (m_Frame == kFirstCaptureFrame)
			RequestCapture("first-image", kFirstImage, image, rect);
		else if (m_Frame == kSecondCaptureFrame)
			RequestCapture("second-image", kSecondImage, image, rect);
		else if (m_Captures == 2)
			Finish(m_ExitCode);
		else if (m_Frame >= kTimeoutFrame)
		{
			std::fprintf(stderr, "FAIL: only %d of 2 frames were captured within %d frames\n", m_Captures, kTimeoutFrame);
			Finish(kExitFail);
		}
		m_Frame++;
	}

private:
	// ImGui positions are relative to the desktop when viewports are enabled, and in points rather than pixels.
	static PixelRect ToPixels(const ImGuiViewport* viewport, ImVec2 min, ImVec2 max)
	{
		const ImVec2 scale = ImGui::GetIO().DisplayFramebufferScale;
		return { (min.x - viewport->Pos.x) * scale.x, (min.y - viewport->Pos.y) * scale.y,
				 (max.x - viewport->Pos.x) * scale.x, (max.y - viewport->Pos.y) * scale.y };
	}

	void RequestCapture(const std::string& phase, const Quadrants& expected, PixelRect image, PixelRect rect)
	{
		m_App.CaptureNextFrame([this, phase, expected, image, rect](const uint8_t* pixels, uint32_t width, uint32_t height)
		{
			m_Captures++;
			if (!pixels)
			{
				std::fprintf(stderr, "SKIP: the %s renderer cannot read frames back\n", RendererName());
				Finish(kExitSkip);
				return;
			}

			const std::string file = std::string("render-test-") + RendererName() + "-" + WindowingName() + "-" + phase + ".png";
			stbi_write_png(file.c_str(), int(width), int(height), 4, pixels, int(width) * 4);
			std::printf("%s: captured %ux%u frame, saved as %s\n", phase.c_str(), width, height, file.c_str());

			Frame frame{ pixels, width, height };
			bool ok = true;
			const float halfW = (image.x1 - image.x0) / 2.0f, halfH = (image.y1 - image.y0) / 2.0f;
			const char* quadrantNames[] = { "top left", "top right", "bottom left", "bottom right" };
			for (int q = 0; q < 4; q++)
			{
				const float x0 = image.x0 + (q % 2) * halfW, y0 = image.y0 + (q / 2) * halfH;
				ok &= CheckArea(frame, phase + " image " + quadrantNames[q], { x0, y0, x0 + halfW, y0 + halfH }, expected[q]);
			}
			ok &= CheckArea(frame, phase + " ImGui rectangle", rect, kRectColor);
			if (!ok)
				Finish(kExitFail);
		});
	}

	struct Frame { const uint8_t* pixels; uint32_t width, height; };

	// Samples a 3x3 grid inside the middle half of area, away from edges where filtering blends colors.
	static bool CheckArea(const Frame& frame, const std::string& name, PixelRect area, Color expected)
	{
		bool ok = true;
		for (int j = 1; j <= 3; j++)
		{
			for (int i = 1; i <= 3; i++)
			{
				const float fx = 0.25f + 0.125f * i, fy = 0.25f + 0.125f * j;
				const int x = int(area.x0 + fx * (area.x1 - area.x0));
				const int y = int(area.y0 + fy * (area.y1 - area.y0));
				if (x < 0 || y < 0 || uint32_t(x) >= frame.width || uint32_t(y) >= frame.height)
				{
					std::fprintf(stderr, "FAIL: %s: pixel (%d, %d) is outside the %ux%u frame\n", name.c_str(), x, y, frame.width, frame.height);
					return false;
				}
				const uint8_t* p = frame.pixels + (size_t(y) * frame.width + x) * 4;
				if (std::abs(p[0] - expected.r) > kTolerance || std::abs(p[1] - expected.g) > kTolerance || std::abs(p[2] - expected.b) > kTolerance)
				{
					std::fprintf(stderr, "FAIL: %s: pixel (%d, %d) is (%d, %d, %d), expected (%d, %d, %d)\n", name.c_str(), x, y,
								 p[0], p[1], p[2], expected.r, expected.g, expected.b);
					ok = false;
				}
			}
		}
		if (ok)
			std::printf("ok: %s\n", name.c_str());
		return ok;
	}

	void Finish(int exitCode)
	{
		m_ExitCode = exitCode;
		m_App.Close();
	}

	Walnut::Application& m_App;
	std::shared_ptr<Walnut::Image> m_Image;
	int m_Frame = 0;
	int m_Captures = 0;
	int m_ExitCode = kExitPass;
};

} // namespace

int main(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Render Test";
	spec.Width = 800;
	spec.Height = 600;
	spec.EnableGraphicsValidation = true; // Vulkan: CTest fails the test on any validation message

	auto app = std::make_unique<Walnut::Application>(spec);
	auto layer = std::make_shared<RenderTestLayer>(*app);
	app->PushLayer(layer);
	app->Run();
	app.reset();

	const int exitCode = layer->ExitCode();
	std::printf("%s\n", exitCode == kExitPass ? "PASS" : exitCode == kExitSkip ? "SKIPPED" : "FAILED");
	return exitCode;
}
