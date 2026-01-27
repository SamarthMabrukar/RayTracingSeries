// ShaderChecker.cpp
// COM DLL for checking Shader Model 6.9 support via DirectX 12
#include <windows.h>
#include <oleauto.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <strsafe.h>
#include "ShaderChecker.h"

// ============== AGILITY SDK CONFIGURATION ==============
// The Agility SDK path is resolved relative to the DLL location at runtime.
// We store the DLL directory and construct the full path in DllMain.
static char g_szAgilitySDKPath[MAX_PATH] = "";

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 717;
    // This pointer will be set to g_szAgilitySDKPath in DllMain
    __declspec(dllexport) extern const char* D3D12SDKPath = g_szAgilitySDKPath;
}

// GLOBALS
HMODULE ghModule = NULL;
long g_cLocks = 0;
long g_cComponents = 0;

// ============== COM CLASS ==============
class CShaderChecker : public IShaderChecker
{
    long m_cRef;
    ITypeInfo* m_pTypeInfo;
    
    // Cached diagnostic data (populated on first query)
    bool m_bQueried;
    bool m_bIsSupported;
    bool m_bExperimentalEnabled;
    D3D_SHADER_MODEL m_highestSM;
    D3D_FEATURE_LEVEL m_featureLevel;
    WCHAR m_szAdapterName[256];
    WCHAR m_szLastError[512];
    
public:
    CShaderChecker() : m_cRef(1), m_pTypeInfo(NULL),
        m_bQueried(false), m_bIsSupported(false), m_bExperimentalEnabled(false),
        m_highestSM(D3D_SHADER_MODEL_5_1), m_featureLevel(D3D_FEATURE_LEVEL_11_0)
    { 
        m_szAdapterName[0] = L'\0';
        m_szLastError[0] = L'\0';
        InterlockedIncrement(&g_cComponents); 
    }
    
    ~CShaderChecker() 
    { 
        // Safe cleanup - wrap in SEH to prevent crashes during destruction
        __try {
            if(m_pTypeInfo) 
            {
                m_pTypeInfo->Release();
                m_pTypeInfo = NULL;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            // Ignore exceptions during cleanup
        }
        InterlockedDecrement(&g_cComponents); 
    }
    
private:
    // Safe release helper - handles exceptions during Release
    template<typename T>
    static void SafeRelease(T*& ptr)
    {
        if(ptr)
        {
            __try {
                ptr->Release();
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                // Ignore exceptions during release
            }
            ptr = NULL;
        }
    }
    
    // Internal method to perform the actual GPU query
    void QueryGPUFeatures()
    {
		MessageBox(NULL,TEXT("Inside QueryGPUFeatures!"),TEXT("Debug Info"),MB_OK);
        if(m_bQueried) return; // Already queried
        m_bQueried = true;
        
        // Use raw pointers and be very careful with cleanup
        ID3D12Device* pDevice = NULL;
        IDXGIFactory4* pFactory = NULL;
        IDXGIAdapter1* pAdapter = NULL;
        HRESULT hr = S_OK;
        
        // Wrap entire operation in SEH
        __try 
		{
            // GUID for experimental shader models
            static const UUID D3D12ExperimentalShaderModels = 
                {0x76f5573e, 0xf13a, 0x40f5, {0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f}};
            
            // Step 1: Enable experimental features (ignore failure - not critical)
            hr = D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, NULL, NULL);
            m_bExperimentalEnabled = SUCCEEDED(hr);
            
            if(!m_bExperimentalEnabled)
            {
                StringCchCopyW(m_szLastError, 512, L"Experimental features not enabled (Developer Mode may be off)");
            }
            
            // Step 2: Get adapter info via DXGI
            hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
			if(SUCCEEDED(hr) && pFactory)
			{
				hr = pFactory->EnumAdapters1(0, &pAdapter);
				if(SUCCEEDED(hr) && pAdapter)
				{
					DXGI_ADAPTER_DESC1 desc = {};
					hr = pAdapter->GetDesc1(&desc);
					if(SUCCEEDED(hr))
					{
						StringCchCopyW(m_szAdapterName, 256, desc.Description);
					}
				}
			}
            
            // Step 3: Create D3D12 device (try multiple feature levels)
            // Try feature levels from highest to lowest
			static const D3D_FEATURE_LEVEL featureLevels[] = {
				D3D_FEATURE_LEVEL_12_2,
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0
			};
			
			for(int i = 0; i < _countof(featureLevels); i++)
			{
				hr = D3D12CreateDevice(
					pAdapter,  // Use specific adapter (may be NULL, that's OK)
					featureLevels[i],
					IID_PPV_ARGS(&pDevice)
				);
				if(SUCCEEDED(hr) && pDevice)
				{
					m_featureLevel = featureLevels[i];
					break;
				}
				pDevice = NULL; // Ensure null if failed
			}
            
            if(!pDevice)
            {
                StringCchCopyW(m_szLastError, 512, L"Failed to create D3D12 device");
                SafeRelease(pAdapter);
                SafeRelease(pFactory);
                return;
            }
            
            // Step 4: Query shader model
            D3D12_FEATURE_DATA_SHADER_MODEL smData = {};
			smData.HighestShaderModel = D3D_SHADER_MODEL_6_9;
			hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &smData, sizeof(smData));
			
			if(hr == E_INVALIDARG)
			{
				smData.HighestShaderModel = D3D_SHADER_MODEL_6_9;
				hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &smData, sizeof(smData));
				if(m_szLastError[0] == L'\0')
					StringCchCopyW(m_szLastError, 512, L"D3D12 runtime does not recognize SM 6.9 (update Agility SDK)");
			}
			
			if(SUCCEEDED(hr))
			{
				m_highestSM = smData.HighestShaderModel;
				m_bIsSupported = (m_highestSM >= D3D_SHADER_MODEL_6_9);
				if(m_bIsSupported)
					StringCchCopyW(m_szLastError, 512, L"SM 6.9 is supported");
				else if(m_szLastError[0] == L'\0')
					StringCchCopyW(m_szLastError, 512, L"GPU does not support SM 6.9");
			}
			else if(m_szLastError[0] == L'\0')
			{
				StringCchCopyW(m_szLastError, 512, L"CheckFeatureSupport failed");
			}
            
            // Cleanup - use safe release
            SafeRelease(pDevice);
            SafeRelease(pAdapter);
            SafeRelease(pFactory);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
		{
            StringCchCopyW(m_szLastError, 512, L"Exception occurred during GPU query");
            // Attempt cleanup
            SafeRelease(pDevice);
            SafeRelease(pAdapter);
            SafeRelease(pFactory);
        }
    }
    
public:
    
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
    {
        if(!ppv) return E_POINTER;
        *ppv = NULL;
        
        if(riid == IID_IUnknown || riid == IID_IDispatch || riid == IID_IShaderChecker)
        {
            *ppv = (IShaderChecker*)this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override
    { 
        return InterlockedIncrement(&m_cRef); 
    }
    
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG ref = InterlockedDecrement(&m_cRef);
        if(ref == 0) 
        {
            __try {
                delete this;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                // Ignore exceptions during delete
            }
        }
        return ref;
    }
    
    // IDispatch
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo) override
    {
        if(!pctinfo) return E_POINTER;
        *pctinfo = 1;
        return S_OK;
    }
    
    // Helper to ensure type info is loaded
    HRESULT EnsureTypeInfo()
    {
        if(m_pTypeInfo) return S_OK;
        
        ITypeLib* pTypeLib = NULL;
        WCHAR szPath[MAX_PATH];
        GetModuleFileNameW(ghModule, szPath, MAX_PATH);
        
        HRESULT hr = LoadTypeLib(szPath, &pTypeLib);
        if(FAILED(hr) || !pTypeLib) 
        {
            // Try loading .tlb file in same directory
            WCHAR szTlbPath[MAX_PATH];
            StringCchCopyW(szTlbPath, MAX_PATH, szPath);
            WCHAR* pDot = wcsrchr(szTlbPath, L'.');
            if(pDot) 
            {
                StringCchCopyW(pDot, 5, L".tlb");
                hr = LoadTypeLib(szTlbPath, &pTypeLib);
            }
            if(FAILED(hr) || !pTypeLib) return E_UNEXPECTED;
        }
        
        hr = pTypeLib->GetTypeInfoOfGuid(IID_IShaderChecker, &m_pTypeInfo);
        pTypeLib->Release();
        
        return (m_pTypeInfo) ? S_OK : E_UNEXPECTED;
    }
    
    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
    {
        if(!ppTInfo) return E_POINTER;
        *ppTInfo = NULL;
        
        if(iTInfo != 0) return DISP_E_BADINDEX;
        
        HRESULT hr = EnsureTypeInfo();
        if(FAILED(hr)) return hr;
        
        m_pTypeInfo->AddRef();
        *ppTInfo = m_pTypeInfo;
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
                                           LCID lcid, DISPID* rgDispId) override
    {
        HRESULT hr = EnsureTypeInfo();
        if(FAILED(hr)) return hr;
        return m_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispId);
    }
    
    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                                    DISPPARAMS* pDispParams, VARIANT* pVarResult,
                                    EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        HRESULT hr = EnsureTypeInfo();
        if(FAILED(hr)) return hr;
        return m_pTypeInfo->Invoke((IDispatch*)this, dispIdMember, wFlags, pDispParams,
                                  pVarResult, pExcepInfo, puArgErr);
    }
    
    // ==================== IShaderChecker Implementation ====================
    
    // Primary check - returns TRUE if SM 6.9 is supported
    HRESULT STDMETHODCALLTYPE get_IsSupported(VARIANT_BOOL* pVal) override
    {
        if(!pVal) return E_POINTER;
		MessageBox(NULL,TEXT("Inside get_IsSupported!"),TEXT("Debug Info"),MB_OK);
        QueryGPUFeatures();
        *pVal = m_bIsSupported ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    
    // Get highest shader model as string (e.g., "6.9", "6.8")
    HRESULT STDMETHODCALLTYPE get_HighestShaderModel(BSTR* pVal) override
    {
        if(!pVal) return E_POINTER;
		MessageBox(NULL,TEXT("Inside get_HighestShaderModel!"),TEXT("Debug Info"),MB_OK);
        QueryGPUFeatures();
        
        const WCHAR* szSM = L"Unknown";
        switch(m_highestSM)
        {
            case D3D_SHADER_MODEL_5_1:  szSM = L"5.1"; break;
            case D3D_SHADER_MODEL_6_0:  szSM = L"6.0"; break;
            case D3D_SHADER_MODEL_6_1:  szSM = L"6.1"; break;
            case D3D_SHADER_MODEL_6_2:  szSM = L"6.2"; break;
            case D3D_SHADER_MODEL_6_3:  szSM = L"6.3"; break;
            case D3D_SHADER_MODEL_6_4:  szSM = L"6.4"; break;
            case D3D_SHADER_MODEL_6_5:  szSM = L"6.5"; break;
            case D3D_SHADER_MODEL_6_6:  szSM = L"6.6"; break;
            case D3D_SHADER_MODEL_6_7:  szSM = L"6.7"; break;
            case D3D_SHADER_MODEL_6_8:  szSM = L"6.8"; break;
            case D3D_SHADER_MODEL_6_9:  szSM = L"6.9"; break;
            default: szSM = L"Unknown"; break;
        }
        *pVal = SysAllocString(szSM);
        return *pVal ? S_OK : E_OUTOFMEMORY;
    }
    
    // Get GPU adapter name
    HRESULT STDMETHODCALLTYPE get_AdapterName(BSTR* pVal) override
    {
        if(!pVal) return E_POINTER;
		MessageBox(NULL,TEXT("Inside get_AdapterName!"),TEXT("Debug Info"),MB_OK);
        QueryGPUFeatures();
        *pVal = SysAllocString(m_szAdapterName[0] ? m_szAdapterName : L"Unknown");
        return *pVal ? S_OK : E_OUTOFMEMORY;
    }
    
    // Get feature level as string
    HRESULT STDMETHODCALLTYPE get_FeatureLevel(BSTR* pVal) override
    {
        if(!pVal) return E_POINTER;
		MessageBox(NULL,TEXT("Inside get_FeatureLevel!"),TEXT("Debug Info"),MB_OK);
        QueryGPUFeatures();
        
        const WCHAR* szFL = L"Unknown";
        switch(m_featureLevel)
        {
            case D3D_FEATURE_LEVEL_11_0: szFL = L"11_0"; break;
            case D3D_FEATURE_LEVEL_11_1: szFL = L"11_1"; break;
            case D3D_FEATURE_LEVEL_12_0: szFL = L"12_0"; break;
            case D3D_FEATURE_LEVEL_12_1: szFL = L"12_1"; break;
            case D3D_FEATURE_LEVEL_12_2: szFL = L"12_2"; break;
            default: szFL = L"Unknown"; break;
        }
        *pVal = SysAllocString(szFL);
        return *pVal ? S_OK : E_OUTOFMEMORY;
    }
    
    // Get last error/status message
    HRESULT STDMETHODCALLTYPE get_LastError(BSTR* pVal) override
    {
        if(!pVal) return E_POINTER;
		MessageBox(NULL,TEXT("Inside get_LastError!"),TEXT("Debug Info"),MB_OK);
        QueryGPUFeatures();
        *pVal = SysAllocString(m_szLastError[0] ? m_szLastError : L"No error");
        return *pVal ? S_OK : E_OUTOFMEMORY;
    }
    
    // Check if experimental features enabled
    HRESULT STDMETHODCALLTYPE get_ExperimentalFeaturesEnabled(VARIANT_BOOL* pVal) override
    {
        if(!pVal) return E_POINTER;
		MessageBox(NULL,TEXT("Inside get_ExperimentalFeaturesEnabled!"),TEXT("Debug Info"),MB_OK);
        QueryGPUFeatures();
        *pVal = m_bExperimentalEnabled ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
};

// ============== CLASS FACTORY ==============
class CClassFactory : public IClassFactory
{
    long m_cRef;
    
public:
    CClassFactory() : m_cRef(1) {}
    
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
    {
        if(!ppv) return E_POINTER;
        *ppv = NULL;
        
        if(riid == IID_IUnknown || riid == IID_IClassFactory)
        {
            *ppv = (IClassFactory*)this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override 
    { 
        return InterlockedIncrement(&m_cRef); 
    }
    
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG ref = InterlockedDecrement(&m_cRef);
        if(ref == 0) delete this;
        return ref;
    }
    
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override
    {
        if(!ppv) return E_POINTER;
        *ppv = NULL;
        
        if(pUnkOuter) return CLASS_E_NOAGGREGATION;
        
        CShaderChecker* pObj = new CShaderChecker;
        if(!pObj) return E_OUTOFMEMORY;
        
        HRESULT hr = pObj->QueryInterface(riid, ppv);
        pObj->Release();
        return hr;
    }
    
    HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) override
    {
        if(fLock)
            InterlockedIncrement(&g_cLocks);
        else
            InterlockedDecrement(&g_cLocks);
        return S_OK;
    }
};

// ============== DLL EXPORTS ==============
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if(dwReason == DLL_PROCESS_ATTACH)
    {
        ghModule = hModule;
        
        // Build Agility SDK path relative to DLL location (not CWD!)
        // This ensures the SDK is found regardless of where the caller runs from
        char szDllPath[MAX_PATH];
        if(GetModuleFileNameA(hModule, szDllPath, MAX_PATH))
        {
            // Remove filename, keep directory
            char* pLastSlash = strrchr(szDllPath, '\\');
            if(pLastSlash) 
            {
                *(pLastSlash + 1) = '\0';
                // Construct full SDK path
                StringCchCopyA(g_szAgilitySDKPath, MAX_PATH, szDllPath);
                StringCchCatA(g_szAgilitySDKPath, MAX_PATH, 
                    "microsoft.direct3d.d3d12.1.717.1-preview\\build\\native\\bin\\x64\\");
            }
        }
        
        // Fallback if path construction failed
        if(g_szAgilitySDKPath[0] == '\0')
        {
            StringCchCopyA(g_szAgilitySDKPath, MAX_PATH, 
                ".\\microsoft.direct3d.d3d12.1.717.1-preview\\build\\native\\bin\\x64\\");
        }
    }
    return TRUE;
}

// THESE FUNCTIONS ARE EXPORTED VIA .DEF FILE
extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if(!ppv) return E_POINTER;
    *ppv = NULL;
    
    if(!IsEqualCLSID(rclsid, CLSID_ShaderChecker))
        return CLASS_E_CLASSNOTAVAILABLE;
    
    CClassFactory* pFactory = new CClassFactory;
    if(!pFactory) return E_OUTOFMEMORY;
    
    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return (g_cComponents == 0 && g_cLocks == 0) ? S_OK : S_FALSE;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
    WCHAR szModulePath[MAX_PATH];
    WCHAR szCLSID[64];
    WCHAR szKey[256];
    HKEY hKey;
    
    // Get DLL path
    GetModuleFileNameW(ghModule, szModulePath, MAX_PATH);
    
    // Convert CLSID to string
    StringFromGUID2(CLSID_ShaderChecker, szCLSID, 64);
    
    // Register Type Library
    ITypeLib* pTypeLib = NULL;
    LoadTypeLib(szModulePath, &pTypeLib);
    if(pTypeLib)
    {
        RegisterTypeLib(pTypeLib, szModulePath, NULL);
        pTypeLib->Release();
    }
    
    // Register CLSID
    StringCchPrintfW(szKey, 256, L"CLSID\\%s", szCLSID);
    RegCreateKeyExW(HKEY_CLASSES_ROOT, szKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)L"Shader Checker", 30);
    RegCloseKey(hKey);
    
    // Register InProcServer32
    StringCchPrintfW(szKey, 256, L"CLSID\\%s\\InProcServer32", szCLSID);
    RegCreateKeyExW(HKEY_CLASSES_ROOT, szKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szModulePath, (lstrlenW(szModulePath) + 1) * sizeof(WCHAR));
    RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (BYTE*)L"Apartment", 20);
    RegCloseKey(hKey);
    
    // Register ProgID
    RegCreateKeyExW(HKEY_CLASSES_ROOT, L"ShaderChecker.Checker", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)L"Shader Checker", 30);
    RegCloseKey(hKey);
    
    StringCchPrintfW(szKey, 256, L"ShaderChecker.Checker\\CLSID");
    RegCreateKeyExW(HKEY_CLASSES_ROOT, szKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szCLSID, (lstrlenW(szCLSID) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);
    
    // NO MessageBox - blocks automation!
    return S_OK;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    WCHAR szCLSID[64];
    WCHAR szKey[256];
    
    // Convert CLSID to string
    StringFromGUID2(CLSID_ShaderChecker, szCLSID, 64);
    
    // Unregister Type Library
    WCHAR szModulePath[MAX_PATH];
    GetModuleFileNameW(ghModule, szModulePath, MAX_PATH);
    ITypeLib* pTypeLib = NULL;
    if(SUCCEEDED(LoadTypeLib(szModulePath, &pTypeLib)))
    {
        UnRegisterTypeLib(LIBID_ShaderCheckerTypeLib, 1, 0, 0x00, SYS_WIN64);
        pTypeLib->Release();
    }
    
    // Delete ProgID
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"ShaderChecker.Checker\\CLSID");
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"ShaderChecker.Checker");
    
    // Delete CLSID
    StringCchPrintfW(szKey, 256, L"CLSID\\%s\\InProcServer32", szCLSID);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, szKey);
    
    StringCchPrintfW(szKey, 256, L"CLSID\\%s", szCLSID);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, szKey);
    
    // NO MessageBox - blocks automation!
    return S_OK;
}
