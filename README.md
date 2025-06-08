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
   - **Process100KOrders** : envoie 100 000 ordres LIMIT NEW et mesure le temps d’exécution. Le test vérifie que le moteur répond en moins de 600 secondes pour garantir sa robustesse sous haute charge.

2. test_MarketOrders.cpp
   - **PrixForceAZero** : crée des ordres MARKET avec des prix non nuls pour vérifier que la classe `Order` réinitialise toujours le prix à 0.
   - **ExecutionComplete** : envoie un MARKET BUY de 100 unités, puis vérifie qu’un seul événement `EXECUTED` est généré avec la quantité exacte et le meilleur prix disponible.
   - **ExecutionPartielleAvecAnnulation** : envoie un MARKET SELL de 500 unités sur deux niveaux de prix (149.00 et 148.00), s’assure que 400 unités sont exécutées et que le reliquat de 100 unités est annulé.
   - **OrdreSansLiquidite** : teste un MARKET BUY pour « MSFT » quand il n’y a pas d’offres SELL, et vérifie une annulation immédiate de l’ordre.
   - **ModificationOrdreMarketInterdit** : tente de modifier un ordre MARKET existant et attend une `InvalidOrderException` pour valider l’interdiction de modification.
   - **ExecutionAvecPlusieursNiveaux** : lance un MARKET BUY de 350 unités sur trois niveaux de prix (150.00, 151.00, 152.00) et vérifie que chaque fill se fait au bon prix, tout en consolidant la quantité totale exécutée.

3. test_MatchingEngine.cpp
   - **ActionNew** : soumet un ordre LIMIT NEW et vérifie la génération d’un événement `PENDING` ainsi que la présence de l’ordre dans le carnet.
   - **ActionModify** : modifie un ordre existant (quantité et prix) et s’assure qu’un événement `MODIFY` est généré et que les attributs de l’ordre sont mis à jour.
   - **ActionCancel** : annule un ordre via `CANCEL` et vérifie la suppression du carnet et la création d’un événement `CANCELED`.
   - **ModifyOrderNotFound** et **CancelOrderNotFound** : tentent de modifier ou d’annuler un `orderId` inexistant et attendent une `OrderNotFoundException` pour chaque cas.
   - **ExecutionCreatesMultipleEvents** : organise deux ordres SELL suivis d’un BUY LIMIT pour tester la création de plusieurs événements (`PENDING`, `EXECUTED`, `PARTIALLY_EXECUTED`) lors d’un crossing.
   - **ModifyThenExecute** : modifie un ordre BUY pour qu’il corresponde à un ordre SELL existant et valide la séquence `MODIFY` → matching → `EXECUTED`.
   - **CancelPartiallyExecutedOrder** : exécute partiellement un ordre puis l’annule, vérifie la génération de l’événement `CANCELED` pour le reliquat.

4. test_Order.cpp
   - **CreateValidOrder** : crée un ordre LIMIT BUY et vérifie tous ses attributs (ID, instrument, side, quantité, prix, statut `PENDING`).
   - **CreateOrderWithInvalidId** : essaie de créer un ordre avec `orderId = 0` et attend une `InvalidOrderException`.
   - **ExecuteOrder** : exécute un ordre en deux phases (`PARTIALLY_EXECUTED` puis `EXECUTED`) et vérifie la mise à jour de la quantité restante.
   - **CancelOrder** : appelle `cancel()` sur un ordre et confirme que son statut devient `CANCELED`.

5. test_OrderMatcher.cpp
   - **MatchingLimitOrdersCroisement** : croise un ordre BUY LIMIT avec plusieurs SELL LIMIT et contrôle qu’un seul trade partiel se produit et que le reste reste en attente (`PARTIALLY_EXECUTED`).
   - **MatchingMarketOrder** : effectue un MARKET BUY contre plusieurs niveaux de SELL et vérifie que chaque niveau est consommé correctement.
   - **PrioritePrixTemps** : s’assure que l’ordre sélectionne d’abord le meilleur prix puis, en cas d’égalité, l’ordre le plus ancien.
   - **PasDeMatchingSiPrixNeCroisentPas** : teste qu’aucun trade n’a lieu lorsque le prix BUY est inférieur au meilleur SELL.
   - **MarketOrderSansLiquidite** : valide que tout MARKET BUY sans offres SELL est immédiatement annulé.
   - **MarketOrderReliquatAnnule** : après consommation partielle d’un MARKET BUY, s’assure que le reliquat est annulé.

### Gestion des erreurs

* **InvalidOrderException** lancé si :

  * `orderId` invalide ou déjà existant pour NEW.
  * Tentative de MODIFY ou CANCEL sur un ordre absent.
  * Modification d’un ordre MARKET.
* **OrderNotFoundException** pour tout MODIFY/CANCEL sans ordre correspondant.
* Tout ordre MARKET sans contrepartie disponible est immédiatement annulé.

---
