#include "SandboxLayer.h"


SandboxLayer::SandboxLayer()
{
    m_VertexArray = Hazel::VertexArray::Create();

    float triangleVertexPositions[3 * 3] = {
        -0.5f, -0.5f, 0.0f, 
            0.5f, -0.5f, 0.0f, 
            0.0f,  0.5f, 0.0f 
    };
    float triangleVertexColors[3 * 4] = {
        0.8f, 0.2f, 0.8f, 1.0f,
        0.2f, 0.3f, 0.8f, 1.0f,
        0.8f, 0.8f, 0.2f, 1.0f
    };

    {
        std::shared_ptr<Hazel::VertexBuffer> vertexBuffer = Hazel::VertexBuffer::Create(triangleVertexPositions, sizeof(triangleVertexPositions));
        Hazel::BufferLayout layout = {
            { Hazel::ShaderDataType::Float3, "a_Position" }
        };
        vertexBuffer->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vertexBuffer);
    }
    {
        std::shared_ptr<Hazel::VertexBuffer> vertexBuffer = Hazel::VertexBuffer::Create(triangleVertexColors, sizeof(triangleVertexColors));
        Hazel::BufferLayout layout = {
            { Hazel::ShaderDataType::Float4, "a_Color" }
        };
        vertexBuffer->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vertexBuffer);
    }

    uint32_t indices[3] = { 0, 1, 2 };
    std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
    indexBuffer = Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
    m_VertexArray->SetIndexBuffer(indexBuffer);

    m_SquareVA = Hazel::VertexArray::Create();

    float squareVertices[5 * 4] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };

    std::shared_ptr<Hazel::VertexBuffer> squareVB = Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
    squareVB->SetLayout({
        { Hazel::ShaderDataType::Float3, "a_Position" },
        { Hazel::ShaderDataType::Float2, "a_TexCoord" }
    });
    m_SquareVA->AddVertexBuffer(squareVB);

    uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
    std::shared_ptr<Hazel::IndexBuffer> squareIB;
    squareIB = Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    m_SquareVA->SetIndexBuffer(squareIB);

    std::string vertexSrc = R"(
        #version 330 core
        
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;

        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;

        out vec4 v_Color;

        void main()
        {
            v_Color = a_Color;
            gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
        }
    )";

    std::string fragmentSrc = R"(
        #version 330 core
        
        layout(location = 0) out vec4 color;
        in vec4 v_Color;
        void main()
        {
            color = v_Color;
        }
    )";

    m_Shader = Hazel::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

    std::string flatColorShaderVertexSrc = R"(
        #version 330 core
        
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;

        void main()
        {
            gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
        }
    )";

    std::string flatColorShaderFragmentSrc = R"(
        #version 330 core
        
        layout(location = 0) out vec4 color;
        
        uniform vec3 u_Color;
        void main()
        {
            color = vec4(u_Color, 1.0);
        }
    )";

    m_FlatColorShader = Hazel::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

    auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

    m_Texture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
    m_ChernoLogoTexture = Hazel::Texture2D::Create("assets/textures/ChernoLogo.png");

    std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->Bind();
    std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
}

void SandboxLayer::OnUpdate(Hazel::Timestep ts)
{
    //m_CameraController.OnUpdate(ts);
    
    Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
    Hazel::RenderCommand::Clear();

    //Hazel::Renderer::BeginScene(m_CameraController.GetCamera());
    
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

    std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader)->Bind();
    std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 20; x++)
        {
            glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
            Hazel::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
        }
    }

    auto textureShader = m_ShaderLibrary.Get("Texture");

    m_Texture->Bind();
    Hazel::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
    m_ChernoLogoTexture->Bind();
    Hazel::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

    //Triangle
    Hazel::Renderer::Submit(m_Shader, m_VertexArray);

    Hazel::Renderer::EndScene();

    if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB))
        HZ_TRACE("Tab key is pressed (poll)!");
}

void SandboxLayer::OnImGuiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
    ImGui::End();
}

// void SandboxLayer::OnEvent(Hazel::Event& e)
// {
//     if (e.GetEventType() == Hazel::EventType::KeyPressed)
//     {
//         Hazel::KeyPressedEvent& ev = (Hazel::KeyPressedEvent&)e;
//         if (ev.GetKeyCode() == HZ_KEY_TAB)
//             HZ_TRACE("Tab key is pressed (event)!");
//         HZ_TRACE("{0}", (char)ev.GetKeyCode());
//     }

//     m_CameraController.OnEvent(e);
// }