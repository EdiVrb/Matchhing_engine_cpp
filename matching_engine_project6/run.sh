
# ===== Script de compilation et exécution (run.sh) =====
#!/bin/bash
# run.sh - Script pour compiler et exécuter le matching engine

echo "=== Compilation du Matching Engine ==="
make clean
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "=== Compilation réussie ==="
    
    if [ $# -eq 2 ]; then
        echo "=== Exécution avec les fichiers fournis ==="
        ./bin/matching_engine "$1" "$2"
    else
        echo "Usage: ./run.sh <input.csv> <output.csv>"
    fi
else
    echo "=== Erreur de compilation ==="
    exit 1
fi
