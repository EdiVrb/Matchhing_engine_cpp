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
1. test_performance.cpp
   **Objectif :** mesurer la performance du traitement massif d’ordres.

   **Classe de test :** PerformanceTest, dérivée de ::testing::Test, initialise un générateur aléatoire pour simuler des prix (100–200) et quantités (1–1000).

   **TEST_F(Process100KOrders) :**
   - Crée une instance de InstrumentManager.
   - Enregistre l’heure de début, puis appelle processOrder 100 000 fois avec des ordres LIMIT NEW aléatoires sur « AAPL ».
   - Calcule la durée en secondes et vérifie qu’elle est < 600 s.
   - Affiche le temps de traitement pour information.

   **But :** garantir une latence acceptable pour un flux élevé d’ordres.

2. test_MarketOrders.cpp
   **Objectif :** valider le comportement des ordres MARKET (prix forcé à 0, exécution et annulation).

   **Fixture :** MarketOrderTest initialise un carnet avec LIMIT SELL à 150, 151, 152 et LIMIT BUY à 149, 148 pour « AAPL ».

   - **PrixForceAZero :** crée deux Order MARKET (prix d’entrée 999.99 et –50.0), vérifie que getPrice() == 0.0.
     **But :** s’assurer que tout MARKET a bien un prix 0.

   - **ExecutionComplete :** envoie un MARKET BUY de 100 unités, parcourt les événements pour orderId 10, et attend :
     - 1 événement EXECUTED
     - executedQuantity = 100
     - affichage price = 0.0
     - executionPrice réel = 150.00
     **But :** tester l’exécution complète sur un niveau unique.

   - **ExecutionPartielleAvecAnnulation :** envoie un MARKET SELL de 500 unités, doit exécuter 150 @149.00 + 250 @148.00 = 400 unités puis annuler 100.
     Vérifie : 1 événement EXECUTED totalisant 400, puis 1 événement CANCELED (test du reliquat).
     **But :** couvrir exécution partielle + annulation.

   - **OrdreSansLiquidite :** MARKET BUY sur « MSFT » sans SELL, vérifie une annulation immédiate (OrderStatus::CANCELED, executedQuantity = 0).
     **But :** assurer l’annulation en l’absence de liquidité.

   - **ModificationOrdreMarketInterdit :** tente MODIFY sur un ordre MARKET, attend InvalidOrderException.
     **But :** interdire la modification des ordres MARKET.

   - **ExecutionAvecPlusieursNiveaux :** MARKET BUY de 350 unités sur trois niveaux (100 @150.00, 200 @151.00, 50 @152.00).
     Vérifie un seul événement EXECUTED totalisant 350, avec chaque executionPrice correct pour chaque fill.
     **But :** valider le routage multi‑niveaux.

3. test_MatchingEngine.cpp
   **Objectif :** s’assurer que MatchingEngine gère NEW/MODIFY/CANCEL et génère les événements adaptés.

   **Fixture :** MatchingEngineTest avec un engine pour « AAPL » et findEvent().

   - **ActionNew :** processOrder(..., Action::NEW) crée un LIMIT BUY, génère un PENDING ; vérifie getOrder(1).
     **But :** tester la création d’ordres.

   - **ActionModify :** modifie (quantity=200, price=151.00), attend un événement MODIFY et getOrder(1) à jour.
     **But :** valider la mise à jour.

   - **ActionCancel :** annule (quantity=0, price=0, Action::CANCEL), attend un CANCEL et disparition de l’ordre.
     **But :** garantir la suppression.

   - **ModifyOrderNotFound & CancelOrderNotFound :** MODIFY/CANCEL sur orderId 999, lance OrderNotFoundException.
     **But :** protéger contre les opérations sur ordres absents.

   - **ExecutionCreatesMultipleEvents :** deux SELL 150.00€ puis un BUY 150 150.00€ → 2 PENDING, 2 EXECUTED, 1 PARTIALLY_EXECUTED.
     **But :** vérifier le comptage et la typologie des événements.

   - **ModifyThenExecute :** SELL 100 à 150€ puis BUY à 149€ modifié à 150 → modification déclenche matching et EXECUTED bilatéral.
     **But :** tester la séquence MODIFY → matching.

   - **CancelPartiallyExecutedOrder :** SELL 50 puis BUY 100, partiel puis CANCEL → génération d’un événement CANCEL et suppression.
     **But :** gérer la suppression d’un ordre partiellement exécuté.

4. test_Order.cpp
   **Objectif :** valider la classe Order.

   **Fixture :** OrderTest avec createTestOrder().

   - **CreateValidOrder :** création d’un LIMIT BUY, vérification ID, instrument, side, type, quantités et statut PENDING.
     **But :** s’assurer de l’initialisation correcte.

   - **CreateOrderWithInvalidId :** orderId = 0, attend InvalidOrderException.
     **But :** interdire les IDs non valides.

   - **ExecuteOrder :** execute(50,…) deux fois → PARTIALLY_EXECUTED puis EXECUTED.
     **But :** tester le suivi du statut et de la quantité restante.

   - **CancelOrder :** cancel() → statut CANCELED.
     **But :** s’assurer que cancel() modifie le statut.

5. test_OrderMatcher.cpp
   **Objectif :** vérifier la logique de OrderMatcher.

   **Fixture :** OrderMatcherTest avec un OrderBook pour « AAPL ».

   - **MatchingLimitOrdersCroisement :** SELL 100 à 150€, 200 à 151€ et BUY 150 à 150.50€ → 1 trade (100 à 150€) + PARTIALLY_EXECUTED.
     **But :** tester le matching LIMIT vs LIMIT.

   - **MatchingMarketOrder :** SELL 150, 151, 152 → MARKET BUY 250 → 2 niveaux (100 + 150) → EXECUTED.
     **But :** valider le matching MARKET.

   - **PrioritePrixTemps :** SELL meilleur prix 149 puis deux à 150 → BUY 250 → respect priorité prix puis temps.
     **But :** s’assurer du bon ordre d’exécution.

   - **PasDeMatchingSiPrixNeCroisentPas :** SELL 151 vs BUY 150 → aucun trade, ordre reste PENDING.
     **But :** confirmer aucun matching hors spread.

   - **MarketOrderSansLiquidite :** carnet vide → MARKET BUY → CANCELED.
     **But :** vérification de l’annulation sans liquidité.

   - **MarketOrderReliquatAnnule :** SELL 100 à 150€ et MARKET BUY 200 → match 100 + annulation 100 restants.
     **But :** tester matching partiel + annulation.

### Gestion des erreurs

* **InvalidOrderException** lancé si :

  * `orderId` invalide ou déjà existant pour NEW.
  * Tentative de MODIFY ou CANCEL sur un ordre absent.
  * Modification d’un ordre MARKET.
* **OrderNotFoundException** pour tout MODIFY/CANCEL sans ordre correspondant.
* Tout ordre MARKET sans contrepartie disponible est immédiatement annulé.

---
