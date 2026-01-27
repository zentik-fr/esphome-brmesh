# Captures d'effets BRMesh

Ce dossier contient les captures BLE des différents effets pour analyse.

## Organisation

Chaque fichier de capture doit suivre la nomenclature :
```
effect_<nom_effet>_<date>_<heure>.txt
```

## Utilisation

1. **Créer une nouvelle capture** :
   ```bash
   # Depuis le dossier racine
   ./ble_capture.sh <nom_effet>
   ```

2. **Documenter la capture** :
   - Copier `TEMPLATE.md` vers un nouveau fichier
   - Remplir toutes les sections
   - Coller les payloads capturés

3. **Analyser** :
   - Comparer avec les autres captures
   - Identifier les patterns
   - Documenter dans `../PROTOCOL_ANALYSIS.md`

## État des captures

| Effet | Capturé | Analysé | Implémenté | Notes |
|-------|---------|---------|------------|-------|
| Normal | ⬜ | ⬜ | ✅ | État de base |
| Rainbow | ⬜ | ⬜ | ⬜ | Arc-en-ciel |
| Strobe | ⬜ | ⬜ | ⬜ | Flash |
| Fade | ⬜ | ⬜ | ⬜ | Fondu |
| Pulse | ⬜ | ⬜ | ⬜ | Pulsation |
| ... | ⬜ | ⬜ | ⬜ | À identifier |

Légende :
- ✅ Fait
- 🔄 En cours
- ⬜ À faire

## Fichiers

- `TEMPLATE.md` : Template pour documenter une nouvelle capture
- `README.md` : Ce fichier

Les captures seront ajoutées automatiquement par le script `ble_capture.sh`.
