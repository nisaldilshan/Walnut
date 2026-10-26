#!/usr/bin/env bash
# Builds and installs Walnut with vcpkg for one windowing system, then builds the app in tests/package
# against the installed package. Used by .github/workflows/ci.yml.
#
# Usage: vcpkg-build.sh <GLFW|SDL>
# RENDERER, BUILD_TYPE and VCPKG_ROOT come from the environment.
# Creates build-<windowing>/ and install-<windowing>/, e.g. build-glfw/ and install-glfw/.
set -euo pipefail

windowing=$1
suffix=$(echo "$windowing" | tr '[:upper:]' '[:lower:]')
build_dir=build-$suffix
install_dir=install-$suffix
toolchain="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

echo "::group::Configure"
cmake -B "$build_dir" \
    -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
    -DRENDERER="$RENDERER" \
    -DWINDOWING_SYSTEM="$windowing" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
echo "::endgroup::"

echo "::group::Build"
cmake --build "$build_dir" --config "$BUILD_TYPE" --parallel
echo "::endgroup::"

echo "::group::Install"
cmake --install "$build_dir" --config "$BUILD_TYPE" --prefix "$install_dir"
echo "::endgroup::"

# Only the installed configuration is generated: vcpkg maps MinSizeRel and RelWithDebInfo to
# Release, which a Debug install does not have.
echo "::group::Build an app against the installed package"
cmake -S tests/package -B "$build_dir-package" \
    -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
    -DVCPKG_MANIFEST_MODE=OFF \
    -DVCPKG_INSTALLED_DIR="$PWD/$build_dir/vcpkg_installed" \
    -DCMAKE_PREFIX_PATH="$PWD/$install_dir" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CONFIGURATION_TYPES="$BUILD_TYPE"
cmake --build "$build_dir-package" --config "$BUILD_TYPE" --parallel
echo "::endgroup::"
