#!/usr/bin/env bash
# Installs the example app on a new iPhone simulator, starts it, and checks that it is still running a minute later.
# Then sends it to the background by opening the Settings app, brings it back, and checks that the same process is
# still running. Leaves the simulator's description, the screenshots, the app's output, the system log and any crash
# reports in the output folder.
# Used by the ios job of .github/workflows/ci.yml. Needs Xcode with an iOS simulator runtime, and jq.
# Usage: simulator-test.sh <App.app> <output folder>
set -uo pipefail

app=$1
out=$2
bundle_id=io.github.nisaldilshan.walnut
mkdir -p "$out"
out=$(cd "$out" && pwd) # the simulator writes the app's output there, so the path has to be absolute

# The newest iOS runtime that is installed, and the newest iPhone it supports. Not every device type Xcode knows
# runs on every runtime: simctl create fails with "Incompatible device" then.
runtime_json=$(xcrun simctl list runtimes iOS --json | jq '[.runtimes[] | select(.isAvailable)] | last')
runtime=$(jq -r '.identifier' <<<"$runtime_json")
device_type=$(jq -r '[.supportedDeviceTypes[]? | select(.productFamily == "iPhone")] | last | .identifier' \
    <<<"$runtime_json")
if [ -z "$runtime" ] || [ "$runtime" = null ] || [ -z "$device_type" ] || [ "$device_type" = null ]; then
    echo "::error::No iOS simulator runtime or iPhone device type"
    xcrun simctl list runtimes
    exit 1
fi
udid=$(xcrun simctl create walnut-test "$device_type" "$runtime") \
    || { echo "::error::simctl create failed for $device_type on $runtime"; exit 1; }
echo "Simulator $udid: $device_type, $runtime" | tee "$out/simulator.txt"
trap 'xcrun simctl shutdown "$udid" >/dev/null 2>&1; xcrun simctl delete "$udid" >/dev/null 2>&1' EXIT

# Boots the simulator and waits until it has finished booting
xcrun simctl bootstatus "$udid" -b || { echo "::error::The simulator did not boot"; exit 1; }
xcrun simctl install "$udid" "$app" || { echo "::error::simctl install failed"; exit 1; }

# The process ID of the app, empty if it is not running
app_pid() {
    xcrun simctl spawn "$udid" launchctl list \
        | awk -v label="UIKitApplication:$bundle_id" 'index($3, label) == 1 && $1 != "-" { print $1 }'
}

xcrun simctl spawn "$udid" log stream --level debug --style compact \
    --predicate 'process == "App" OR (eventMessage CONTAINS "io.github.nisaldilshan.walnut")' > "$out/system-log.txt" 2>&1 &
log_pid=$!

# The app's stdout and stderr go to files of the output folder
xcrun simctl launch --terminate-running-process --stdout="$out/stdout.txt" --stderr="$out/stderr.txt" \
    "$udid" "$bundle_id" | tee "$out/launch.txt"

# Give the app time to create its window and render a few frames
sleep 30
xcrun simctl io "$udid" screenshot "$out/screenshot-30s.png"
sleep 30
xcrun simctl io "$udid" screenshot "$out/screenshot-60s.png"
pid_started=$(app_pid)

# To the background and back, if it is running. iOS ends apps that use the GPU in the background, so the app has to
# stop rendering until it is back.
if [ -n "$pid_started" ]; then
    xcrun simctl launch "$udid" com.apple.Preferences
    sleep 10
    xcrun simctl io "$udid" screenshot "$out/screenshot-background.png"
    xcrun simctl launch "$udid" "$bundle_id" | tee "$out/launch-resume.txt"
    sleep 20
    xcrun simctl io "$udid" screenshot "$out/screenshot-resumed.png"
fi
pid=$(app_pid)

kill "$log_pid" 2>/dev/null
# Crash reports of the simulator's processes end up in the Mac's own folder
find ~/Library/Logs/DiagnosticReports -name 'App*' -newer "$out/simulator.txt" -exec cp {} "$out/" \; 2>/dev/null

echo "----- The app's output"
cat "$out/stdout.txt" "$out/stderr.txt" 2>/dev/null | tail -n 100
echo "----- Crash reports"
ls "$out" | grep -E '\.(ips|crash)$' || echo "none"

if [ -z "$pid_started" ]; then
    echo "::error::$bundle_id is not running 60 seconds after it was started"
    exit 1
fi
echo "$bundle_id is running as process $pid_started 60 seconds after it was started"
if [ -z "$pid" ]; then
    echo "::error::$bundle_id is not running after going to the background and back"
    exit 1
fi
if [ "$pid" != "$pid_started" ]; then
    echo "::error::$bundle_id was restarted (process $pid_started, then $pid) after going to the background and back"
    exit 1
fi
echo "$bundle_id is still running as process $pid after going to the background and back"
