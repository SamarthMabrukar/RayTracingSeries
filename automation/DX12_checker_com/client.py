# client.py
# Shader Model 6.9 Checker - COM Automation Client
import win32com.client
import pythoncom
import sys
import subprocess
import os
import ctypes

def is_admin():
    """Check if running with admin privileges"""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def check_sm69(verbose=True):
    """Check if Shader Model 6.9 is supported via COM automation
    
    Args:
        verbose: If True, print detailed diagnostic info
        
    Returns:
        bool: True if SM 6.9 is supported
    """
    print("Shader Model 6.9 Checker")
    print("=" * 50)
    
    # Validate DLL exists
    dll_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ShaderChecker.dll")
    if not os.path.exists(dll_path):
        print(f"[ERROR] ShaderChecker.dll not found at: {dll_path}")
        print("  Build the DLL first using Compile.bat")
        return False
    
    # Validate Agility SDK exists
    sdk_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 
                            "microsoft.direct3d.d3d12.1.717.1-preview",
                            "build", "native", "bin", "x64", "D3D12Core.dll")
    if not os.path.exists(sdk_path):
        print(f"[WARNING] Agility SDK not found at expected location.")
        print(f"  Expected: {sdk_path}")
        print("  SM 6.9 check may use older D3D12 runtime (false negative possible)")
    
    # Initialize COM
    pythoncom.CoInitialize()
    
    try:
        # Create COM object
        checker = None
        try:
            checker = win32com.client.Dispatch("ShaderChecker.Checker")
        except:
            try:
                checker = win32com.client.Dispatch("ShaderCheckerTypeLib.CShaderChecker")
            except Exception as e:
                print(f"[ERROR] COM object not available: {e}")
                print("\nDLL may not be registered. Run as Administrator:")
                print("  python client.py register")
                return False
        
        print("Querying GPU...")
        
        # Get diagnostic info (new enhanced properties)
        try:
            adapter_name = checker.AdapterName
            feature_level = checker.FeatureLevel
            highest_sm = checker.HighestShaderModel
            experimental = checker.ExperimentalFeaturesEnabled
            last_error = checker.LastError
        except AttributeError:
            # Old interface without new properties
            adapter_name = "Unknown (rebuild DLL for diagnostics)"
            feature_level = "Unknown"
            highest_sm = "Unknown"
            experimental = False
            last_error = ""
        
        # Get the main result
        try:
            result = checker.IsSupported
        except Exception as e:
            print(f"[ERROR] Failed to query IsSupported: {e}")
            result = False
        
        # Print diagnostic info
        print("")
        print(f"  GPU Adapter:       {adapter_name}")
        print(f"  Feature Level:     {feature_level}")
        print(f"  Highest SM:        {highest_sm}")
        print(f"  Experimental:      {'Enabled' if experimental else 'Disabled'}")
        
        if verbose and last_error:
            print(f"  Status:            {last_error}")
        
        # Release COM object safely
        try:
            del checker
        except:
            pass  # Ignore cleanup errors
        
        # Print result
        print("")
        print("=" * 50)
        if result:
            print("[OK] Shader Model 6.9 is SUPPORTED")
            print("=" * 50)
            print("\nSM 6.9 features available:")
            print("  - Cooperative Vectors (AI/ML acceleration)")
            print("  - Extended Wave Operations")
            print("  - Advanced Hit Object support")
        else:
            print("[--] Shader Model 6.9 is NOT supported")
            print("=" * 50)
            print(f"\nYour GPU supports up to SM {highest_sm}")
            if not experimental:
                print("\nTip: Enable Windows Developer Mode for experimental features")
        
        return bool(result)
        
    except pythoncom.com_error as e:
        hr, msg, exc, arg = e.args
        print(f"[ERROR] COM Error: HRESULT=0x{hr & 0xFFFFFFFF:08X}")
        if exc:
            print(f"  Description: {exc[2]}")
        return False
        
    except Exception as e:
        print(f"[ERROR] {e}")
        if "Class not registered" in str(e) or "Invalid class string" in str(e):
            print("\nDLL not registered. Run as Administrator:")
            print("  python client.py register")
        return False
        
    finally:
        pythoncom.CoUninitialize()

def register():
    """Register the DLL using regsvr32"""
    dll_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ShaderChecker.dll")
    
    if not os.path.exists(dll_path):
        print("[ERROR] ShaderChecker.dll not found!")
        print(f"  Expected at: {dll_path}")
        return False
    
    if not is_admin():
        print("[WARNING] Not running as Administrator.")
        print("  Registration may fail. Run as Admin if it does.")
    
    print(f"Registering: {dll_path}")
    
    try:
        # Use regsvr32 with full path
        result = subprocess.run(
            ["regsvr32", "/s", dll_path], 
            capture_output=True, 
            text=True,
            timeout=30  # 30 second timeout
        )
        
        if result.returncode == 0:
            print("[OK] Registered successfully!")
            return True
        else:
            print(f"[ERROR] regsvr32 failed (code {result.returncode})")
            if result.stderr:
                print(f"  {result.stderr}")
            if not is_admin():
                print("\n  Try running as Administrator!")
            return False
            
    except subprocess.TimeoutExpired:
        print("[ERROR] Registration timed out!")
        return False
    except Exception as e:
        print(f"[ERROR] Registration failed: {e}")
        return False

def unregister():
    """Unregister the DLL"""
    dll_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ShaderChecker.dll")
    
    if not os.path.exists(dll_path):
        print("[ERROR] ShaderChecker.dll not found!")
        return False
    
    if not is_admin():
        print("[WARNING] Not running as Administrator.")
    
    print(f"Unregistering: {dll_path}")
    
    try:
        result = subprocess.run(
            ["regsvr32", "/s", "/u", dll_path], 
            capture_output=True, 
            text=True,
            timeout=30
        )
        
        if result.returncode == 0:
            print("[OK] Unregistered successfully!")
            return True
        else:
            print(f"[ERROR] Unregistration failed (code {result.returncode})")
            return False
            
    except subprocess.TimeoutExpired:
        print("[ERROR] Unregistration timed out!")
        return False
    except Exception as e:
        print(f"[ERROR] Unregistration failed: {e}")
        return False

def print_usage():
    print("Usage: python client.py [command]")
    print("")
    print("Commands:")
    print("  (none)      - Check SM 6.9 support")
    print("  register    - Register the COM DLL (requires Admin)")
    print("  unregister  - Unregister the COM DLL (requires Admin)")
    print("  test        - Same as no command")
    print("  help        - Show this help")

if __name__ == "__main__":
    # Change to script directory
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    
    if len(sys.argv) > 1:
        cmd = sys.argv[1].lower()
        if cmd == "register":
            success = register()
            sys.exit(0 if success else 1)
        elif cmd == "unregister":
            success = unregister()
            sys.exit(0 if success else 1)
        elif cmd == "test":
            success = check_sm69()
            sys.exit(0 if success else 1)
        elif cmd in ("help", "-h", "--help", "/?"):
            print_usage()
            sys.exit(0)
        else:
            print(f"Unknown command: {cmd}")
            print_usage()
            sys.exit(1)
    else:
        success = check_sm69()
        sys.exit(0 if success else 1)