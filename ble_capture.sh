#!/bin/bash

# Script pour faciliter l'utilisation du sniffer BLE BRMesh
# Usage: ./ble_capture.sh [nom_effet]

EFFECT_NAME=${1:-"test"}
CAPTURE_FILE="captures/effect_${EFFECT_NAME}_$(date +%Y%m%d_%H%M%S).txt"

# Créer le dossier captures s'il n'existe pas
mkdir -p captures

echo "╔═══════════════════════════════════════════════╗"
echo "║     BRMesh BLE Effect Capture Tool           ║"
echo "╚═══════════════════════════════════════════════╝"
echo ""
echo "📝 Effet à capturer : $EFFECT_NAME"
echo "💾 Fichier de capture : $CAPTURE_FILE"
echo ""
echo "📋 Instructions :"
echo "  1. Assurez-vous que le sniffer BLE est uploadé sur l'ESP32"
echo "  2. Ouvrez l'application brMesh/Broadlink BLE"
echo "  3. Activez l'effet '$EFFECT_NAME' sur un spot"
echo "  4. Attendez 5-10 secondes"
echo "  5. Appuyez sur Ctrl+C pour arrêter"
echo ""
echo "🔍 Recherche du port série de l'ESP32..."

# Trouver le port ESP32 (macOS)
ESP_PORT=$(ls /dev/cu.usbserial-* 2>/dev/null | head -1)
if [ -z "$ESP_PORT" ]; then
    ESP_PORT=$(ls /dev/cu.SLAB_USBtoUART 2>/dev/null | head -1)
fi
if [ -z "$ESP_PORT" ]; then
    ESP_PORT=$(ls /dev/cu.wchusbserial* 2>/dev/null | head -1)
fi

if [ -z "$ESP_PORT" ]; then
    echo "❌ Aucun ESP32 trouvé sur les ports série"
    echo ""
    echo "Ports disponibles :"
    ls /dev/cu.* 2>/dev/null
    echo ""
    echo "Veuillez spécifier le port manuellement :"
    echo "  screen <port> 500000"
    exit 1
fi

echo "✅ ESP32 trouvé sur : $ESP_PORT"
echo ""
echo "🎬 Démarrage de la capture..."
echo "   (Appuyez sur Ctrl+A puis K pour quitter screen)"
echo ""

# Créer le header du fichier
cat > "$CAPTURE_FILE" << EOF
# Capture BLE pour l'effet : $EFFECT_NAME
# Date : $(date)
# Port : $ESP_PORT
# Baudrate : 500000

## Notes
- Ajoutez vos observations ici
- Décrivez le comportement visuel de l'effet
- Notez les paramètres modifiés dans l'application

## Captures brutes

EOF

# Démarrer screen et rediriger vers le fichier
screen -L -Logfile "$CAPTURE_FILE.raw" "$ESP_PORT" 500000

echo ""
echo "✅ Capture terminée"
echo "📁 Fichiers créés :"
echo "   - $CAPTURE_FILE (avec header)"
echo "   - $CAPTURE_FILE.raw (données brutes)"
echo ""
echo "📊 Prochaines étapes :"
echo "  1. Ouvrez $CAPTURE_FILE"
echo "  2. Ajoutez vos observations"
echo "  3. Identifiez les patterns dans les payloads"
echo "  4. Documentez dans PROTOCOL_ANALYSIS.md"
