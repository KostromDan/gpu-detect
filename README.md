# GPU Detect

A lightweight utility for detecting and categorizing GPUs on Windows systems as either integrated or dedicated.

## Overview

This project was created to replace the Vulkan-based GPU detection logic used
in the [Crash Assistant](https://github.com/KostromDan/Crash-Assistant) project for showing a warning if Minecraft is
running on an integrated GPU while a dedicated GPU is available. This is a very common problem for Windows notebooks.
The original implementation was increasing the mod size by ~5 MB, and some modpack creators were not happy
about this. Therefore, this lightweight alternative was developed using DirectX APIs instead
(~20 KB in jar zip).

#### Output Example:

```text
INTEGRATED : AMD Radeon(TM) Graphics
DEDICATED : NVIDIA GeForce RTX 4090
```

## Features

- Detects both integrated and dedicated GPUs
- Outputs GPU information including device description and type
- Uses DirectX 12 and DXGI APIs for efficient hardware detection
- Significantly smaller footprint compared to Vulkan-based solutions (~20 KB in jar zip)
- Standalone executable with no external dependencies
- EXE and DLL (JNI for usage in Java) versions

## Requirements

- Windows operating system
- DirectX 12 compatible system

## Pre-built Binary

Download the latest release from the [Releases](https://github.com/KostromDan/gpu-detect/releases) page.

## Usage

### EXE:

    Run the executable from command prompt to see a list of available GPUs with their types: `gpu-detect.exe`
    
    If you just running exe as file it will close immediately after all printed, so you might want to pipe the output to a
    file or run it from the command prompt to see the results.

### JNI:

    Usege from Java:
    ```java
    public class DirectXGPUDetector {
        public native static String getSerialisedGPUs();
    }
    ```
    Load `gpu-detect-jni.dll` and use `getSerialisedGPUs` function.

## How It Works

The utility uses Windows DXGI (DirectX Graphics Infrastructure) to enumerate available graphics adapters and determine
whether they are integrated (UMA - Unified Memory Architecture) or dedicated GPUs. It specifically:

1. Creates a DXGI factory to access graphics hardware information
2. Enumerates adapters with different power preferences
3. Uses D3D12 feature checking to determine if an adapter uses UMA (indicating integrated GPU)
4. Displays the detected GPUs with their types and descriptions


