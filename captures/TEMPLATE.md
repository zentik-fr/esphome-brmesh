# Template de capture d'effet BRMesh

## Informations générales

- **Date** : [Date de la capture]
- **Effet testé** : [Nom de l'effet]
- **Application utilisée** : brMesh / Broadlink BLE
- **Version de l'app** : [Si connue]

## Comportement visuel de l'effet

[Décrivez ce que fait l'effet visuellement]
- Couleurs utilisées : 
- Vitesse/rythme : 
- Pattern/séquence : 

## Paramètres modifiables dans l'application

- [ ] Vitesse/Speed
- [ ] Luminosité/Brightness  
- [ ] Couleurs
- [ ] Autres : 

## Captures BLE

### Capture 1 : État de référence (lumière normale)

**Conditions** :
- État : ON
- Couleur : Blanc
- Luminosité : 50%
- Effet : OFF

**Payload** :
```
[Coller le hex dump ici]
```

### Capture 2 : Effet activé (configuration par défaut)

**Conditions** :
- Effet : [Nom]
- Paramètres : Par défaut

**Payload** :
```
[Coller le hex dump ici]
```

### Capture 3 : Variation (optionnel)

**Conditions** :
- Effet : [Nom]
- Modification : [Ex: vitesse maximale]

**Payload** :
```
[Coller le hex dump ici]
```

## Analyse

### Comparaison des payloads

```
Réf:   02 01 1A 1B FF F0 FF [XX XX XX ...] YY ZZ
Effet: 02 01 1A 1B FF F0 FF [XX XX XX ...] YY ZZ
                              ↑           ↑
                              Octets identiques ou différents
```

### Octets identifiés

| Position | Valeur Réf | Valeur Effet | Interprétation |
|----------|------------|--------------|----------------|
| 0-14     | [Standard] | [Standard]   | Header BLE     |
| 15-20    | XX XX XX   | XX XX XX     | Préfixe BRMesh |
| ...      | ...        | ...          | ...            |

### Hypothèses

1. [Votre hypothèse sur la structure]
2. [Identification des octets d'effet]
3. [Pattern observé]

## Notes et observations

[Tout ce qui pourrait être utile]

## Prochaines étapes

- [ ] Tester d'autres variations de paramètres
- [ ] Comparer avec d'autres effets
- [ ] Valider l'hypothèse avec un test
