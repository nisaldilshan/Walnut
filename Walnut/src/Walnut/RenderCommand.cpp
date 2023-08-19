#include "RenderCommand.h"

namespace Walnut {

	std::unique_ptr<RenderingBackend> RenderCommand::s_RendererAPI = RenderingBackend::Create();

} 