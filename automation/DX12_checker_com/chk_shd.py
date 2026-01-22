import win32com.client
import pythoncom
import traceback

def debug_com_object():
    """Debug COM object creation and method discovery"""
    
    print("Creating COM object...")
    try:
        # Initialize COM
        pythoncom.CoInitialize()
        
        # Create object using CLSID
        clsid = "{54F71999-647C-4B98-BFF9-84F28A9A25E5}"
        checker = win32com.client.Dispatch(clsid)
        
        print(f"✓ Object created successfully: {checker}")
        
        # List all available methods/properties
        print("\nAvailable methods/properties:")
        for attr in dir(checker):
            if not attr.startswith('_'):  # Skip private attributes
                print(f"  - {attr}")
        
        # Try to get TypeInfo
        print("\nTrying to get TypeInfo...")
        try:
            tinfo = checker._oleobj_.GetTypeInfo()
            print(f"  TypeInfo: {tinfo}")
        except Exception as e:
            print(f"  Failed to get TypeInfo: {e}")
        
        # Try different ways to call the property
        print("\nTrying different access methods:")
        
        # Method 1: Direct property
        try:
            result = checker.IsSupported
            print(f"  Direct property (IsSupported): {result}")
        except Exception as e:
            print(f"  Direct property failed: {e}")
        
        # Method 2: Call as method
        try:
            result = checker.get_IsSupported()
            print(f"  Method call (get_IsSupported()): {result}")
        except Exception as e:
            print(f"  Method call failed: {e}")
        
        # Method 3: Use dispatch
        try:
            # Get the underlying IDispatch
            dispatch = checker._oleobj_.QueryInterface(pythoncom.IID_IDispatch)
            
            # Try to get DISPID for "IsSupported"
            names = ["IsSupported", "get_IsSupported"]
            for name in names:
                try:
                    dispid = dispatch.GetIDsOfNames(0, name, 1)[0]
                    print(f"  Found DISPID for '{name}': {dispid}")
                    
                    # Try to invoke it
                    result = dispatch.Invoke(dispid, 0, pythoncom.DISPATCH_PROPERTYGET, ())
                    print(f"  Invoke returned: {result}")
                except Exception as e:
                    print(f"  Failed for '{name}': {e}")
                    
        except Exception as e:
            print(f"  Dispatch query failed: {e}")
        
        # Cleanup
        del checker
        
    except Exception as e:
        print(f"✗ Failed to create object: {e}")
        traceback.print_exc()
        
    finally:
        pythoncom.CoUninitialize()

def check_com_registration():
    """Check Windows Registry for COM registration"""
    import winreg
    
    print("\n" + "="*60)
    print("Checking COM Registration in Registry")
    print("="*60)
    
    clsid = "{54F71999-647C-4B98-BFF9-84F28A9A25E5}"
    
    # Check CLSID
    try:
        key = winreg.OpenKey(winreg.HKEY_CLASSES_ROOT, f"CLSID\\{clsid}")
        print(f"✓ CLSID found: {clsid}")
        
        # Get default value (description)
        try:
            desc, _ = winreg.QueryValueEx(key, "")
            print(f"  Description: {desc}")
        except:
            pass
            
        # Check InProcServer32
        try:
            subkey = winreg.OpenKey(key, "InProcServer32")
            dll_path, _ = winreg.QueryValueEx(subkey, "")
            threading, _ = winreg.QueryValueEx(subkey, "ThreadingModel")
            print(f"  DLL Path: {dll_path}")
            print(f"  Threading Model: {threading}")
            winreg.CloseKey(subkey)
        except Exception as e:
            print(f"  InProcServer32 error: {e}")
            
        winreg.CloseKey(key)
    except Exception as e:
        print(f"✗ CLSID not found: {e}")
    
    # Check TypeLib
    try:
        key = winreg.OpenKey(winreg.HKEY_CLASSES_ROOT, f"CLSID\\{clsid}\\TypeLib")
        typelib_id, _ = winreg.QueryValueEx(key, "")
        print(f"  TypeLib ID: {typelib_id}")
        winreg.CloseKey(key)
    except:
        print("  No TypeLib registered")
    
    # Check ProgID
    print("\nChecking ProgIDs...")
    progids = ["ShaderChecker.Checker", "CShaderChecker"]
    for progid in progids:
        try:
            key = winreg.OpenKey(winreg.HKEY_CLASSES_ROOT, progid)
            print(f"✓ ProgID found: {progid}")
            
            # Get CLSID from ProgID
            try:
                clsid_key = winreg.OpenKey(key, "CLSID")
                progid_clsid, _ = winreg.QueryValueEx(clsid_key, "")
                print(f"  CLSID: {progid_clsid}")
                winreg.CloseKey(clsid_key)
            except:
                pass
                
            winreg.CloseKey(key)
        except:
            print(f"✗ ProgID not found: {progid}")

if __name__ == "__main__":
    print("="*60)
    print("DETAILED COM OBJECT DEBUG")
    print("="*60)
    
    debug_com_object()
    check_com_registration()
    
    print("\n" + "="*60)
    print("QUICK FIX ATTEMPT")
    print("="*60)
    
    # Try with late binding (no type info)
    try:
        pythoncom.CoInitialize()
        
        # Create object with no type info
        clsid = pythoncom.MakeIID("{54F71999-647C-4B98-BFF9-84F28A9A25E5}")
        unknown = pythoncom.CoCreateInstance(clsid, None, pythoncom.CLSCTX_ALL, pythoncom.IID_IUnknown)
        dispatch = unknown.QueryInterface(pythoncom.IID_IDispatch)
        
        # Try to invoke by DISPID 1 (since IDL shows id=1)
        print("\nTrying DISPID 1 (from IDL)...")
        try:
            result = dispatch.Invoke(1, 0, pythoncom.DISPATCH_PROPERTYGET, ())
            print(f"  DISPID 1 returned: {result}")
            print(f"  Shader Model 6.9 supported: {bool(result)}")
        except Exception as e:
            print(f"  DISPID 1 failed: {e}")
            
        pythoncom.CoUninitialize()
    except Exception as e:
        print(f"Quick fix failed: {e}")
        traceback.print_exc()
        