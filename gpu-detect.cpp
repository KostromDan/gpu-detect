// gpu-detect.cpp : prints each adapter as INTEGRATED or DEDICATED
#include <windows.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <cstdio>
#include <wrl/client.h>

// Link against system DXGI, D3D12 and COM
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "ole32.lib")

// Use Microsoft's WRL::ComPtr for COM smart pointers
using Microsoft::WRL::ComPtr;

/**
 * Returns true if the adapter is UMA (i.e. integrated) by querying D3D12_FEATURE_ARCHITECTURE1.
 *
 * @param adapter Pointer to the DXGI adapter
 * @return true if the adapter uses UMA (integrated), false otherwise
 */
static bool IsUMAAdapter(IDXGIAdapter1 *adapter) {
    ComPtr<ID3D12Device> device;

    // Try to create a D3D12 device for the adapter
    if (FAILED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
        // Assume dedicated if we can't create a device
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
 * Enumerates adapters of a given GPU preference and prints those that match the
 * desired UMA / non‑UMA property. Returns the number of adapters printed.
 */
static int EnumerateAdapters(IDXGIFactory6 *factory,
                             DXGI_GPU_PREFERENCE preference,
                             bool wantUMA) {
    const char *label = wantUMA ? "INTEGRATED" : "DEDICATED";
    int printed = 0;

    for (UINT idx = 0;; ++idx) {
        ComPtr<IDXGIAdapter1> adapter;
        HRESULT hr = factory->EnumAdapterByGpuPreference(idx, preference,
                                                         IID_PPV_ARGS(&adapter));

        if (hr == DXGI_ERROR_NOT_FOUND)
            break; // finished

        if (FAILED(hr)) {
            fprintf(stderr,
                    "Warning: EnumAdapterByGpuPreference(%u) failed (0x%08lx)\n",
                    idx, hr);
            continue;
        }

        DXGI_ADAPTER_DESC1 desc = {};
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue; // skip WARP

        bool isUMA = IsUMAAdapter(adapter.Get());
        if (isUMA != wantUMA)
            continue;

        // Convert UTF‑16 description to UTF‑8 for printf
        char description[256];
        WideCharToMultiByte(CP_UTF8, 0,
                            desc.Description, -1,
                            description, sizeof(description),
                            nullptr, nullptr);

        printf("%s : %s\n", label, description);
        ++printed;
    }

    return printed;
}

int main() {
    // Initialise COM – D3D12 relies on a multi‑threaded apartment.
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hrCo)) {
        fprintf(stderr, "Error: COM initialisation failed (0x%08lx)\n", hrCo);
        return -1;
    }

    // Create DXGI factory
    ComPtr<IDXGIFactory6> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        fprintf(stderr, "Error: Failed to create DXGI factory\n");
        CoUninitialize();
        return -1;
    }

    // 1) Enumerate integrated GPUs (minimum power preference)
    int integratedCount = EnumerateAdapters(
        factory.Get(),
        DXGI_GPU_PREFERENCE_MINIMUM_POWER,
        true // Look for UMA (integrated) adapters
    );

    // 2) Enumerate dedicated GPUs (high performance preference)
    int dedicatedCount = EnumerateAdapters(
        factory.Get(),
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        false // Look for non-UMA (dedicated) adapters
    );

    // Exit with error if no adapters found
    if (integratedCount + dedicatedCount == 0) {
        fprintf(stderr, "Error: No supported graphics adapters found\n");
        CoUninitialize();
        return -1;
    }

    CoUninitialize();
    return 0;
}
