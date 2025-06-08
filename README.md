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
# Nettoyer puis créer un dossier de build s'il existe déjà
rm -rf build && mkdir build && cd build
# Sinon :
mkdir build && cd build


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
atching_engine_project6/
├── CMakeLists.txt
├── main.cpp
|
├── data/
│   └── input_cpp_project.csv       # Exemple de fichier CSV
│   └── output.csv       # Exemple de fichier CSV
├── include/
│   ├── core/
│   │   └── BookSide.hpp
│   │   └── InstrumentManager.hpp
│   │   └── MatchingEngine.hpp
│   │   └── Order.hpp
│   │   └── OrderBook.hpp
│   │   └── OrderEvent.hpp
│   │   └── OrderMatcher.hpp
│   │   └── PriceLevel.hpp
│   │   └── Trade.hpp
│   ├── io/
│   │   ├── CSVReader.h
│   │   └── CSVWriter.h
│   ├── types/
│   │   ├── Enums.hpp
│   │   └── OrderTypes.hpp
│   └── utils/
│   |   └── Logger.hpp
│   │   └── TimeUtils.hpp
├── src/
│   ├── core/
│   │   └── InstrumentManager.cpp
│   │   └── MatchingEngine.cpp
│   │   └── Order.cpp
│   │   └── OrderBook.cpp
│   │   └── OrderMatcher.cpp
│   │   └── PriceLevel.cpp
│   ├── io/
│   │   ├── CSVReader.cpp
│   │   └── CSVWriter.cpp
│   └── utils/
│   |   └── Logger.cpp
├── tests/
│   ├── test_Order.cpp
│   ├── test_MarketOrders.cpp
│   ├── test_OrderMatcher.cpp
│   ├── test_MatchingEngine.cpp
│   └── test_performance.cpp
├── build/                           # Répertoire de build (gitignored) => sera crée lors de la compilation
├── LICENSE                          # Licence MIT
└── README.md                        # Ce fichier
```

##  Contribution

1. Clonez le dépôt : `git clone <url>`
2. Créez une branche : `git checkout -b feature/ma-fonctionnalité`
3. Codez, testez, puis ouvrez une pull request.

---



##  Tests unitaires et gestion des erreurs

### Tests Unitaires

- **test_Order.cpp**  
  - Création d’un ordre valide et vérification de tous ses attributs.  
  - Lancement d’une exception `InvalidOrderException` si l’ID est invalide.  
  - Simulation d’exécutions partielles et totales (`execute`) et passage des statuts `PARTIALLY_EXECUTED` à `EXECUTED`.  
  - Annulation d’un ordre (`cancel`) et passage au statut `CANCELED`.

- **test_MarketOrders.cpp**  
  - Vérification que tout ordre MARKET a son prix forcé à 0.  
  - Exécution totale d’un ordre MARKET BUY et contrôle du prix d’exécution.  
  - Exécution partielle suivie d’une annulation pour un MARKET SELL sur plusieurs niveaux.  
  - Annulation immédiate si pas de contrepartie disponible.  
  - Interdiction de modifier un ordre MARKET (`InvalidOrderException`).  
  - Exécution multi-niveaux : agrégation de la quantité exécutée et vérification des prix par contrepartie.

- **test_OrderMatcher.cpp**  
  - Matching LIMIT vs LIMIT avec création d’un trade unique et statut `PARTIALLY_EXECUTED`.  
  - Matching MARKET vs plusieurs niveaux avec génération de plusieurs trades.  
  - Priorité prix-temps (meilleur prix puis ancienneté).  
  - Pas de trade si les prix ne se croisent pas et conservation de l’ordre `PENDING`.  
  - Annulation du reliquat pour MARKET (statut `CANCELED`).  
  - Annulation d’un MARKET sans liquidité (statut `CANCELED`).

- **test_MatchingEngine.cpp**  
  - Gestion des actions **NEW**, **MODIFY** et **CANCEL** : création, mise à jour et suppression d’ordres.  
  - Vérification du déclenchement d’une `OrderNotFoundException` sur `MODIFY`/`CANCEL` d’un ordre introuvable.  
  - Séquence `MODIFY` → matching : lancement automatique d’événements `EXECUTED`.  
  - Annulation d’un ordre partiellement exécuté (CANCEL sur reliquat).

- **test_performance.cpp**  
  - Lecture de 100 000 ordres LIMIT en moins de 10 minutes, validation du temps de traitement.

### Gestion des erreurs

* **InvalidOrderException** lancé si :

  * `orderId` invalide ou déjà existant pour NEW.
  * Tentative de MODIFY ou CANCEL sur un ordre absent.
  * Modification d’un ordre MARKET.
* **OrderNotFoundException** pour tout MODIFY/CANCEL sans ordre correspondant.
* Tout ordre MARKET sans contrepartie disponible est immédiatement annulé.

---
