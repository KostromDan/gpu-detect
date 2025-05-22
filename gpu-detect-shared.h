// gpu-detect-shared.h: Common GPU detection functionality
#pragma once

#include <windows.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <cstdio>
#include <cstring>
#include <wrl/client.h>

// Use Microsoft's WRL::ComPtr for COM smart pointers
using Microsoft::WRL::ComPtr;

/**
 * Returns true if the adapter is UMA (i.e. integrated) by querying D3D12_FEATURE_ARCHITECTURE1.
 */
static bool IsUMAAdapter(IDXGIAdapter1 *adapter, const char *adapterName, char *buffer, size_t bufferSize,
                         size_t *writtenSize) {
    ComPtr<ID3D12Device> device;

    // Try to create a D3D12 device for the adapter
    HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr)) {
        // Assume dedicated if we can't create a device
        int written = snprintf(buffer + *writtenSize, bufferSize - *writtenSize,
                               "Warning: Failed to create D3D12 device for adapter '%s' (0x%08lx) - assuming dedicated\n",
                               adapterName, hr);
        if (written > 0) *writtenSize += written;
        return false;
    }

    // Query the architectural features of the adapter
    D3D12_FEATURE_DATA_ARCHITECTURE1 arch = {};
    bool isUMA = SUCCEEDED(device->CheckFeatureSupport(
        D3D12_FEATURE_ARCHITECTURE1, &arch, sizeof(arch)
    )) && arch.UMA;

    return isUMA;
}

/**
 * Returns true if `description` already exists in the first `len` bytes of `buffer`.
 */
static bool AlreadyListed(const char *buffer, size_t len, const char *description) {
    if (len == 0) return false;

    const char *end = buffer + len; // protect against stray NULs beyond len
    char saved = *end;
    *const_cast<char *>(end) = '\0';

    bool found = (strstr(buffer, description) != nullptr);

    *const_cast<char *>(end) = saved;
    return found;
}

/**
 * Enumerates adapters of a given GPU preference and writes output to buffer.
 * Returns the number of adapters printed.
 */
static int EnumerateAdapters(IDXGIFactory6 *factory,
                             DXGI_GPU_PREFERENCE preference,
                             bool wantUMA,
                             char *buffer,
                             size_t bufferSize,
                             size_t *writtenSize) {
    const char *label = wantUMA ? "INTEGRATED" : "DEDICATED";
    int printed = 0;
    size_t offset = *writtenSize;

    for (UINT idx = 0;; ++idx) {
        ComPtr<IDXGIAdapter1> adapter;
        HRESULT hr = factory->EnumAdapterByGpuPreference(idx, preference,IID_PPV_ARGS(&adapter));

        if (hr == DXGI_ERROR_NOT_FOUND)
            break; // finished

        if (FAILED(hr)) {
            int written = snprintf(buffer + offset, bufferSize - offset,
                                   "Warning: EnumAdapterByGpuPreference(%u) failed (0x%08lx)\n",
                                   idx, hr);
            if (written > 0) offset += written;
            continue;
        }

        DXGI_ADAPTER_DESC1 desc = {};
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue; // skip WARP

        // Convert UTF‑16 description to UTF‑8 for printf
        char description[256] = {0};
        WideCharToMultiByte(CP_UTF8, 0,
                            desc.Description, -1,
                            description, sizeof(description),
                            nullptr, nullptr);

        /* duplicate filter */
        if (AlreadyListed(buffer, offset, description))
            continue;

        bool isUMA = IsUMAAdapter(adapter.Get(), description, buffer, bufferSize, &offset);
        if (isUMA != wantUMA)
            continue;

        int written = snprintf(buffer + offset, bufferSize - offset,
                               "%s : %s\n", label, description);
        if (written > 0) {
            offset += written;
            ++printed;
        }
    }

    *writtenSize = offset;
    return printed;
}

/**
 * Detects GPU information and writes output to the provided buffer.
 * Returns a pointer to the buffer.
 */
static char *DetectGPUs(char *buffer, size_t bufferSize) {
    size_t offset = 0;

    // Initialise COM – D3D12 relies on a multi‑threaded apartment.
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hrCo)) {
        snprintf(buffer, bufferSize,
                 "Error: COM initialisation failed (0x%08lx)\n", hrCo);
        return buffer;
    }

    // Create DXGI factory
    ComPtr<IDXGIFactory6> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        snprintf(buffer, bufferSize, "Error: Failed to create DXGI factory\n");
        CoUninitialize();
        return buffer;
    }

    // 1) Enumerate integrated GPUs (minimum power preference)
    int integratedCount = EnumerateAdapters(
        factory.Get(),
        DXGI_GPU_PREFERENCE_MINIMUM_POWER,
        true, // Look for UMA (integrated) adapters
        buffer,
        bufferSize,
        &offset
    );

    // 2) Enumerate dedicated GPUs (high performance preference)
    int dedicatedCount = EnumerateAdapters(
        factory.Get(),
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        false, // Look for non-UMA (dedicated) adapters
        buffer,
        bufferSize,
        &offset
    );

    // Exit with error if no adapters found
    if (integratedCount + dedicatedCount == 0) {
        snprintf(buffer + offset, bufferSize - offset,
                 "Error: No supported graphics adapters found\n");
    }

    CoUninitialize();
    return buffer;
}
