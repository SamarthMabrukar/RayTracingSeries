import win32com.client

# Method 1: Use the coclass name from IDL (CShaderChecker)
try:
    # Try with the coclass name
    checker = win32com.client.Dispatch("CShaderChecker")
    result = checker.IsSupported
    print(f"Method 1 (CShaderChecker): Shader Model 6.9 supported = {bool(result)}")
except Exception as e:
    print(f"Method 1 failed: {e}")
    
    # Method 2: Try with CLSID directly
    try:
        import pythoncom
        clsid = '{54F71999-647C-4B98-BFF9-84F28A9A25E5}'
        checker = win32com.client.Dispatch(clsid)
        result = checker.IsSupported
        print(f"Method 2 (CLSID): Shader Model 6.9 supported = {bool(result)}")
    except Exception as e2:
        print(f"Method 2 failed: {e2}")
        