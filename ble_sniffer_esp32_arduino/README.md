# BRMesh BLE Sniffer

## Description

Ce sniffer BLE capture les paquets publicitaires BLE avec Company ID `0xFFF0` (Fastcon/BRMesh) pour analyser le protocole des effets spéciaux.

## Matériel requis

- ESP32 (n'importe quelle carte)
- Câble USB
- Arduino IDE ou PlatformIO

## Installation

### Avec Arduino IDE

1. Ouvrir `ble_sniffer_esp32_arduino.ino`
2. Sélectionner votre carte ESP32 (Tools > Board)
3. Sélectionner le port série (Tools > Port)
4. Compiler et uploader (Ctrl+U)

### Avec PlatformIO

```bash
# Depuis le dossier racine du projet
pio run -t upload -e esp32dev
```

## Utilisation

### Méthode 1 : Script automatique (recommandé)

```bash
# Depuis le dossier racine
./ble_capture.sh rainbow  # Remplacer "rainbow" par le nom de l'effet
```

Le script va :
- Trouver automatiquement le port ESP32
- Créer un fichier de capture
- Lancer le moniteur série
- Sauvegarder les données

### Méthode 2 : Manuelle

1. **Connecter le moniteur série** :
   ```bash
   screen /dev/cu.usbserial-* 500000
   # ou
   screen /dev/cu.SLAB_USBtoUART 500000
   ```

2. **Ouvrir l'application brMesh** sur votre téléphone

3. **Activer un effet** sur un spot LED

4. **Observer les paquets** dans la console :
   ```
   🎯 MATCH F0 FF - 25
   .                    RAW: 02 01 1A 1B FF F0 FF ...
   ------------------
   ```

5. **Noter le payload** complet

6. **Quitter** : Ctrl+A puis K (avec screen)

## Format des données capturées

### Structure du paquet BLE

```
[Length] [Type] [Data...]
02       01      1A              # Flags
1B       FF      F0 FF [DATA]    # Manufacturer Data (Company ID 0xFFF0)
```

### Exemple de capture

```
Effet: OFF
RAW: 02 01 1A 1B FF F0 FF 71 8E 4A A7 87 FA 6F 74 16 A7 05 69 35 48 79 6D A8 32 79 4C 57 37 00 49

Effet: ON (blanc, 50%)
RAW: 02 01 1A 1B FF F0 FF 71 8E 4A A7 87 FA 6F 74 16 A7 05 69 35 48 79 6D A8 32 79 4C 57 37 BF 22

Effet: Rainbow
RAW: 02 01 1A 1B FF F0 FF 71 8E 4A A7 87 FA 6F 74 16 A7 05 69 35 48 79 6D A8 32 79 4C 57 37 ?? ??
                                                                                              ↑↑ ↑↑
                                                                                      À analyser
```

## Conseils pour la capture

### 1. Captures à effectuer

Pour chaque effet disponible, capturez :

- **État de base** : Lumière allumée, blanc, 50%
- **Effet activé** : L'effet que vous voulez analyser
- **Variations** : Changement de vitesse, couleur, etc.

### 2. Effets à tester

Liste probable des effets dans brMesh (à confirmer) :
- ⭕ Normal (pas d'effet)
- 🌈 Rainbow (arc-en-ciel)
- ⚡ Strobe (flash)
- 🌊 Fade (fondu)
- 💫 Pulse (pulsation)
- 🔄 Rotate (rotation de couleurs)
- 🎪 Party (fête)
- etc.

### 3. Que noter pour chaque effet

```markdown
## Effet: [Nom]

### Comportement visuel
[Description de ce que fait l'effet]

### Paramètres modifiables
- Vitesse : Oui/Non
- Couleurs : Oui/Non
- Luminosité : Oui/Non

### Payload capturé
[Hex dump complet]

### Observations
[Octets qui changent, patterns observés]
```

## Analyse des captures

### Étape 1 : Comparer les payloads

Utilisez un comparateur de texte pour voir les différences :

```bash
# Comparer deux captures
diff capture_normal.txt capture_rainbow.txt
```

### Étape 2 : Identifier les patterns

Cherchez :
- **Octets constants** : Identifient le type de message
- **Octets variables** : Paramètres de l'effet
- **Octets ajoutés** : Extension du protocole pour les effets

### Étape 3 : Déduire la structure

Exemple hypothétique :

```
Octet 0-14 : Header BLE standard
Octet 15-17 : Préfixe BRMesh (71 8E 4A)
Octet 18-20 : Adresse/Séquence
Octet 21-26 : Données lumière standard (6 octets)
Octet 27    : Effect ID (si présent)
Octet 28    : Effect Speed (si présent)
Octet 29-30 : Checksum
```

## Fichiers de sortie

Les captures sont sauvegardées dans `captures/` :

```
captures/
├── effect_rainbow_20260127_143022.txt      # Capture avec notes
├── effect_rainbow_20260127_143022.txt.raw  # Données brutes
├── effect_strobe_20260127_143156.txt
└── ...
```

## Dépannage

### Problème : Aucun paquet capturé

**Solutions** :
- Vérifier que l'ESP32 est bien connecté et uploadé
- Vérifier que le baudrate est 500000
- Rapprocher l'ESP32 du spot LED
- Vérifier que le spot est bien en mode BRMesh (pas brLight)

### Problème : Trop de paquets

**Solutions** :
- Modifier `RSSI_MIN` dans le code pour filtrer les signaux faibles
- Éteindre les autres appareils BLE
- Utiliser un mode filtré

### Problème : Port série introuvable

**Sur macOS** :
```bash
ls /dev/cu.*
# Chercher usbserial, SLAB_USBtoUART, ou wchusbserial
```

**Sur Linux** :
```bash
ls /dev/ttyUSB*
ls /dev/ttyACM*
```

## Prochaines étapes

Une fois les captures effectuées :

1. 📝 Documentez dans [PROTOCOL_ANALYSIS.md](../PROTOCOL_ANALYSIS.md)
2. 🔍 Analysez les patterns
3. 💻 Implémentez dans le code (voir [EFFECTS_IMPLEMENTATION_GUIDE.md](../EFFECTS_IMPLEMENTATION_GUIDE.md))
4. ✅ Testez et validez

## Ressources

- [Guide d'implémentation complet](../EFFECTS_IMPLEMENTATION_GUIDE.md)
- [Analyse du protocole](../PROTOCOL_ANALYSIS.md)
- [Modifications du code](../CHANGES.md)
- [Projet brMeshMQTT](https://github.com/ArcadeMachinist/brMeshMQTT)
- [Reverse engineering par Moody](https://mooody.me/posts/2023-04/reverse-the-fastcon-ble-protocol/)
