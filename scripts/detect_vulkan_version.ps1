# Detects Vulkan SDK version and outputs the target-env string
# Usage: ./detect_vulkan_version.ps1

if (-not $env:VULKAN_SDK) {
    Write-Output "vulkan1.3"
    exit 0
}

$headerPath = Join-Path $env:VULKAN_SDK "Include\vulkan\vulkan_core.h"

if (-not (Test-Path $headerPath)) {
    Write-Output "vulkan1.3"
    exit 0
}

$content = Get-Content $headerPath -Raw

# Look for VK_HEADER_VERSION_COMPLETE with VK_MAKE_API_VERSION
if ($content -match '#define VK_HEADER_VERSION_COMPLETE VK_MAKE_API_VERSION\((\d+), (\d+), (\d+), (\d+)\)') {
    $variant = $matches[1]
    $major = $matches[2]
    $minor = $matches[3]
    $patch = $matches[4]

    Write-Output "vulkan$major.$minor"
} else {
    # Fallback: try to find VK_API_VERSION_X_X definitions
    if ($content -match '#define VK_API_VERSION_1_3') {
        Write-Output "vulkan1.3"
    } elseif ($content -match '#define VK_API_VERSION_1_2') {
        Write-Output "vulkan1.2"
    } elseif ($content -match '#define VK_API_VERSION_1_1') {
        Write-Output "vulkan1.1"
    } elseif ($content -match '#define VK_API_VERSION_1_0') {
        Write-Output "vulkan1.0"
    } else {
        Write-Output "vulkan1.3"
    }
}