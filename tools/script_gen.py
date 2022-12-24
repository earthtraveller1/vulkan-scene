import os

# A set of functions for generating system specific scripts.

if os.name == 'nt':
    SCRIPT_EXTENSION = '.ps1'
else:
    SCRIPT_EXTENSION = '.sh'

CLANG_TIDY_CHECKS = [
    "bugprone-*",
    "clang-analyzer-*",
    "misc-*",
    "performance-*",
    "portability-*",
    "readability-duplicate-include"
]

def generate_clean_script(files_to_clean: list):
    if os.name == 'nt':
        REMOVE_COMMAND = 'Remove-Item -Force'
    else:
        REMOVE_COMMAND = 'rm -f'
    
    script_file = open(f'clean{SCRIPT_EXTENSION}', 'w')
    
    if os.name == 'posix':
        # `sh` is the only shell that is guaranteed to exist.
        script_file.write('#!/bin/sh\n')
    
    for file in files_to_clean:
        script_file.write(f'{REMOVE_COMMAND} {file}\n')
    
    script_file.close()

def clang_tidy(files_to_tidy: list, compile_flags: str):
    script_file = open(f'clang-tidy{SCRIPT_EXTENSION}', 'w')
    
    command = 'clang-tidy --checks='
    for check in CLANG_TIDY_CHECKS:
        command += f'{check},'
    
    # Remove the last comma
    command = command[:-1]
    
    for file in files_to_tidy:
        script_file.write(f'{command} {file} -- {compile_flags}\n')