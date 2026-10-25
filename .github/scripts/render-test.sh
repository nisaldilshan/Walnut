#!/usr/bin/env bash
# Runs the render test (tests/render) of a Linux build on a virtual display, with Mesa's software
# renderers: llvmpipe for OpenGL and lavapipe for Vulkan and WebGPU. Used by .github/workflows/ci.yml.
#
# Usage: render-test.sh <cmake build folder>
# Needs the xvfb, mesa-vulkan-drivers and libgl1-mesa-dri packages.
set -euo pipefail

build_dir=$1

# Conan's run environment points the Vulkan loader at the validation layers.
if [ -f "$build_dir/generators/conanrun.sh" ]; then
    set +u # the script extends variables that may not be set yet, e.g. LD_LIBRARY_PATH
    source "$build_dir/generators/conanrun.sh"
    set -u
fi

# Only lavapipe, so the loader does not try the hardware drivers.
VK_ICD_FILENAMES=$(ls /usr/share/vulkan/icd.d/lvp_icd*.json | head -n 1)
export VK_ICD_FILENAMES
export LIBGL_ALWAYS_SOFTWARE=1

xvfb-run --auto-servernum --server-args="-screen 0 1280x1024x24" \
    ctest --test-dir "$build_dir" --output-on-failure
