if(CMAKE_SYSTEM_NAME MATCHES "Android")
add_library(App SHARED src/WalnutApp.cpp)
# SDL's Java activity loads the app from libmain.so
set_target_properties(App PROPERTIES OUTPUT_NAME main)
else()
add_executable(App src/WalnutApp.cpp )
endif()

#### iOS APP BUNDLE ###
# On iOS the app is the bundle App.app in the build folder: the executable and an Info.plist generated from
# iOS/Info.plist.in. main() comes from SDL_main.h, which src/WalnutApp.cpp includes on iOS.
if(IOS)
    set(WALNUT_IOS_BUNDLE_ID "io.github.nisaldilshan.walnut" CACHE STRING "Bundle identifier of the example app on iOS")
    set(WALNUT_IOS_EXECUTABLE App)
    # The deployment target, from os.version in the Conan profile
    set(WALNUT_IOS_MINIMUM_OS_VERSION "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    if(NOT WALNUT_IOS_MINIMUM_OS_VERSION)
        set(WALNUT_IOS_MINIMUM_OS_VERSION 15.0)
    endif()
    # The SDK, from os.sdk in the Conan profile
    if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
        set(WALNUT_IOS_PLATFORM iPhoneSimulator)
    else()
        set(WALNUT_IOS_PLATFORM iPhoneOS)
    endif()
    # What the app needs from the device, for the renderer it is built with
    if(RENDERER STREQUAL "OpenGL")
        set(WALNUT_IOS_RENDERER_CAPABILITY opengles-3)
    else()
        set(WALNUT_IOS_RENDERER_CAPABILITY metal) # MoltenVK and Dawn run on Metal
    endif()
    configure_file(iOS/Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
    set_target_properties(App PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${WALNUT_IOS_BUNDLE_ID}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
    if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
        # The simulator runs apps with an ad-hoc signature. A device needs a signature from an Apple development
        # certificate and a provisioning profile, which this does not set up. Xcode signs the bundle itself.
        add_custom_command(TARGET App POST_BUILD
            COMMAND codesign --force --sign - --timestamp=none "$<TARGET_BUNDLE_DIR:App>"
            COMMENT "Signing App.app ad hoc"
            VERBATIM)
    endif()
endif()

# walnut brings in its include paths and imgui.
target_link_libraries(App PRIVATE walnut)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_link_libraries(App PRIVATE SDL3::SDL3) # SDL2::SDL2main
else()
    target_link_libraries(App PUBLIC glfw)
endif()

if (RENDERER STREQUAL "WebGPU")
    # Puts the WebGPU runtime library (e.g. libwebgpu_dawn.so) next to App; does nothing on Emscripten
    target_copy_webgpu_binaries(App)
endif()

if (IS_EMSCRIPTEN)
    # Emit index.html (plus index.js/index.wasm) using shell.html as the page template,
    # so a static server pointed at the build folder serves the app at its root URL.
    # The renderer's own link flags (WebGL2, WebGPU) come from walnut-graphics-<renderer>.
    set(APP_SHELL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/shell.html")
    set_target_properties(App PROPERTIES
        OUTPUT_NAME index
        SUFFIX ".html"
        LINK_DEPENDS "${APP_SHELL_FILE}")
    target_link_options(App PRIVATE -sWASM=1 "SHELL:--shell-file \"${APP_SHELL_FILE}\"")
    # The example fills an image the size of its viewport every frame, which outgrows the default 16 MB heap
    # on large screens (a maximized window on a 4K monitor aborts with OOM).
    target_link_options(App PRIVATE -sALLOW_MEMORY_GROWTH=1)
    # Runtime checks that slow the app down, so not in the Release build hosted on GitHub Pages
    target_link_options(App PRIVATE $<$<CONFIG:Debug>:-sSAFE_HEAP=1>)
    if (RENDERER STREQUAL "OpenGL")
        target_link_options(App PRIVATE "$<$<CONFIG:Debug>:-sGL_DEBUG=1;-sGL_ASSERTIONS=1>")
    endif()
endif()

function(copy_runtime_dlls_at_build targetName)
    if(WIN32)
        add_custom_command(
            TARGET ${targetName}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t "$<TARGET_FILE_DIR:${targetName}>" "$<TARGET_RUNTIME_DLLS:${targetName}>"
            COMMENT "Copying runtime DLLs for ${targetName}"
            USES_TERMINAL
            COMMAND_EXPAND_LISTS
        )
    endif()
endfunction()

copy_runtime_dlls_at_build(App)

#### ANDROID APK ###
# CPack's APK generator packages the app without Gradle: `cmake --build <build folder> --target package` makes
# WalnutApp.apk in the build folder (conan build does this). It needs CMake 4.5 or newer, the Android SDK with build
# tools and platform 35, and a JDK. With Conan they come from conanfile.py; otherwise CPack looks for them in
# ANDROID_HOME and JAVA_HOME.
if(ANDROID)
    if(CMAKE_VERSION VERSION_LESS 4.5)
        message(WARNING "Packaging the app as an APK needs CMake 4.5 or newer, so there is no package target")
        return()
    endif()

    # SDL's Java side (SDLActivity and the classes it uses) is only distributed as source, in SDL's source release.
    # It has to come from the same SDL version as libSDL3.so.
    set(WALNUT_SDL_JAVA_VERSION 3.4.0)
    set(WALNUT_SDL_JAVA_SHA256 082cbf5f429e0d80820f68dc2b507a94d4cc1b4e70817b119bbb8ec6a69584b8)
    if(DEFINED SDL3_VERSION AND NOT SDL3_VERSION VERSION_EQUAL WALNUT_SDL_JAVA_VERSION)
        message(FATAL_ERROR "The app is built with SDL ${SDL3_VERSION}, but the APK gets the Java sources of SDL "
                            "${WALNUT_SDL_JAVA_VERSION}. Update WALNUT_SDL_JAVA_VERSION and WALNUT_SDL_JAVA_SHA256 in "
                            "${CMAKE_CURRENT_LIST_FILE}.")
    endif()
    set(sdl_archive "${CMAKE_CURRENT_BINARY_DIR}/SDL3-${WALNUT_SDL_JAVA_VERSION}.tar.gz")
    # Skipped when the archive is there already, with the right hash
    file(DOWNLOAD
        "https://github.com/libsdl-org/SDL/releases/download/release-${WALNUT_SDL_JAVA_VERSION}/SDL3-${WALNUT_SDL_JAVA_VERSION}.tar.gz"
        "${sdl_archive}"
        EXPECTED_HASH SHA256=${WALNUT_SDL_JAVA_SHA256}
        STATUS sdl_download_status)
    list(GET sdl_download_status 0 sdl_download_error)
    if(NOT sdl_download_error EQUAL 0)
        message(FATAL_ERROR "Could not download SDL's source release for its Java sources: ${sdl_download_status}")
    endif()
    set(sdl_java_path "SDL3-${WALNUT_SDL_JAVA_VERSION}/android-project/app/src/main/java")
    file(ARCHIVE_EXTRACT INPUT "${sdl_archive}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/sdl-java" PATTERNS "${sdl_java_path}")
    set(sdl_java_dir "${CMAKE_CURRENT_BINARY_DIR}/sdl-java/${sdl_java_path}")
    if(NOT EXISTS "${sdl_java_dir}/org/libsdl/app/SDLActivity.java")
        message(FATAL_ERROR "SDL's source release has no ${sdl_java_path}/org/libsdl/app/SDLActivity.java")
    endif()

    # What the app needs from the device, for the renderer it is built with
    if(RENDERER STREQUAL "Vulkan")
        # The renderer creates its Vulkan instance for version 1.2
        set(WALNUT_APK_RENDERER_FEATURE
            "<uses-feature android:name=\"android.hardware.vulkan.version\" android:version=\"0x402000\" android:required=\"true\" />")
    else()
        # OpenGL ES 3.0
        set(WALNUT_APK_RENDERER_FEATURE "<uses-feature android:glEsVersion=\"0x00030000\" android:required=\"true\" />")
    endif()
    configure_file(Android/AndroidManifest.xml.in "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" @ONLY)

    # The contents of the APK, in a component of their own. The generator packages libc++_shared.so itself.
    set(CPACK_APK_ABI "${CMAKE_ANDROID_ARCH_ABI}")
    if(NOT CPACK_APK_ABI)
        set(CPACK_APK_ABI "${ANDROID_ABI}") # set by the NDK's toolchain file
    endif()
    set(apk_library_dir "lib/${CPACK_APK_ABI}")
    install(TARGETS App LIBRARY DESTINATION ${apk_library_dir} COMPONENT apk)
    get_target_property(walnut_type walnut TYPE)
    if(walnut_type STREQUAL "SHARED_LIBRARY")
        install(TARGETS walnut LIBRARY DESTINATION ${apk_library_dir} COMPONENT apk)
    endif()
    # Shared libraries of the dependencies, set by conanfile.py
    set(WALNUT_APK_SHARED_LIBRARIES "" CACHE STRING "Shared libraries of the dependencies to package in the APK, such as libSDL3.so")
    if(WALNUT_APK_SHARED_LIBRARIES)
        install(FILES ${WALNUT_APK_SHARED_LIBRARIES} DESTINATION ${apk_library_dir} COMPONENT apk)
    else()
        message(WARNING "WALNUT_APK_SHARED_LIBRARIES is empty, so the APK will not have libSDL3.so")
    endif()
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" DESTINATION . COMPONENT apk)

    set(CPACK_GENERATOR APK)
    set(CPACK_PACKAGE_NAME WalnutApp)
    set(CPACK_PACKAGE_FILE_NAME WalnutApp)
    # Only the apk component: the install rules of WALNUT_INSTALL (headers, CMake package) are not for the APK
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};apk;/")
    set(CPACK_APK_JAVA_SOURCE_DIRECTORIES "${sdl_java_dir}")
    # Java 8, which SDL's Gradle project builds for as the Android Gradle plugin's default. For Java 9 and newer,
    # javac compiles string concatenation to StringConcatFactory calls that the platform's Java library lacks.
    set(CPACK_APK_JAVA_VERSION 8)
    # The API level the libraries are built for, from the toolchain (os.api_level in the Conan profile)
    set(CPACK_APK_MIN_SDK_VERSION ${CMAKE_SYSTEM_VERSION})
    # SDL's Java sources are written for API level 35
    set(CPACK_APK_COMPILE_SDK_VERSION 35)
    set(CPACK_APK_TARGET_SDK_VERSION 35)
    set(WALNUT_ANDROID_SDK_ROOT "" CACHE PATH "Android SDK for packaging the APK, instead of ANDROID_HOME")
    if(WALNUT_ANDROID_SDK_ROOT)
        set(CPACK_APK_SDK_ROOT "${WALNUT_ANDROID_SDK_ROOT}")
    endif()
    set(WALNUT_JAVA_HOME "" CACHE PATH "JDK for packaging the APK, instead of JAVA_HOME")
    if(WALNUT_JAVA_HOME)
        set(CPACK_APK_JAVA_HOME "${WALNUT_JAVA_HOME}")
    endif()
    include(CPack)
endif()