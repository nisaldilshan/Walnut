add_library (walnut-graphics-opengl STATIC
                    ../../ImGui/ImGuiBuildOpenGL.cpp
                    OpenGLGraphics.cpp
                    GlfwOpenGLRenderingBackend.cpp
                    OpenGLImage.cpp   
                )

target_link_libraries(walnut-graphics-opengl PUBLIC glfw3 glad)