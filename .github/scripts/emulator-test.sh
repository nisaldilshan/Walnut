#!/usr/bin/env bash
# Installs the example app's APK on the running emulator, starts it, and checks that it is still running a minute
# later. Leaves the emulator's ABIs and EGL extensions, two screenshots and the logcat in the output folder.
# Used by .github/workflows/android-emulator.yml.
# Usage: emulator-test.sh <apk> <output folder>
set -uo pipefail

apk=$1
out=$2
package=io.github.nisaldilshan.walnut
mkdir -p "$out"

adb shell getprop ro.product.cpu.abilist | tee "$out/abilist.txt"
adb shell dumpsys SurfaceFlinger | grep -iE "^EGL|GLES|gl_colorspace" | tee "$out/egl.txt"
grep -q EGL_KHR_gl_colorspace "$out/egl.txt" && echo "EGL_KHR_gl_colorspace: present" || echo "EGL_KHR_gl_colorspace: absent"
adb install -r "$apk" || { echo "::error::adb install failed"; exit 1; }

# A freshly booted emulator is still busy (input devices, carrier configuration, install broadcasts). Started right
# away, the app's activity was relaunched within a second, which ends an SDL app.
sleep 90
adb logcat -c
adb shell am start -W -n "$package/org.libsdl.app.SDLActivity" | tee "$out/am-start.txt"

# Give the app time to create its window and render a few frames
sleep 30
adb exec-out screencap -p > "$out/screenshot-30s.png"
sleep 30
adb exec-out screencap -p > "$out/screenshot-60s.png"

adb logcat -d > "$out/logcat.txt"
adb shell dumpsys activity activities | grep -E 'ResumedActivity' | tee "$out/resumed.txt"
pid=$(adb shell pidof "$package" | tr -d '\r')

echo "----- App, SDL and crash lines from logcat"
grep -E "SDL|walnut|Walnut|AndroidRuntime|FATAL|Fatal signal|DEBUG +:|libc +:|linker|vulkan|Vulkan|EGL|GLES|OpenGL" "$out/logcat.txt" | grep -v ' I chatty' | tail -n 150

if [ -z "$pid" ]; then
    echo "::error::$package is not running 60 seconds after it was started"
    exit 1
fi
echo "$package is running as process $pid"
