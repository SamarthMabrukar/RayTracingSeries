import winreg

def extract_com_info():
    clsid = "{54F71999-647C-4B98-BFF9-84F28A9A25E5}"
    base_key = winreg.HKEY_CLASSES_ROOT
    print("="*80)
    print(f"COMPLETE COM REGISTRATION REPORT FOR CLSID: {clsid}")
    print("="*80)

    # 1. DIRECT CLSID CHECK
    print("\n[1] DIRECT CLSID REGISTRATION:")
    try:
        clsid_key_path = f"CLSID\\{clsid}"
        clsid_key = winreg.OpenKey(base_key, clsid_key_path)
        try:
            name, _ = winreg.QueryValueEx(clsid_key, "")
            print(f"   ✓ Found. Display Name: '{name}'")
        except FileNotFoundError:
            print("   ✓ Found (No display name set)")
        winreg.CloseKey(clsid_key)
    except FileNotFoundError:
        print(f"   ✗ NOT FOUND at HKCR\\CLSID\\{clsid}")
        return

    # 2. INPROCSERVER32 DETAILS
    print("\n[2] INPROCSERVER32 (DLL PATH & THREADING):")
    try:
        server_path = f"CLSID\\{clsid}\\InProcServer32"
        server_key = winreg.OpenKey(base_key, server_path)
        dll_path, _ = winreg.QueryValueEx(server_key, "")
        print(f"   ✓ DLL Path: {dll_path}")
        try:
            threading, _ = winreg.QueryValueEx(server_key, "ThreadingModel")
            print(f"   ✓ ThreadingModel: {threading}")
        except FileNotFoundError:
            print("   ⚠ ThreadingModel not specified (defaults to 'Apartment')")
        winreg.CloseKey(server_key)
    except FileNotFoundError:
        print("   ✗ InProcServer32 key not found.")

    # 3. TYPELIB REGISTRATION
    print("\n[3] TYPE LIBRARY REGISTRATION:")
    try:
        typelib_path = f"CLSID\\{clsid}\\TypeLib"
        typelib_key = winreg.OpenKey(base_key, typelib_path)
        typelib_id, _ = winreg.QueryValueEx(typelib_key, "")
        print(f"   ✓ TypeLib ID: {typelib_id}")
        winreg.CloseKey(typelib_key)
        # Check TypeLib details
        try:
            tlib_key = winreg.OpenKey(base_key, f"TypeLib\\{typelib_id}")
            print(f"   ✓ TypeLib root key exists.")
            winreg.CloseKey(tlib_key)
        except FileNotFoundError:
            print("   ⚠ TypeLib ID registered but root key is missing.")
    except FileNotFoundError:
        print("   ⚠ No TypeLib registered for this CLSID.")

    # 4. FIND ALL PROGIDs LINKED TO THIS CLSID
    print("\n[4] SEARCHING FOR LINKED ProgIDs...")
    found_progids = []
    try:
        # Iterate through HKCR to find ProgIDs pointing to our CLSID
        root = winreg.OpenKey(base_key, "")
        i = 0
        while True:
            try:
                subkey_name = winreg.EnumKey(root, i)
                # Skip system keys
                if not subkey_name.startswith(('.', 'CLSID', 'TypeLib', 'Interface', 'AppID', 'Record')):
                    try:
                        subkey = winreg.OpenKey(base_key, subkey_name)
                        # Check if this ProgID has a CLSID subkey
                        try:
                            clsid_subkey = winreg.OpenKey(subkey, "CLSID")
                            progid_clsid, _ = winreg.QueryValueEx(clsid_subkey, "")
                            if progid_clsid.upper() == clsid.upper():
                                found_progids.append(subkey_name)
                            winreg.CloseKey(clsid_subkey)
                        except FileNotFoundError:
                            pass  # No CLSID subkey, not a ProgID
                        winreg.CloseKey(subkey)
                    except (FileNotFoundError, PermissionError):
                        pass
                i += 1
            except OSError:  # No more keys
                break
        winreg.CloseKey(root)
    except Exception as e:
        print(f"   Error during ProgID search: {e}")

    if found_progids:
        print(f"   ✓ Found {len(found_progids)} ProgID(s):")
        for progid in found_progids:
            print(f"      - '{progid}'")
    else:
        print("   ⚠ No ProgIDs found pointing to this CLSID.")

    # 5. SUMMARY AND DIAGNOSIS
    print("\n" + "="*80)
    print("DIAGNOSIS & PYTHON ACCESS STRATEGY")
    print("="*80)

    if found_progids:
        print(f"\nRECOMMENDED: Use the ProgID with win32com.")
        print(f'    import win32com.client')
        print(f'    obj = win32com.client.Dispatch("{found_progids[0]}")')
        print(f'    # Then try: obj.IsSupported or obj.get_IsSupported()')
    else:
        print(f"\nNO PROGID FOUND. You must use the CLSID directly.")
        print(f'    import win32com.client')
        print(f'    obj = win32com.client.Dispatch("{clsid}")')
        print(f'    # Then try: obj.IsSupported or obj.get_IsSupported()')

    print("\nIf property access still fails with the above, the issue is likely:")
    print("  1. The COM object's `get_IsSupported` method is returning an error HRESULT.")
    print("  2. The Type Library (.tlb) is not correctly registered or embedded.")
    print("  3. The DLL itself fails to create/query the D3D12 device.")

    print("\nNEXT STEP: Run the script above. If it finds a ProgID, use it in this one-liner:")
    print(f'  python -c "import win32com.client; print(win32com.client.Dispatch(\'{found_progids[0] if found_progids else clsid}\').IsSupported)"')
    print("\n" + "="*80)

if __name__ == "__main__":
    try:
        # Quick admin check
        test_key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
        winreg.CloseKey(test_key)
        extract_com_info()
    except PermissionError:
        print("ERROR: This script must be run as Administrator.")
        print("Right-click on your Python IDE or Command Prompt and select 'Run as administrator'.")
        