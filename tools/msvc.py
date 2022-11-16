# Contains MSVC related toolchain stuff.
# Mostly for finding MSVC and it's stuff.

import os

# Finds the location of MSVC toolchain on the computer.
def find_msvc() -> str:
    visual_studio_dir = "C:\\Program Files\\Microsoft Visual Studio"
    
    if not os.path.isdir(visual_studio_dir):
        visual_studio_dir = "C:\\Program Files (x86)\\Microsoft Visual Studio"
    
    if not os.path.isdir(visual_studio_dir):
        print("[FATAL ERROR]: Visual Studio does not seems to be installed, or it's not in a place that could be found.")
        exit(-1)
    
    visual_studio_version = "0"
    
    for install in os.listdir(visual_studio_dir):
        if int(visual_studio_version) < int(install):
            visual_studio_version = install
    
    visual_studio_dir += f"\\{visual_studio_version}"
    
    msvc_dir = f"{visual_studio_dir}\\Community\\VC\\Tools"
    
    if not os.path.isdir(msvc_dir):
        msvc_dir = f"{visual_studio_dir}\\Professional\\VC\\Tools"
    
    if not os.path.isdir(msvc_dir):
        msvc_dir = f"{visual_studio_dir}\\Enterprise\\VC\\Tools"
    
    if not os.path.isdir(msvc_dir):
        print("[FATAL ERROR]: Either your Visual Studio installation is corrupted, or you do not have the C/C++ workloads installed.")
        exit(-1)
    
    # I'm making a leap of faith here and assuming that everyone has only one  
    # version of MSVC installed in their Visual Studio installation.
    msvc_dir += f"\\{os.listdir(msvc_dir)[0]}"
    
    # It's most likely guaranteed, but just wanna make sure.
    if not os.path.isdir(msvc_dir):
        print("[FATAL ERROR]: MSVC is not installed in the way I expected it to be!")
        exit(-1)
    
    return msvc_dir