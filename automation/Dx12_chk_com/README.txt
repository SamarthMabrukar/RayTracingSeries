=============================================
 Shader Model 6.9 Checker - Portable Solution
=============================================

This solution checks if your GPU supports DirectX 12 Shader Model 6.9.
It uses the D3D12 Agility SDK and COM automation for scripting support.

REQUIREMENTS
------------
- Windows 10 version 1903 or later / Windows 11
- Visual Studio 2019 or 2022 (for building)
- Python 3.x with pywin32 (for the client)
- DirectX 12 compatible GPU

PORTABLE DESIGN
---------------
This solution is fully portable:
- All paths are relative to the solution folder
- Copy the entire folder to any location/machine
- The Agility SDK is included in the folder
- No hardcoded paths

FOLDER STRUCTURE
----------------
DX12_checker_com/
  |-- ShaderChecker.cpp      # COM DLL source
  |-- ShaderChecker.h        # Header file
  |-- ShaderChecker.idl      # Interface definition
  |-- ShaderChecker.def      # DLL exports
  |-- Compile.bat            # Build script (auto-finds VS)
  |-- client.py              # Python automation client
  |-- README.txt             # This file
  |-- microsoft.direct3d.d3d12.1.717.1-preview/
      |-- build/native/bin/x64/
          |-- D3D12Core.dll  # Agility SDK runtime

QUICK START
-----------
1. Double-click Compile.bat (or run from Developer Command Prompt)
2. Open Admin Command Prompt in this folder
3. Run: regsvr32 ShaderChecker.dll
4. Run: python client.py

USAGE
-----
python client.py              # Check SM 6.9 support
python client.py register     # Register the DLL (requires Admin)
python client.py unregister   # Unregister the DLL (requires Admin)
python client.py help         # Show help

SAMPLE OUTPUT
-------------
Shader Model 6.9 Checker
==================================================
Querying GPU...

  GPU Adapter:       NVIDIA GeForce RTX 4090
  Feature Level:     12_2
  Highest SM:        6.9
  Experimental:      Enabled
  Status:            SM 6.9 is supported

==================================================
[OK] Shader Model 6.9 is SUPPORTED
==================================================

COPYING TO ANOTHER MACHINE
--------------------------
1. Copy the entire DX12_checker_com folder
2. On the new machine:
   - Install Visual Studio (or Build Tools)
   - Install Python with pywin32: pip install pywin32
3. Run Compile.bat to rebuild the DLL
4. Register and test

TROUBLESHOOTING
---------------
"cl.exe not found"
  - Run Compile.bat from Developer Command Prompt
  - Or install Visual Studio Build Tools

"Invalid class string" / "Class not registered"
  - Run: python client.py register (as Admin)
  - Or: regsvr32 ShaderChecker.dll (as Admin)

"SM 6.9 not supported" (false negative?)
  - Enable Windows Developer Mode (Settings > For Developers)
  - Update GPU drivers to latest version
  - Verify Agility SDK folder exists

API REFERENCE
-------------
COM ProgID: ShaderChecker.Checker

Properties:
  IsSupported              - VARIANT_BOOL: True if SM 6.9 supported
  HighestShaderModel       - BSTR: e.g., "6.9", "6.8", "6.6"
  AdapterName              - BSTR: GPU name
  FeatureLevel             - BSTR: e.g., "12_2", "12_1"
  LastError                - BSTR: Diagnostic message
  ExperimentalFeaturesEnabled - VARIANT_BOOL: True if experimental on

Example (Python):
  import win32com.client
  checker = win32com.client.Dispatch("ShaderChecker.Checker")
  print(f"SM 6.9 Supported: {checker.IsSupported}")
  print(f"GPU: {checker.AdapterName}")
  print(f"Highest SM: {checker.HighestShaderModel}")

Example (PowerShell):
  $checker = New-Object -ComObject ShaderChecker.Checker
  Write-Host "SM 6.9: $($checker.IsSupported)"
  Write-Host "GPU: $($checker.AdapterName)"

EXE SERVER MODE
---------------
You can also run this as a standalone HTTP server or Windows Service.

Building the EXE:
  1. Run: build_exe.bat
  2. Output: dist\ShaderCheckerServer.exe

Running as Standalone Server:
  dist\ShaderCheckerServer.exe
  
  This starts an HTTP server on port 8069.

Installing as Windows Service (requires Admin):
  dist\ShaderCheckerServer.exe install
  dist\ShaderCheckerServer.exe start

Removing Windows Service (requires Admin):
  dist\ShaderCheckerServer.exe stop
  dist\ShaderCheckerServer.exe remove

HTTP API Endpoints:
  GET http://localhost:8069/check   - Check SM 6.9 support (returns JSON)
  GET http://localhost:8069/health  - Health check

API Response Example:
  {
    "success": true,
    "sm69_supported": true,
    "adapter_name": "NVIDIA GeForce RTX 4090",
    "feature_level": "12_2",
    "highest_shader_model": "6.9",
    "experimental_features": true
  }

Testing the Server:
  python test_server.py              # Test localhost
  python test_server.py 192.168.1.5  # Test remote server

LICENSE
-------
This solution uses the D3D12 Agility SDK under Microsoft's license.
See microsoft.direct3d.d3d12.1.717.1-preview/LICENSE.txt
