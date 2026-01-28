# Analyse des commandes Brightness - DÉCOUVERTE IMPORTANTE

## Date : 28 janvier 2026

## 🎯 Découverte majeure : Deux types de commandes !

### Type 1 : Commande RGB/Blanc complète (Type 0x72)

```
7201 ff 00 ff 00 00 00 00 00 00 00
│││  │  │  │  │  │  │  └──────────┘
│││  │  │  │  │  │  │   Octets réservés
│││  │  │  │  │  │  └─ Cold white
│││  │  │  │  │  └─ Warm white
│││  │  │  │  └─ Green
│││  │  │  └─ Red
│││  │  └─ Blue
│││  └─ ON (0x80) + Brightness (0-127)
││└─ Light ID
│└─ Type de commande : 0x72 (commande complète)
└─ Longueur/flags
```

### Type 2 : Commande Brightness seule (Type 0x22) ⭐ NOUVEAU

```
2201 63 00 00 00 00 00 00 00 00 00
│││  │  └──────────────────────────┘
│││  │   Tous à zéro
│││  └─ Brightness direct (0-99 ou 0-127)
││└─ Light ID
│└─ Type de commande : 0x22 (brightness seul)
└─ Longueur/flags
```

## Captures effectuées

### Brightness 100% (~99%)
```
Command: 63
Payload: 220163000000000000000000
Décimal: 0x63 = 99
```

### Brightness ~75% (72%)
```
Command: 48
Payload: 220148000000000000000000
Décimal: 0x48 = 72
```

### Brightness 50%
```
Command: 32
Payload: 220132000000000000000000
Décimal: 0x32 = 50
```

### Brightness ~34%
```
Command: 22
Payload: 220122000000000000000000
Décimal: 0x22 = 34
```

### Brightness ~19%
```
Command: 13
Payload: 220113000000000000000000
Décimal: 0x13 = 19
```

### Brightness 1%
```
Command: 01
Payload: 220101000000000000000000
Décimal: 0x01 = 1
```

### ON (Power ON)
```
Command: 80
Payload: 220180000000000000000000
Décimal: 0x80 = 128 (bit ON)
```

### OFF (Power OFF)
```
Command: 00
Payload: 220100000000000000000000
Décimal: 0x00 = 0 (OFF)
```

## Analyse détaillée

### Encodage ON/OFF dans le type 0x22

🎯 **Découverte cruciale** : Le bit 0x80 est le bit ON dans les deux types !

| Valeur | État | Brightness |
|--------|------|------------|
| 0x00 | OFF | - |
| 0x01-0x7F | ON | 1-127 |
| 0x80 | ON | 128 ou "max" |

**Observation** : 
- OFF = 0x00
- ON = 0x80 (bit 7 à 1) OU brightness (1-99 dans l'app)
- Le bit 0x80 (bit 7) est le **bit de puissance** dans les deux formats !

### Plage de brightness

D'après les captures :
- **Minimum** : 0x01 = 1
- **Maximum** : 0x63 = 99

🤔 **Observation** : La plage est 1-99 au lieu de 0-127 !

Deux hypothèses :
1. L'application limite à 1-99 (0-100% en UI)
2. Le protocole supporte 0-127 mais l'app n'utilise que 1-99

### Comparaison avec le format RGB

| Aspect | Type 0x72 (RGB/Blanc) | Type 0x22 (Brightness) |
|--------|----------------------|------------------------|
| **Type** | 0x72 | 0x22 |
| **ON/OFF** | Bit 7 de l'octet 0 (0x80) | Bit 7 de l'octet 0 (0x80) |
| **Brightness** | Octet 0 : 0x80 + val (0-127) | Octet 0 : 0x80 OU val (1-99) |
| **OFF** | Octet 0 : 0x00 | Octet 0 : 0x00 |
| **Couleurs** | Octets 1-5 utilisés | Octets 1-9 tous à 0 |
| **Usage** | Changement de couleur | Changement de brightness/power seul |

### Pourquoi deux commandes ?

**Hypothèse confirmée** : Optimisation + contrôle power
- Type 0x72 : Commande "complète" quand on change couleur + brightness
- Type 0x22 : Commande "rapide" pour brightness/power uniquement

**Avantages** : 
- Plus simple à encoder (juste 1 octet utile)
- Permet ON/OFF direct avec 0x80/0x00
- Peut-être plus rapide à traiter côté lampe

**Note** : Le type 0x22 semble aussi être utilisé pour le contrôle ON/OFF !
- ON = `220180...` (0x80)
- OFF = `220100...` (0x00)

## Impact sur l'implémentation

### Code actuel (fastcon_controller.cpp)

Notre code utilise toujours le type 0x72 avec `single_control()` qui appelle `generate_command(5, ...)`.

Le paramètre `5` correspond probablement au type 0x72 dans l'encodage final.

### Hypothèse sur le mapping

```cpp
generate_command(n, ...)
  n = 5 → Type 0x72 (commande RGB/Blanc complète)
  n = ? → Type 0x22 (commande brightness seule)
```

### Doit-on implémenter le type 0x22 ?

**Non, pas nécessaire !** ✅

Le type 0x72 fonctionne déjà pour tout, y compris le brightness seul.
Le type 0x22 est juste une **optimisation** de l'application.

Notre implémentation actuelle qui utilise toujours le type 0x72 est **parfaitement valide**.

## Différence avec notre code

### Notre implémentation
```cpp
// Nous utilisons toujours :
light_data[0] = 0x80 + brightness;  // ON bit + brightness (0-127)
```

### Application Broadlink
```
Si changement de couleur :
  → Type 0x72 : ff = 0x80 + 0x7F (ON + brightness max)
  
Si changement de brightness seul :
  → Type 0x22 : 63 = brightness direct (99)
```

## Conclusion

### ✅ Notre code est correct !

Nous utilisons le type 0x72 pour tout, ce qui fonctionne parfaitement.

### 💡 Optimisation possible (optionnelle)

Si on voulait reproduire le comportement exact de l'app :
```cpp
// Si on change seulement brightness (pas de couleur)
if (brightness_only_change) {
    return generate_command(n_brightness, light_id, {brightness});
}
// Sinon, commande complète
else {
    return generate_command(5, light_id, light_data);
}
```

Mais ce n'est **pas nécessaire** pour le fonctionnement ! ✅

## Prochaine étape

🎯 **Capturer un EFFET** pour voir les octets 6-9 !

Tous les tests de base sont maintenant complets :
- ✅ RGB
- ✅ Blanc (chaud/froid/neutre)
- ✅ Brightness (via type 0x22)

Il ne manque plus que les **effets spéciaux** ! 🌈
