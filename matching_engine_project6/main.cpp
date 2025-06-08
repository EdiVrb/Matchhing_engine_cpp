// ===== main.cpp =====
#include <iostream>
#include <string>
#include <chrono>
#include "core/InstrumentManager.hpp"
#include "io/CSVReader.hpp"
#include "io/CSVWriter.hpp"
#include "exceptions/Exceptions.hpp"
#include "utils/Logger.hpp"

// Fonctions de parsing
Side parseSide(const std::string& str) {
    if (str == "BUY") return Side::BUY;
    if (str == "SELL") return Side::SELL;
    throw std::invalid_argument("Invalid side: " + str);
}

OrderType parseOrderType(const std::string& str) {
    if (str == "LIMIT") return OrderType::LIMIT;
    if (str == "MARKET") return OrderType::MARKET;
    throw std::invalid_argument("Invalid order type: " + str);
}

Action parseAction(const std::string& str) {
    if (str == "NEW") return Action::NEW;
    if (str == "MODIFY") return Action::MODIFY;
    if (str == "CANCEL") return Action::CANCEL;
    throw std::invalid_argument("Invalid action: " + str);
}

// Fonction pour parser le prix avec gestion spéciale des ordres MARKET
Price parsePrice(const std::string& str, OrderType type) {
    // Pour les ordres MARKET, toujours retourner 0 peu importe la valeur
    if (type == OrderType::MARKET) {
        return 0.0;
    }
    
    // Pour les ordres LIMIT, parser normalement
    try {
        if (str.empty() || str == "na" || str == "NA" || str == "null" || str == "NULL") {
            throw std::invalid_argument("Invalid price for LIMIT order");
        }
        return std::stod(str);
    } catch (...) {
        throw std::invalid_argument("Invalid price: " + str);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.csv> <output.csv>" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    
    try {
        // Initialiser le logger
        Logger::init("matching_engine.log");
        Logger::log("Starting Matching Engine");
        
        // Mesurer le temps de traitement
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Initialiser les composants
        InstrumentManager manager;
        CSVReader reader(inputFile);
        CSVWriter writer(outputFile);
        
        writer.writeHeader();
        
        size_t orderCount = 0;
        size_t errorCount = 0;
        
        // Traiter chaque ligne du fichier
        reader.readLine([&](const std::vector<std::string>& fields) {
            if (fields.size() < 8) {
                errorCount++;
                Logger::log("Invalid line format: insufficient fields");
                return;
            }
            
            try {
                // Parser les champs
                Timestamp timestamp = std::stoull(fields[0]);
                OrderId orderId = std::stoull(fields[1]);
                std::string instrument = fields[2];
                Side side = parseSide(fields[3]);
                OrderType type = parseOrderType(fields[4]);
                Quantity quantity = std::stoull(fields[5]);
                // Utiliser la fonction parsePrice qui gère les ordres MARKET
                Price price = parsePrice(fields[6], type);
                Action action = parseAction(fields[7]);
                
                // Traiter l'ordre
                manager.processOrder(timestamp, orderId, instrument, 
                                   side, type, quantity, price, action);
                
                orderCount++;
                
                // Log périodique
                if (orderCount % 10000 == 0) {
                    Logger::log("Processed " + std::to_string(orderCount) + " orders");
                }
                
            } catch (const std::exception& e) {
                errorCount++;
                Logger::log("Error processing order: " + std::string(e.what()));
            }
        });
        
        // Écrire tous les événements dans le fichier de sortie
        auto allEvents = manager.getAllEvents();
        
        for (const auto& event : allEvents) {
            writer.writeEvent(event);
        }
        
        // Mesurer le temps final
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
        
        // Afficher les statistiques
        std::cout << "=== Matching Engine Statistics ===" << std::endl;
        std::cout << "Total orders processed: " << orderCount << std::endl;
        std::cout << "Total events generated: " << allEvents.size() << std::endl;
        std::cout << "Total errors: " << errorCount << std::endl;
        std::cout << "Total execution time: " << duration.count() << " seconds" << std::endl;
        std::cout << "Orders per second: " << (orderCount / (duration.count() + 1)) << std::endl;
        
        Logger::log("Matching Engine completed successfully");
        Logger::close();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        Logger::log("Fatal error: " + std::string(e.what()));
        Logger::close();
        return 1;
    }
}