// ShaderChecker.h
// Enhanced interface with diagnostic properties
#pragma once

#include <windows.h>
#include <oaidl.h>

// GUIDs
const CLSID CLSID_ShaderChecker = 
    {0x54f71999, 0x647c, 0x4b98, {0xbf, 0xf9, 0x84, 0xf2, 0x8a, 0x9a, 0x25, 0xe5}};

const IID IID_IShaderChecker = 
    {0xaac09469, 0x23d7, 0x4b8e, {0xb4, 0x54, 0x77, 0x6c, 0xa3, 0xb1, 0x9a, 0xd2}};

const GUID LIBID_ShaderCheckerTypeLib = 
    {0x65c6a5b5, 0xb00f, 0x4ece, {0x82, 0x62, 0xb9, 0x93, 0xb4, 0xb8, 0x05, 0x42}};

// Enhanced interface with diagnostic properties
class IShaderChecker : public IDispatch
{
public:
    // Primary check - returns TRUE if SM 6.9 is supported
    virtual HRESULT __stdcall get_IsSupported(VARIANT_BOOL* pVal) = 0;
    
    // Get the highest shader model supported (e.g., "6.9", "6.8")
    virtual HRESULT __stdcall get_HighestShaderModel(BSTR* pVal) = 0;
    
    // Get the GPU adapter name
    virtual HRESULT __stdcall get_AdapterName(BSTR* pVal) = 0;
    
    // Get the feature level (e.g., "12_2", "12_1")
    virtual HRESULT __stdcall get_FeatureLevel(BSTR* pVal) = 0;
    
    // Get last error/status message
    virtual HRESULT __stdcall get_LastError(BSTR* pVal) = 0;
    
    // Check if experimental features are enabled
    virtual HRESULT __stdcall get_ExperimentalFeaturesEnabled(VARIANT_BOOL* pVal) = 0;
};
