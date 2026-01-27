# Guide d'implémentation des effets BRMesh

## État actuel

✅ Le code est maintenant **préparé** pour supporter les effets
🔍 Le protocole des effets doit être **découvert** via sniffing BLE
⏳ L'implémentation finale sera faite une fois le protocole identifié

## Structure préparée

### 1. Nouvelle structure `LightEffect` ([fastcon_light.h](components/fastcon/fastcon_light.h))

```cpp
struct LightEffect {
    uint8_t effect_id{0};     // 0 = no effect, 1+ = effect codes
    uint8_t speed{128};       // Effect speed (0-255)
    uint8_t param1{0};        // Additional parameter 1
    uint8_t param2{0};        // Additional parameter 2
};
```

### 2. Méthode modifiée `get_light_data()` ([fastcon_controller.cpp](components/fastcon/fastcon_controller.cpp))

La méthode accepte maintenant un paramètre `effect` optionnel et logue les tentatives d'utilisation d'effets.

### 3. Détection des effets dans `write_state()` ([fastcon_light.cpp](components/fastcon/fastcon_light.cpp))

Le code détecte maintenant quand un effet est demandé et le logue pour debug.

## Prochaines étapes

### Étape 1 : Capture des paquets BLE

#### A. Préparer le matériel

```bash
# 1. Uploader le sniffer BLE sur un ESP32
cd ble_sniffer_esp32_arduino
# Compiler et uploader via Arduino IDE ou PlatformIO

# 2. Connecter le moniteur série à 500000 baud
```

#### B. Capturer les effets

Pour chaque effet disponible dans l'application brMesh :

1. **Allumer la lumière** en mode normal
2. **Capturer le payload de base**
3. **Activer l'effet** dans l'app
4. **Capturer le nouveau payload**
5. **Noter les différences**

#### C. Format de documentation

Créez un fichier avec vos captures :

```markdown
## Effet: [Nom de l'effet]
### Payload sans effet
[Hex dump]

### Payload avec effet
[Hex dump]

### Différences observées
- Octet X : changé de 0x00 à 0x0F
- Octets supplémentaires : 3 octets ajoutés
```

### Étape 2 : Analyse des captures

Une fois les captures obtenues, cherchez :

#### A. Patterns communs

- **Position des octets d'effet** dans le payload
- **Valeurs constantes** (identifient le type de commande)
- **Valeurs variables** (paramètres de l'effet)

#### B. Questions à répondre

1. **Les effets utilisent-ils un type de commande différent ?**
   - Actuellement : `n = 5` dans `generate_command(5, ...)`
   - Peut-être : `n = 6` ou autre pour les effets ?

2. **Les effets ajoutent-ils des octets au payload ?**
   - Format actuel : 6 octets (ON/OFF+brightness, B, R, G, Warm, Cold)
   - Format effet : 6 + X octets supplémentaires ?

3. **Y a-t-il une liste d'effets prédéfinis ?**
   - Effect ID 1 = Rainbow ?
   - Effect ID 2 = Strobe ?
   - etc.

### Étape 3 : Implémentation

Une fois le protocole identifié, voici les modifications à faire :

#### A. Mapper les effets dans `write_state()` ([fastcon_light.cpp](components/fastcon/fastcon_light.cpp))

Décommenter et compléter la section :

```cpp
if (!effect_name.empty())
{
    if (effect_name == "Rainbow") effect.effect_id = 1;      // À découvrir
    else if (effect_name == "Strobe") effect.effect_id = 2;  // À découvrir
    else if (effect_name == "Fade") effect.effect_id = 3;    // À découvrir
    // ... autres effets
}
```

#### B. Encoder les effets dans `get_light_data()` ([fastcon_controller.cpp](components/fastcon/fastcon_controller.cpp))

Décommenter et adapter la section :

```cpp
if (effect.effect_id != 0)
{
    // Format découvert par sniffing :
    light_data.push_back(effect.effect_id);
    light_data.push_back(effect.speed);
    // Ajouter d'autres octets si nécessaire
}
```

#### C. Activer les effets dans `get_traits()` ([fastcon_light.cpp](components/fastcon/fastcon_light.cpp))

Décommenter et ajouter les noms d'effets découverts :

```cpp
traits.add_supported_effect("Rainbow");
traits.add_supported_effect("Strobe");
traits.add_supported_effect("Fade");
// ... autres effets supportés
```

#### D. Activer les effets dans le YAML ([brmesh.yaml](brmesh.yaml))

Décommenter et personnaliser :

```yaml
light:
  - platform: fastcon
    id: brmesh_lamp_1
    name: "Lamp1"
    light_id: 1
    effects:
      - strobe:
          name: "Strobe"
      - pulse:
          name: "Pulse"
          transition_length: 2s
      - random:
          name: "Random"
```

## Exemples d'effets ESPHome

Une fois implémentés, les effets apparaîtront dans Home Assistant comme des effets natifs.

### Types d'effets possibles

1. **Effets natifs BRMesh** (à découvrir)
   - Contrôlés par les spots LED directement
   - Probablement : Rainbow, Strobe, Fade, etc.

2. **Effets ESPHome** (déjà disponibles)
   - Contrôlés par l'ESP32
   - strobe, pulse, random, flicker, etc.

### Différence entre les deux

- **Effets natifs BRMesh** : Plus efficaces, exécutés par le spot LED
- **Effets ESPHome** : Plus flexibles, exécutés par l'ESP32, nécessitent des mises à jour fréquentes

## Debugging

### Logs utiles

Activez le niveau de log `DEBUG` ou `VERBOSE` dans votre YAML :

```yaml
logger:
  level: DEBUG
  logs:
    fastcon.light: VERBOSE
    fastcon.controller: VERBOSE
```

### Messages à surveiller

- `Effect active: [nom] (not yet implemented)` - Effet demandé mais pas encore implémenté
- `Effect requested: id=X, speed=Y` - Paramètres d'effet envoyés
- `Effects not yet implemented - protocol needs to be discovered` - Rappel que le protocole n'est pas encore connu

## Ressources

- **Sniffer BLE** : [ble_sniffer_esp32_arduino/](ble_sniffer_esp32_arduino/)
- **Analyse du protocole** : [PROTOCOL_ANALYSIS.md](PROTOCOL_ANALYSIS.md)
- **Référence brMeshMQTT** : https://github.com/ArcadeMachinist/brMeshMQTT
- **Reverse engineering** : https://mooody.me/posts/2023-04/reverse-the-fastcon-ble-protocol/

## Contact et contribution

Si vous découvrez le protocole des effets :

1. Documentez vos captures dans un fichier
2. Partagez vos découvertes (GitHub issue/PR)
3. Testez l'implémentation
4. Contribuez à améliorer ce guide
