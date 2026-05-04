import os

# Konfiguracja - edytuj według potrzeb
OUTPUT_FILE = "project_context.txt"
EXCLUDE_DIRS = {'.git', 'build', '.vscode', '__pycache__', 'node_modules', 'devcontainer_data'}
#ALLOWED_EXTENSIONS = {'.hpp', '.cpp', '.h', '.c', '.md', '.txt', '.yml', '.yaml', 'Dockerfile', 'CMakeLists.txt'}
ALLOWED_EXTENSIONS = {'.hpp', '.h'}

def merge_project():
    with open(OUTPUT_FILE, 'w', encoding='utf-8') as outfile:
        for root, dirs, files in os.walk('.'):
            # Usuwamy niechciane foldery z walk
            dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]
            
            for file in files:
                if file == OUTPUT_FILE or file == 'context_merger.py':
                    continue
                
                # Sprawdzamy rozszerzenie lub konkretne nazwy plików (np. Dockerfile)
                if any(file.endswith(ext) for ext in ALLOWED_EXTENSIONS) or file in ALLOWED_EXTENSIONS:
                    file_path = os.path.join(root, file)
                    relative_path = os.path.relpath(file_path, '.')
                    
                    try:
                        with open(file_path, 'r', encoding='utf-8') as infile:
                            content = infile.read()
                            
                        outfile.write(f"\n{'='*50}\n")
                        outfile.write(f"FILE: {relative_path}\n")
                        outfile.write(f"{'='*50}\n\n")
                        outfile.write(content)
                        outfile.write("\n\n")
                        print(f"Added: {relative_path}")
                    except Exception as e:
                        print(f"Skipping {relative_path}: {e}")

if __name__ == "__main__":
    merge_project()
    print(f"\nDone! Context saved to: {OUTPUT_FILE}")