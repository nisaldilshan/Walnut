add_library (walnut-imgui-part1 STATIC imgui-webgpu-part1.cpp)

add_library (walnut-graphics-webgpu STATIC
                    ImGuiBuildWGPU.cpp
                    WebGPUGraphics.cpp
                    GlfwWebGPURenderingBackend.cpp
                    WebGPUImage.cpp
                )

target_include_directories(walnut-graphics-webgpu PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src)                

if(MSVC)
#for Windows static libs are in _deps\dawn-build\src\dawn in build folder of WEBGPU
target_link_libraries(walnut-graphics-webgpu PUBLIC 
                                        walnut-imgui-part1 
                                        glfw3webgpu 
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/webgpu_dawn.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/dawn_native.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/dawn_common.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/dawn_utils.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/dawn_platform.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/tint.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/tint_utils_io.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/tint_val.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/tint_diagnostic_utils.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/absl_base.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/absl_strings.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/absl_strings_internal.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/absl_throw_delegate.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/absl_str_format_internal.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/absl_int128.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/absl_raw_logging_internal.lib
                                        ${CMAKE_SOURCE_DIR}/WalnutApp/SPIRV-Tools.lib
                        )

endif()

target_link_libraries(walnut-graphics-webgpu PUBLIC walnut-imgui-part1 glfw3webgpu webgpu_dawn)
