import os

# A set of functions for generating system specific scripts.

if os.name == 'nt':
    SCRIPT_EXTENSION = '.ps1'
else:
    SCRIPT_EXTENSION = ''

def generate_clean_script(files_to_clean: list):
    if os.name == 'nt':
        REMOVE_COMMAND = 'Remove-Item -Force'
    else:
        REMOVE_COMMAND = 'rm -f'
    
    script_file = open(f'clean{SCRIPT_EXTENSION}', 'w')
    for file in files_to_clean:
        script_file.write(f'{REMOVE_COMMAND} {file}')
    
    script_file.close()