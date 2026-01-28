# Analyse des captures logcat - Couleurs RGB

## Date : 28 janvier 2026

## Captures effectuées

### Tests RGB

#### Test 1 : Rouge pur
```
Command: ff00ff000000
Payload: 7201ff00ff00000000000000
Key: 56219394
```

#### Test 2 : Vert pur
```
Command: ff0000ff0000
Payload: 7201ff0000ff000000000000
Key: 56219394
```

#### Test 3 : Bleu pur
```
Command: ffff00000000
Payload: 7201ffff0000000000000000
Key: 56219394
```

### Tests Blancs

#### Test 4 : Blanc chaud
```
Command: ff000000ff00
Payload: 7201ff000000ff0000000000
Key: 56219394
```

#### Test 5 : Blanc neutre (mélange)
```
Command: ff0000007a85
Payload: 7201ff0000007a8500000000
Key: 56219394
```

#### Test 6 : Blanc froid
```
Command: ff00000000ff
Payload: 7201ff00000000ff00000000
Key: 56219394
```

## Analyse détaillée

### Structure du payload complet

```
72 01 ff 00 ff 00 00 00 00 00 00 00
│  │  │  │  │  │  │  │  │  │  │  │
│  │  └──┴──┴──┴──┴──┴──┴──┴──┴──┴─ Données lumière (10 octets)
│  └─ Light ID (0x01 = Lumière 1)
└─ Type/Commande (0x72)
```

### Structure des données lumière (10 octets)

| Position | Rouge | Vert | Bleu | Chaud | Neutre | Froid | Description |
|----------|-------|------|------|-------|--------|-------|-------------|
| **Octet 0** | `ff` | `ff` | `ff` | `ff` | `ff` | `ff` | **ON/OFF + Brightness** (0x80 + 0x7F = 0xFF) |
| **Octet 1** | `00` | `00` | `ff` | `00` | `00` | `00` | **Bleu** (0-255) |
| **Octet 2** | `ff` | `00` | `00` | `00` | `00` | `00` | **Rouge** (0-255) |
| **Octet 3** | `00` | `ff` | `00` | `00` | `00` | `00` | **Vert** (0-255) |
| **Octet 4** | `00` | `00` | `00` | `ff` | `7a` | `00` | **Blanc chaud** (0-255) |
| **Octet 5** | `00` | `00` | `00` | `00` | `85` | `ff` | **Blanc froid** (0-255) |
| **Octets 6-9** | `00 00 00 00` | `00 00 00 00` | `00 00 00 00` | `00 00 00 00` | `00 00 00 00` | `00 00 00 00` | **Réservés** (effets) |

## Confirmation de l'implémentation actuelle

✅ **Le code actuel est CORRECT !**

Notre implémentation dans `fastcon_controller.cpp` utilise déjà ce format :

```cpp
light_data[0] = 0x80 + brightness;  // ON/OFF + brightness
light_data[1] = blue;                // Bleu
light_data[2] = red;                 // Rouge
light_data[3] = green;               // Vert
light_data[4] = warm_white;          // Blanc chaud
light_data[5] = cold_white;          // Blanc froid
```

## Observations importantes

### 1. Format du command (getCmd)

Le format dans `getCmd` reste cohérent avec le payload :
```
ff    00  ff  00  00  00
│     │   │   │   │   │
ON+Br B   R   G   Warm Cold
```

### 2. Blanc neutre = Mélange chaud + froid

Le blanc neutre utilise un **mélange des deux canaux** :
```
Blanc neutre: 7a 85
              │  │
              │  └─ 0x85 = 133 (Blanc froid)
              └─ 0x7a = 122 (Blanc chaud)
```

Cela confirme que la température de couleur est contrôlée par le **ratio** entre warm et cold white !

### 3. Formule de température de couleur

```
Warm% = warm / (warm + cold)
Cold% = cold / (warm + cold)

Exemple blanc neutre :
Warm% = 122 / (122 + 133) = 47.8%
Cold% = 133 / (122 + 133) = 52.2%
```

### 2. Payload vs Command

- **Command** : Format d'entrée de l'application
- **Payload** : Format encodé pour BLE après ajout du header

### 3. Octets réservés (6-9)

Les octets 6-9 sont actuellement à `00` mais pourraient être utilisés pour :
- **Effets spéciaux** (ID d'effet, paramètres)
- **Timing** (durée, vitesse)
- **Modes spéciaux**

## Prochaines captures nécessaires

### 🎯 Effets à capturer

Pour identifier le protocole des effets, il faut maintenant capturer :

1. **Mode Flash/Strobe**
   ```bash
   adb logcat | grep "jyq_helper: getPayloadWithInnerRetry"
   # Activer l'effet Flash dans l'app
   ```

2. **Mode Rainbow/Arc-en-ciel**
3. **Mode Fade/Fondu**
4. **Mode Pulse/Pulsation**
5. **Autres effets disponibles**

### Commande pour capturer

```bash
# Capturer en continu
adb logcat | grep "jyq_helper: getPayloadWithInnerRetry" > captures/logcat_effects.txt

# Pendant la capture :
# 1. Activer chaque effet dans l'app
# 2. Noter l'heure et le nom de l'effet
# 3. Attendre 5-10 secondes
# 4. Passer à l'effet suivant
```

## Hypothèses sur les effets

### Hypothèse 1 : Octets supplémentaires

Les effets pourraient utiliser les octets 6-9 :

```
Payload normal: 72 01 ff 00 ff 00 00 00 | 00 00 00 00
Payload effet:  72 01 ff 00 ff 00 00 00 | 01 80 00 00
                                          │  │  │  │
                                          │  │  └──┴─ Paramètres
                                          │  └─ Speed/Vitesse
                                          └─ Effect ID
```

### Hypothèse 2 : Type de commande différent

```
Normal: 72 01 ... (type 0x72)
Effet:  73 01 ... (type 0x73 ?)
```

### Hypothèse 3 : Format étendu

```
Payload normal: 12 octets total
Payload effet:  14-16 octets total ?
```

## Format de documentation pour les prochaines captures

```markdown
### Effet : [Nom]

**Commande (getCmd)** : [hex]
**Payload** : [hex]
**Séquence** : [numéro]

**Observations** :
- Comportement visuel : [description]
- Différence avec normal : [octets qui changent]
```

## Comparaison avec l'implémentation

### Code actuel (fastcon_controller.cpp)

```cpp
std::vector<uint8_t> light_data = {
    0x80 + brightness,  // ✅ Correspond à l'octet 0
    blue,               // ✅ Correspond à l'octet 1
    red,                // ✅ Correspond à l'octet 2
    green,              // ✅ Correspond à l'octet 3
    warm_white,         // ✅ Correspond à l'octet 4
    cold_white,         // ✅ Correspond à l'octet 5
    // Octets 6-9 non utilisés actuellement
};
```

### Taille du vecteur

- **Actuel** : 6 octets (indices 0-5)
- **Payload brut** : 10 octets (indices 0-9)
- **Manquant** : 4 octets (indices 6-9)

⚠️ **Note** : Les octets 6-9 sont probablement ignorés/remplis à 0 par le protocole actuel, mais ils seront nécessaires pour les effets.

## Actions à effectuer

### Immédiat

1. ✅ Confirmer que le format RGB est correct (FAIT)
2. 🔄 Capturer les effets spéciaux avec logcat
3. ⏳ Analyser les payloads des effets

### Code à modifier (une fois effets capturés)

1. **Ajuster la taille de light_data** si nécessaire
2. **Ajouter les octets d'effet** (6-9)
3. **Implémenter le mapping** effet → codes

## Références

- **Log source** : Capture logcat Android du 28/01/2026
- **Application** : Broadlink BLE (brMesh)
- **Key utilisée** : 56219394
- **Light ID** : 01

## Prochaine étape

🎯 **Capturer un effet (Flash/Rainbow/etc.)** et documenter ici pour comparer avec les payloads normaux.

Commande recommandée :
```bash
adb logcat -c  # Clear logs
adb logcat | grep -E "jyq_helper|jyq_device" | tee captures/logcat_effects_$(date +%H%M%S).txt
```

Puis activer différents effets dans l'app et observer les changements de payload.
