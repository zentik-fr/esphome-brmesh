# Guide de capture des effets via ADB logcat

## ✅ Avantage de cette méthode

Au lieu d'utiliser le sniffer BLE ESP32, nous pouvons capturer directement depuis l'application Android !

### Avantages
- ✅ Plus simple (pas besoin d'ESP32)
- ✅ Données déjà décodées (payload en clair)
- ✅ Informations de debug de l'app
- ✅ Plus rapide

## 📱 Prérequis

1. **Téléphone Android** avec l'app Broadlink BLE installée
2. **ADB** installé sur votre Mac
3. **Mode développeur** activé sur le téléphone
4. **Débogage USB** activé

### Installation ADB (si nécessaire)

```bash
# Avec Homebrew
brew install android-platform-tools

# Vérifier l'installation
adb version
```

### Activer le débogage USB

1. Aller dans **Paramètres** → **À propos du téléphone**
2. Taper 7 fois sur **Numéro de build**
3. Retour → **Options développeur**
4. Activer **Débogage USB**
5. Brancher le téléphone au Mac
6. Accepter l'autorisation sur le téléphone

## 🎬 Procédure de capture

### Étape 1 : Préparer la capture

```bash
# Vérifier la connexion
adb devices

# Devrait afficher :
# List of devices attached
# [ID_DU_TELEPHONE]    device

# Nettoyer les logs précédents
adb logcat -c

# Démarrer la capture
adb logcat | grep -E "jyq_helper|jyq_device" | tee captures/effects_$(date +%Y%m%d_%H%M%S).txt
```

### Étape 2 : Activer les effets

Dans l'application Broadlink BLE :

1. **Ouvrir** l'app
2. **Sélectionner** un spot LED
3. **Activer un effet** (ex: Rainbow)
4. **Attendre** 3-5 secondes
5. **Noter** dans le terminal : `# Effet: Rainbow activé`
6. **Passer** à l'effet suivant

### Étape 3 : Arrêter la capture

```bash
# Appuyer sur Ctrl+C pour arrêter
# Le fichier est sauvegardé automatiquement
```

## 📊 Format des logs capturés

### Exemple de ligne intéressante

```
I jyq_helper: getPayloadWithInnerRetry---> payload:7201ff00ff00000000000000,  key: 56219394
```

### Décodage

```
payload: 72 01 ff 00 ff 00 00 00 00 00 00 00
         │  │  └─────────────────────────┘
         │  │  Données lumière (10 octets)
         │  └─ Light ID
         └─ Type/Commande
```

## 🎨 Effets à capturer

Créez un tableau pour suivre vos captures :

| Effet | Capturé | Payload | Différence | Notes |
|-------|---------|---------|------------|-------|
| Normal (RGB) | ✅ | `7201ff00ff00000000000000` | - | Rouge pur |
| Flash/Strobe | ⬜ | | | |
| Rainbow | ⬜ | | | |
| Fade | ⬜ | | | |
| Pulse | ⬜ | | | |
| Smooth | ⬜ | | | |
| ... | ⬜ | | | |

## 📝 Template de documentation

Pour chaque effet capturé, créez une section :

```markdown
### Effet : [Nom de l'effet]

**Horodatage** : [HH:MM:SS]

**Payload capturé** :
```
I jyq_helper: getPayloadWithInnerRetry---> payload:XXXXXXXXXXX,  key: 56219394
```

**Décodage** :
```
72 01 [données...]
```

**Comportement visuel** :
[Description de ce que fait l'effet]

**Différence avec RGB normal** :
- Octet X : changé de 0x00 à 0xYY
- Octets supplémentaires : [oui/non]
```

## 🔍 Ce qu'on cherche

### 1. Octets supplémentaires

```
Normal: 72 01 ff 00 ff 00 00 00 00 00 00 00 (10 octets de données)
Effet:  72 01 ff 00 ff 00 00 00 01 80 XX XX (octets 6-9 utilisés ?)
                                  ↑  ↑  ↑  ↑
                              Effect Speed Params
```

### 2. Type de commande différent

```
Normal: 72 01 ... (type 0x72)
Effet:  XX 01 ... (type différent ?)
```

### 3. Format de payload différent

```
Normal: 12 octets total
Effet:  Plus de 12 octets ?
```

## 🛠️ Commandes utiles

### Capturer seulement les payloads

```bash
adb logcat | grep "getPayloadWithInnerRetry" | cut -d':' -f4 | tee captures/payloads_only.txt
```

### Capturer et afficher en temps réel

```bash
adb logcat | grep --color=always "getPayloadWithInnerRetry"
```

### Capturer dans un fichier avec horodatage

```bash
adb logcat -v time | grep "jyq_helper" > captures/full_log_$(date +%Y%m%d_%H%M%S).txt
```

### Comparer deux captures

```bash
diff captures/effect_normal.txt captures/effect_rainbow.txt
```

## ⚠️ Troubleshooting

### Problème : `adb: command not found`

**Solution** :
```bash
brew install android-platform-tools
```

### Problème : `no devices/emulators found`

**Solutions** :
1. Vérifier que le téléphone est branché
2. Accepter l'autorisation de débogage USB
3. Essayer un autre câble USB
4. Redémarrer le serveur ADB :
   ```bash
   adb kill-server
   adb start-server
   adb devices
   ```

### Problème : Aucun log visible

**Solutions** :
1. Vérifier que l'app Broadlink BLE est ouverte
2. Activer/désactiver une lumière pour générer des logs
3. Vérifier les filtres grep

### Problème : Trop de logs

**Solutions** :
```bash
# Filtrer plus précisément
adb logcat | grep "payload:" | grep "jyq_helper"

# Ou exclure certains tags
adb logcat | grep -v "irrelevant_tag"
```

## 📈 Analyse après capture

### 1. Organiser les données

```bash
# Créer un fichier par effet
grep "Rainbow" captures/effects_full.txt > captures/effect_rainbow.txt
grep "Strobe" captures/effects_full.txt > captures/effect_strobe.txt
```

### 2. Extraire les payloads

```bash
# Extraire uniquement les payloads
grep "payload:" captures/effect_rainbow.txt | cut -d':' -f4 | cut -d',' -f1
```

### 3. Comparer

```bash
# Afficher côte à côte
paste captures/payload_normal.txt captures/payload_rainbow.txt
```

## ✅ Checklist complète

- [ ] ADB installé et fonctionnel
- [ ] Téléphone connecté et autorisé
- [ ] App Broadlink BLE ouverte
- [ ] Spot LED connecté et fonctionnel
- [ ] Capture de l'état normal (RGB) effectuée
- [ ] Pour chaque effet :
  - [ ] Effet activé dans l'app
  - [ ] Payload capturé
  - [ ] Comportement noté
  - [ ] Différences identifiées

## 🎯 Objectif final

Une fois tous les effets capturés et analysés, nous pourrons :

1. ✅ Identifier le format exact des effets
2. ✅ Compléter le code dans `fastcon_controller.cpp`
3. ✅ Mapper les effets ESPHome vers les codes BRMesh
4. ✅ Tester et valider

## 📞 Prochaines étapes

1. **Capturer** au moins 2-3 effets différents
2. **Documenter** dans `captures/analysis_effects.md`
3. **Analyser** les patterns
4. **Implémenter** le support dans le code

---

**Temps estimé** : 30-60 minutes pour capturer tous les effets disponibles

**Difficulté** : Facile 🟢

Bon courage ! 🚀
