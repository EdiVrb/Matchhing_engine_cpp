# Matching Engine C++

**Date :** 28 mai 2025
**Rendu :** Dimanche 8 Juin 2025

##  Objectif du projet

Développer un *matching engine* performant en C++ capable de :

* Lire et désérialiser des ordres depuis un fichier CSV d’entrée.
* Maintenir un carnet d’ordres (order book) par instrument.
* Exécuter les ordres selon la priorité prix‐temps.
* Gérer les actions **NEW**, **MODIFY**, **CANCEL** et les types **LIMIT** et **MARKET**.
* Sérialiser le résultat dans un CSV de sortie.

##  Fonctionnalités clés

1. **Parsing CSV** : lecture ligne par ligne avec `CSVReader`.
2. **Carnet d’ordres** : structures `BookSide`, `PriceLevel`, `OrderBook`.
3. **Matching** : `OrderMatcher` gère les ordres LIMIT et MARKET, multi‐niveaux, priorité prix‐temps.
4. **Gestion des ordres** : création, modification, annulation (`InstrumentManager` / `MatchingEngine`).
5. **Événements** : collecte de `OrderEvent` pour état (PENDING, EXECUTED, PARTIALLY\_EXECUTED, CANCELED).
6. **Output CSV** : `CSVWriter` génère le fichier de sortie avec tous les événements.

##  Prérequis

* **Compilateur C++17** (ou supérieur).
* **CMake** (>= 3.10).
* **GoogleTest** pour les tests unitaires.

## 🛠️ Compilation et exécution

Ouvrez un terminal dans le répertoire racine du projet :

```bash
# Nettoyer puis créer un dossier de build
rm -rf build && mkdir build && cd build

# Générer les Makefiles
cmake ..

# Compiler (4 jobs simultanés)
make -j4

# Lancer votre engine sur un CSV d'exemple\
./matching_engine ../data/input_cpp_project.csv output.csv
```

##  Exécuter les tests

Depuis `build` :

```bash
# Lancer tous les tests et afficher le détail
ctest --verbose
```

Ou lancer individuellement :

```bash
./test_order
./test_market_orders
./test_order_matcher
./test_matching_engine
./test_performance
```

##  Structure du dépôt

```
├── CMakeLists.txt
├── data/                  # CSV d'exemple
├── include/               # En‐têtes (core, io, types, utils)
├── src/                   # Implémentations
├── tests/                 # Tests unitaires GoogleTest
├── build/                 # Répertoire de build (gitignored)
└── README.md              # Ce fichier
```

##  Contribution

1. Clonez le dépôt : `git clone <url>`
2. Créez une branche : `git checkout -b feature/ma-fonctionnalité`
3. Codez, testez, puis ouvrez une pull request.

##  Licence

Ce projet est sous licence MIT. Voir le fichier [LICENSE](LICENSE).

---



##  Tests unitaires et gestion des erreurs

### Tests unitaires couverts

* **test\_Order.cpp** : création, exécution partielle, exécution complète et annulation d’instances `Order`.
* **test\_MarketOrders.cpp** :

  * Vérifie le prix forcé à zéro pour MARKET.
  * Exécution complète, partielle puis annulation d’un ordre MARKET.
  * Scénarios multi-niveaux et absence de liquidité.
  * Interdiction de modifier un ordre MARKET.
* **test\_OrderMatcher.cpp** :

  * Matching LIMIT vs LIMIT avec priorité prix-temps.
  * Matching MARKET sur plusieurs niveaux.
  * Comportement lorsque les prix ne se croisent pas.
  * Annulation automatique du reliquat pour MARKET.
* **test\_MatchingEngine.cpp** :

  * Traitement des actions NEW, MODIFY, CANCEL.
  * Lancement de matching après modification.
  * Génération d’événements (PENDING, EXECUTED, PARTIALLY\_EXECUTED, CANCELED).
* **test\_performance.cpp** : mesure de la latence de traitement sur 100 000 ordres.

### Gestion des erreurs

* **InvalidOrderException** lancé si :

  * `orderId` invalide ou déjà existant pour NEW.
  * Tentative de MODIFY ou CANCEL sur un ordre absent.
  * Modification d’un ordre MARKET.
* **OrderNotFoundException** pour tout MODIFY/CANCEL sans ordre correspondant.
* Tout ordre MARKET sans contrepartie disponible est immédiatement annulé.

---
