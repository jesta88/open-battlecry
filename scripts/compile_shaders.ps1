# PowerShell script for shader compilation
# Allows execution: Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

$ErrorActionPreference = "Stop"

# Check for Vulkan SDK
if (-not $env:VULKAN_SDK) {
    Write-Error "ERROR: VULKAN_SDK environment variable not set!"
    Write-Host "Please install Vulkan SDK and restart PowerShell."
    exit 1
}

$glslang = "$env:VULKAN_SDK\Bin\glslangValidator.exe"

if (-not (Test-Path $glslang)) {
    Write-Error "ERROR: glslangValidator.exe not found at $glslang"
    exit 1
}

# Detect Vulkan version automatically
try {
    $vulkanTargetEnv = & "$PSScriptRoot\detect_vulkan_version.ps1"
} catch {
    $vulkanTargetEnv = "vulkan1.3"
}
Write-Host "Using $vulkanTargetEnv as target environment" -ForegroundColor Cyan

Write-Host "Creating shader output directory..." -ForegroundColor Green
New-Item -ItemType Directory -Force -Path "bin\shaders" | Out-Null

Write-Host "Compiling shaders..." -ForegroundColor Green

# Compile GLSL shaders
$glslShaders = Get-ChildItem -Path "src\shaders\*.glsl" -ErrorAction SilentlyContinue
foreach ($shader in $glslShaders) {
    Write-Host "Compiling $($shader.Name)..." -ForegroundColor Yellow

    $stage = ""
    if ($shader.BaseName -match "mesh") {
        $stage = "mesh"
    } elseif ($shader.BaseName -match "frag") {
        $stage = "frag"
    } elseif ($shader.BaseName -match "vert") {
        $stage = "vert"
    } elseif ($shader.BaseName -match "comp") {
        $stage = "comp"
    } elseif ($shader.BaseName -match "task") {
        $stage = "task"
    } else {
        Write-Warning "Cannot determine stage for $($shader.Name), skipping"
        continue
    }

    $output = "bin\shaders\$($shader.BaseName).spv"
    & $glslang -V -S $stage --target-env $vulkanTargetEnv -o $output $shader.FullName

    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Compiled to $output" -ForegroundColor Green
    } else {
        Write-Error "✗ Failed to compile $($shader.Name)"
    }
}

# Compile shader files with standard extensions
$extensions = @("*.vert", "*.frag", "*.mesh", "*.task", "*.comp", "*.geom", "*.tesc", "*.tese")
foreach ($ext in $extensions) {
    $shaders = Get-ChildItem -Path "src\shaders\$ext" -ErrorAction SilentlyContinue
    foreach ($shader in $shaders) {
        Write-Host "Compiling $($shader.Name)..." -ForegroundColor Yellow

        $stage = $shader.Extension.TrimStart('.')
        $output = "bin\shaders\$($shader.Name).spv"

        & $glslang -V -S $stage --target-env $vulkanTargetEnv -o $output $shader.FullName

        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ Compiled to $output" -ForegroundColor Green
        } else {
            Write-Error "✗ Failed to compile $($shader.Name)"
        }
    }
}

# Compile HLSL shaders
$hlslShaders = Get-ChildItem -Path "src\shaders\*.hlsl" -ErrorAction SilentlyContinue
foreach ($shader in $hlslShaders) {
    Write-Host "Compiling HLSL $($shader.Name)..." -ForegroundColor Yellow

    $stage = ""
    if ($shader.BaseName -match "vertex|_vs") {
        $stage = "vert"
    } elseif ($shader.BaseName -match "fragment|pixel|_ps") {
        $stage = "frag"
    } elseif ($shader.BaseName -match "mesh|_ms") {
        $stage = "mesh"
    } elseif ($shader.BaseName -match "task|_as") {
        $stage = "task"
    } else {
        Write-Warning "Cannot determine stage for $($shader.Name), skipping"
        continue
    }

    $output = "bin\shaders\$($shader.BaseName).spv"
    & $glslang -V -D -S $stage -e main --target-env $vulkanTargetEnv -o $output $shader.FullName

    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Compiled to $output" -ForegroundColor Green
    } else {
        Write-Error "✗ Failed to compile $($shader.Name)"
    }
}

Write-Host "`nShader compilation complete!" -ForegroundColor Green
Write-Host "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")