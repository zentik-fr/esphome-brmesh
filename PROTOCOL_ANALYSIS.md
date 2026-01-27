# Analyse du protocole BRMesh - Effets spéciaux

## Structure des données actuellement connues

D'après le code existant et l'implémentation brMeshMQTT, voici la structure des commandes :

### Format des commandes de base

```
Octet 0: 0x80 + intensité (0-127)  # Bit 7 = ON/OFF, bits 0-6 = brightness
Octet 1: Bleu (0-255)
Octet 2: Rouge (0-255)  
Octet 3: Vert (0-255)
Octet 4: Blanc chaud (0-255)
Octet 5: Blanc froid (0-255)
```

## Recherche des effets spéciaux

### Hypothèses sur l'implémentation des effets

Les spots LED BRMesh pourraient avoir des effets natifs encodés de plusieurs façons :

1. **Via un octet spécial dans les données de commande** (non découvert actuellement)
2. **Via un type de commande différent** (le paramètre `n` dans `generate_command`)
3. **Via une séquence spéciale de valeurs RGB**

### Type de commande actuel

Dans `FastconController::single_control()`, le type de commande utilisé est `5` :
```cpp
return this->generate_command(5, light_id_, result_data, true);
```

Le paramètre `n` (type) pourrait avoir d'autres valeurs pour différentes fonctions :
- `n = 5` : Contrôle simple de la lumière (actuel)
- `n = ?` : Effets spéciaux (à découvrir)

## Plan pour découvrir les effets

### Méthode 1 : Sniffing BLE avec l'application mobile

1. **Activer le sniffer BLE** :
   ```bash
   # Compiler et uploader ble_sniffer_esp32_arduino.ino sur un ESP32
   # Monitorer la sortie série à 500000 baud
   ```

2. **Capturer les paquets** :
   - Ouvrir l'application brMesh/Broadlink BLE
   - Activer différents effets sur les spots
   - Noter les payloads BLE capturés pour chaque effet

3. **Analyser les différences** :
   - Comparer avec les commandes de couleur/brightness normales
   - Identifier les octets qui changent
   - Déduire le format des commandes d'effet

### Méthode 2 : Recherche documentaire

Chercher dans :
- Issues GitHub du projet brMeshMQTT
- Reverse engineering du firmware des spots
- Applications mobiles décompilées

### Méthode 3 : Test systématique

Tester différentes valeurs pour :
- Le type de commande (`n` dans generate_command)
- Les octets supplémentaires dans light_data
- Les combinaisons de valeurs RGB spéciales

## Prochaines étapes

1. **Utiliser le sniffer BLE** pour capturer les paquets quand vous activez un effet
2. **Documenter les payloads** capturés ici
3. **Implémenter le support des effets** dans le code C++

## Format potentiel des effets (à confirmer)

```cpp
// Hypothèse : structure étendue pour les effets
std::vector<uint8_t> light_data = {
    0x80 + brightness,  // Octet 0 : ON/OFF + brightness
    effect_id,          // Octet 1 : ID de l'effet (0 = pas d'effet)
    speed,              // Octet 2 : Vitesse de l'effet
    ...                 // Autres paramètres
};
```

---

## Instructions pour la capture

### Utilisation du sniffer BLE

1. **Upload le sniffer** :
   ```bash
   cd ble_sniffer_esp32_arduino
   # Compiler et uploader via Arduino IDE ou platformio
   ```

2. **Monitorer les paquets** :
   ```bash
   # Connecter le moniteur série à 500000 baud
   # Les paquets avec Company ID 0xFFF0 seront affichés
   ```

3. **Activer les effets** :
   - Ouvrir l'app brMesh
   - Tester chaque effet disponible
   - Noter le payload pour chaque effet

4. **Documenter ici** :
   ```
   Effet: [Nom de l'effet]
   Payload: [Hex dump complet]
   Notes: [Observations]
   ```
