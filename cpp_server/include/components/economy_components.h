#ifndef NOVAFORGE_COMPONENTS_ECONOMY_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_ECONOMY_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

/**
 * @brief Insurance policy on a ship
 */
class InsurancePolicy : public ecs::Component {
public:
    std::string policy_id;
    std::string ship_type;
    std::string tier = "basic";    // "basic", "standard", "platinum"
    float coverage_fraction = 0.5f; // fraction of ship value paid out
    double premium_paid = 0.0;     // Credits paid for policy
    double payout_value = 0.0;     // Credits paid out on loss
    float duration_remaining = -1.0f; // seconds, -1 = permanent
    bool active = true;
    bool claimed = false;

    COMPONENT_TYPE(InsurancePolicy)
};

/**
 * @brief Tracks bounty rewards earned by a player
 */
class BountyLedger : public ecs::Component {
public:
    double total_bounty_earned = 0.0;
    int total_kills = 0;
    
    struct BountyRecord {
        std::string target_id;
        std::string target_name;
        double bounty_amount = 0.0;
        std::string faction;
    };
    
    std::vector<BountyRecord> recent_kills;  // last N kills
    static constexpr int MAX_RECENT = 50;
    
    COMPONENT_TYPE(BountyLedger)
};

/**
 * @brief Market order tracking for stations
 */
class MarketHub : public ecs::Component {
public:
    struct Order {
        std::string order_id;
        std::string item_id;
        std::string item_name;
        std::string owner_id;       // entity that placed the order
        bool is_buy_order = false;   // true = buy, false = sell
        double price_per_unit = 0.0;
        int quantity = 1;
        int quantity_remaining = 1;
        float duration_remaining = -1.0f;  // seconds, -1 = permanent
        bool fulfilled = false;
    };

    std::string station_id;
    std::vector<Order> orders;
    double broker_fee_rate = 0.02;  // 2% broker fee
    double sales_tax_rate = 0.04;   // 4% sales tax

    COMPONENT_TYPE(MarketHub)
};
class ContractBoard : public ecs::Component {
public:
    struct ContractItem {
        std::string item_id;
        std::string name;
        int quantity = 1;
        float volume = 1.0f;
    };

    struct Contract {
        std::string contract_id;
        std::string issuer_id;
        std::string assignee_id;
        std::string type;            // "item_exchange", "courier", "auction"
        std::string status;          // "outstanding", "in_progress", "completed", "expired", "failed"
        std::vector<ContractItem> items_offered;
        std::vector<ContractItem> items_requested;
        double isc_reward = 0.0;
        double isc_collateral = 0.0;
        float duration_remaining = -1.0f;
        float days_to_complete = 3.0f;
    };

    std::vector<Contract> contracts;

    COMPONENT_TYPE(ContractBoard)
};
/**
 * @brief Manufacturing facility for blueprint-based production
 *
 * Tracks manufacturing jobs: blueprint, materials, output,
 * time remaining, and job status.
 */
class ManufacturingFacility : public ecs::Component {
public:
    std::string facility_id;
    std::string station_id;
    int max_jobs = 1;  // concurrent job slots

    struct MaterialRequirement {
        std::string material_id;
        int quantity = 0;
    };

    struct ManufacturingJob {
        std::string job_id;
        std::string blueprint_id;
        std::string owner_id;
        std::string output_item_id;
        std::string output_item_name;
        int output_quantity = 1;
        int runs = 1;
        int runs_completed = 0;
        float time_per_run = 3600.0f;   // seconds per run
        float time_remaining = 3600.0f; // current run time remaining
        std::string status;             // "pending", "active", "completed", "cancelled"
        double install_cost = 0.0;
        std::vector<MaterialRequirement> materials;
        float material_efficiency = 0.0f;  // 0-10, reduces material cost
        float time_efficiency = 0.0f;      // 0-20, reduces time
    };

    std::vector<ManufacturingJob> jobs;

    int activeJobCount() const {
        int count = 0;
        for (const auto& j : jobs)
            if (j.status == "active") ++count;
        return count;
    }

    COMPONENT_TYPE(ManufacturingFacility)
};

/**
 * @brief Research laboratory for invention and blueprint research
 *
 * Supports ME/TE research on blueprints and Tech II invention.
 */
class ResearchLab : public ecs::Component {
public:
    std::string lab_id;
    std::string station_id;
    int max_jobs = 1;

    struct ResearchJob {
        std::string job_id;
        std::string blueprint_id;
        std::string owner_id;
        std::string research_type;       // "material_efficiency", "time_efficiency", "invention"
        float time_remaining = 3600.0f;
        float total_time = 3600.0f;
        std::string status;              // "active", "completed", "failed"
        // ME/TE research
        int target_level = 1;            // target ME or TE level
        // Invention
        std::string output_blueprint_id; // T2 blueprint on success
        float success_chance = 0.5f;     // 0.0-1.0
        std::string datacore_1;
        std::string datacore_2;
        double install_cost = 0.0;
    };

    std::vector<ResearchJob> jobs;

    int activeJobCount() const {
        int count = 0;
        for (const auto& j : jobs)
            if (j.status == "active") ++count;
        return count;
    }

    COMPONENT_TYPE(ResearchLab)
};
/**
 * @brief Refining facility — converts raw ore into refined minerals
 *
 * Attached to station entities that offer refining services.
 * Efficiency determines the yield: at 1.0 (100%) all ore converts
 * to minerals; at 0.5 only half does.
 *
 * Each ore type maps to one or more mineral outputs via refine_ratios.
 */
class RefiningFacility : public ecs::Component {
public:
    struct RefineRecipe {
        std::string ore_type;           // input ore name (e.g. "Ferrite")
        int ore_units_required = 100;   // units consumed per batch
        struct MineralOutput {
            std::string mineral_type;   // output mineral name (e.g. "Stellium")
            int base_quantity = 333;    // base output per batch at 100% efficiency
        };
        std::vector<MineralOutput> outputs;
    };

    float efficiency = 0.5f;            // 0.0–1.0, refining yield multiplier
    float tax_rate = 0.05f;             // fraction taken as tax
    std::vector<RefineRecipe> recipes;

    COMPONENT_TYPE(RefiningFacility)
};
// ==================== Enhanced Market ====================

class MarketOrder : public ecs::Component {
public:
    enum class OrderType { Buy, Sell };

    OrderType type = OrderType::Buy;
    std::string item_type;
    int quantity = 0;
    int quantity_remaining = 0;
    float price_per_unit = 0.0f;
    std::string region_id;
    std::string station_id;
    std::string owner_id;
    float expiry_time = 86400.0f;
    float elapsed_time = 0.0f;
    bool is_npc_order = false;
    bool is_filled = false;

    COMPONENT_TYPE(MarketOrder)
};

class AIFleetDispatch : public ecs::Component {
public:
    enum class DispatchType { Mining, Hauling, Production };

    DispatchType type = DispatchType::Mining;
    std::string target_system_id;
    std::string order_id;
    bool dispatched = false;
    float estimated_completion = 0.0f;
    float elapsed = 0.0f;
    int fleet_size = 1;

    bool isComplete() const { return elapsed >= estimated_completion; }

    COMPONENT_TYPE(AIFleetDispatch)
};

// ==================== Living Universe: AI Economic Actors ====================

/**
 * @brief Makes NPCs real economic participants with wallets, ship ownership,
 *        and permanent death.
 *
 * NPCs are no longer disposable spawns — they own ships, earn and spend Credits,
 * and permanently die if they cannot afford a replacement hull.
 */
class AIEconomicActor : public ecs::Component {
public:
    std::string owned_ship_type;        // e.g. "Rifter", "Caracal"
    double ship_value = 0.0;            // estimated hull Credits value
    double repair_cost_ratio = 0.15;    // fraction of ship_value for repairs
    bool is_destroyed = false;          // ship currently destroyed
    bool permanently_dead = false;      // cannot afford replacement — removed from sim
    int destruction_count = 0;          // total times destroyed
    int replacement_count = 0;          // total times replaced ship
    float time_alive = 0.0f;           // seconds since last respawn / spawn

    bool canAffordReplacement(double wallet) const {
        return wallet >= ship_value;
    }

    COMPONENT_TYPE(AIEconomicActor)
};
// ==================== Phase 15: Convoy Ambush AI ====================

/**
 * @brief Represents a trade route between two systems with associated cargo info.
 */
class ConvoyRoute : public ecs::Component {
public:
    std::string route_id;
    std::string origin_system;
    std::string destination_system;
    std::string cargo_type;
    double cargo_value = 0.0;
    float security_level = 0.5f;    // 0.0 (lawless) to 1.0 (high-sec)
    int active_convoys = 0;          // currently travelling this route
    float pirate_interest = 0.0f;   // 0.0 to 1.0, grows with cargo value

    COMPONENT_TYPE(ConvoyRoute)
};

/**
 * @brief Represents a planned or active pirate ambush on a convoy route.
 */
class ConvoyAmbush : public ecs::Component {
public:
    enum class AmbushState { Planned, Active, Successful, Failed, Dispersed };

    std::string ambush_id;
    std::string pirate_entity_id;
    std::string route_id;
    AmbushState state = AmbushState::Planned;
    float cooldown_remaining = 0.0f;
    double loot_value = 0.0;         // Credits value captured
    int ships_attacked = 0;

    static std::string stateToString(AmbushState s) {
        switch (s) {
            case AmbushState::Planned:     return "planned";
            case AmbushState::Active:      return "active";
            case AmbushState::Successful:  return "successful";
            case AmbushState::Failed:      return "failed";
            case AmbushState::Dispersed:   return "dispersed";
            default:                       return "unknown";
        }
    }

    COMPONENT_TYPE(ConvoyAmbush)
};
// ==================== Living Universe: Supply/Demand Economy ====================

class SupplyDemand : public ecs::Component {
public:
    struct CommodityState {
        std::string commodity_id;
        float supply = 100.0f;        // current supply units
        float demand = 100.0f;        // current demand units
        float base_price = 100.0f;    // base price per unit
        float current_price = 100.0f; // computed price
        float supply_rate = 1.0f;     // units produced per tick by NPCs
        float demand_rate = 1.0f;     // units consumed per tick by NPCs
    };

    std::vector<CommodityState> commodities;
    float price_elasticity = 0.5f;          // how sensitive prices are to supply/demand ratio
    float npc_activity_modifier = 1.0f;     // scales NPC production/consumption rates
    float price_floor_multiplier = 0.2f;    // minimum price = base_price * floor
    float price_ceiling_multiplier = 5.0f;  // maximum price = base_price * ceiling
    float supply_decay_rate = 0.01f;        // natural supply reduction per tick (consumption)
    float demand_drift_rate = 0.005f;       // demand drifts toward baseline per tick

    void addCommodity(const std::string& id, float base_price, float initial_supply, float initial_demand) {
        for (auto& c : commodities) {
            if (c.commodity_id == id) return; // already exists
        }
        CommodityState cs;
        cs.commodity_id = id;
        cs.base_price = base_price;
        cs.current_price = base_price;
        cs.supply = initial_supply;
        cs.demand = initial_demand;
        commodities.push_back(cs);
    }

    CommodityState* getCommodity(const std::string& id) {
        for (auto& c : commodities) {
            if (c.commodity_id == id) return &c;
        }
        return nullptr;
    }

    const CommodityState* getCommodity(const std::string& id) const {
        for (const auto& c : commodities) {
            if (c.commodity_id == id) return &c;
        }
        return nullptr;
    }

    int getCommodityCount() const { return static_cast<int>(commodities.size()); }

    COMPONENT_TYPE(SupplyDemand)
};

// ==================== Phase 15: Black Market & Smuggling ====================

class BlackMarket : public ecs::Component {
public:
    struct Listing {
        std::string item_id;
        std::string seller_id;
        float price = 0.0f;
        int quantity = 0;
        float risk_level = 0.0f;    // 0.0 (safe) to 1.0 (dangerous)
        bool contraband = false;
        float expiry_timer = 0.0f;
        float max_expiry = 600.0f;  // 10 minutes default
    };

    std::vector<Listing> listings;
    float security_level = 0.5f;            // system security (higher = more risky to trade)
    float detection_chance_base = 0.1f;     // base chance of being detected per transaction
    float price_markup = 1.5f;              // black market items cost more
    int max_listings = 20;
    float listing_refresh_timer = 0.0f;
    float listing_refresh_interval = 120.0f;  // new listings every 2 minutes

    void addListing(const std::string& item_id, const std::string& seller_id,
                    float price, int quantity, bool contraband_item, float risk) {
        if (static_cast<int>(listings.size()) >= max_listings) {
            listings.erase(listings.begin());  // remove oldest
        }
        Listing l;
        l.item_id = item_id;
        l.seller_id = seller_id;
        l.price = price * price_markup;
        l.quantity = quantity;
        l.contraband = contraband_item;
        l.risk_level = risk;
        l.max_expiry = 600.0f;
        listings.push_back(l);
    }

    int getListingCount() const { return static_cast<int>(listings.size()); }

    Listing* findListing(const std::string& item_id) {
        for (auto& l : listings) {
            if (l.item_id == item_id) return &l;
        }
        return nullptr;
    }

    const Listing* findListing(const std::string& item_id) const {
        for (const auto& l : listings) {
            if (l.item_id == item_id) return &l;
        }
        return nullptr;
    }

    COMPONENT_TYPE(BlackMarket)
};

// ==================== Price History for Dynamic Price Graphs ====================

/**
 * @brief Tracks historical price data for market items to enable price graph visualization
 * 
 * Stores timestamped price snapshots with sell price, buy price, and volume.
 * Used by PriceHistorySystem to provide trend analysis and historical data.
 */
class PriceHistory : public ecs::Component {
public:
    struct PriceEntry {
        std::string item_id;
        float timestamp = 0.0f;        // game time when recorded
        double sell_price = 0.0;       // lowest sell price at snapshot time
        double buy_price = 0.0;        // highest buy price at snapshot time
        int volume = 0;                // trade volume in this period
        
        PriceEntry() = default;
        PriceEntry(const std::string& id, float ts, double sell, double buy, int vol)
            : item_id(id), timestamp(ts), sell_price(sell), buy_price(buy), volume(vol) {}
    };

    std::string region_id;
    std::map<std::string, std::vector<PriceEntry>> item_history;  // item_id -> history
    int max_entries_per_item = 720;  // 30 days of hourly data

    void addEntry(const std::string& item_id, float timestamp, 
                  double sell_price, double buy_price, int volume) {
        auto& history = item_history[item_id];
        history.push_back(PriceEntry(item_id, timestamp, sell_price, buy_price, volume));
        
        // Trim to max entries
        while (static_cast<int>(history.size()) > max_entries_per_item) {
            history.erase(history.begin());
        }
    }

    std::vector<PriceEntry> getHistory(const std::string& item_id, int max_count = 0) const {
        auto it = item_history.find(item_id);
        if (it == item_history.end()) return {};
        
        const auto& history = it->second;
        if (max_count <= 0 || max_count >= static_cast<int>(history.size())) {
            return history;
        }
        
        // Return last max_count entries
        return std::vector<PriceEntry>(history.end() - max_count, history.end());
    }

    int getEntryCount(const std::string& item_id) const {
        auto it = item_history.find(item_id);
        return (it != item_history.end()) ? static_cast<int>(it->second.size()) : 0;
    }

    COMPONENT_TYPE(PriceHistory)
};

/**
 * @brief Tracks the economic flow lifecycle for a star system
 *
 * Records production, transport, consumption, and destruction of goods
 * so that the economy operates without fake NPC market orders.
 * Everything is produced, transported, consumed, or destroyed.
 */
class EconomicFlowState : public ecs::Component {
public:
    // Per-commodity flow tracking: commodity_id -> amount per tick
    std::map<std::string, float> production_rate;     // goods produced
    std::map<std::string, float> consumption_rate;    // goods consumed
    std::map<std::string, float> transport_in_rate;   // goods arriving
    std::map<std::string, float> transport_out_rate;  // goods leaving
    std::map<std::string, float> destruction_rate;    // goods destroyed (combat, decay)

    // Aggregate metrics
    float total_production = 0.0f;
    float total_consumption = 0.0f;
    float economic_health = 1.0f;    // 0.0=collapsed, 1.0=balanced, >1=surplus

    // Regional market delays
    float market_info_delay = 0.0f;  // seconds of information lag
    float last_update_time = 0.0f;

    COMPONENT_TYPE(EconomicFlowState)
};

/**
 * @brief Tracks end-to-end production chains (mining → refining → manufacturing → market)
 *
 * Enables the economy to function without fake NPC market orders by tracking
 * each stage of the production pipeline. Everything is produced, transported,
 * consumed, or destroyed.
 */
class ResourceProductionChain : public ecs::Component {
public:
    struct ChainStage {
        std::string stage_name;      // "mining", "refining", "manufacturing", "market"
        std::string input_resource;   // resource consumed at this stage
        std::string output_resource;  // resource produced at this stage
        float conversion_rate = 1.0f; // input -> output ratio
        float throughput = 0.0f;      // units processed per tick
        float efficiency = 1.0f;      // 0.0-1.0 efficiency modifier
        float bottleneck_factor = 1.0f; // 0.0-1.0, reduced when downstream is full
    };

    std::vector<ChainStage> stages;
    float overall_efficiency = 1.0f;  // product of all stage efficiencies
    float total_output = 0.0f;        // final product output per tick
    bool is_active = true;            // chain enabled/disabled
    std::string chain_id;             // unique identifier for this chain
    float uptime = 0.0f;             // seconds chain has been running

    COMPONENT_TYPE(ResourceProductionChain)
};

/**
 * @brief Trade route distortion from titan assembly pressure in the outer rim
 */
class OuterRimLogisticsDistortion : public ecs::Component {
public:
    std::string region_id;
    struct RouteDistortion {
        std::string route_id;
        float efficiency = 1.0f;
        float threat_level = 0.0f;
        float price_impact = 0.0f;
    };
    std::vector<RouteDistortion> routes;
    float global_threat = 0.0f;
    float distortion_factor = 0.5f;
    float recovery_rate = 0.05f;
    int disrupted_route_count = 0;
    float total_price_impact = 0.0f;
    int max_routes = 50;
    bool active = true;

    COMPONENT_TYPE(OuterRimLogisticsDistortion)
};

// ==================== Incursion Events ====================

/**
 * @brief Dynamic group PVE incursion event spawning in a star system
 *
 * Tracks waves of NPCs, influence level, participant roster,
 * and reward pool for incursion encounters.
 */
class Incursion : public ecs::Component {
public:
    enum class IncursionState { Pending, Active, Withdrawing, Defeated };

    struct Wave {
        int wave_number = 0;
        std::string npc_type;
        int count = 0;
        bool spawned = false;
        bool defeated = false;
    };

    std::string incursion_id;
    std::string system_id;
    std::vector<Wave> waves;
    IncursionState state = IncursionState::Pending;
    float influence = 1.0f;
    float reward_pool = 0.0f;
    float duration = 3600.0f;
    float elapsed = 0.0f;
    int tier = 1;
    std::vector<std::string> participants;
    int total_waves_spawned = 0;
    int total_waves_defeated = 0;
    int max_waves = 10;
    int max_participants = 20;
    bool active = true;

    COMPONENT_TYPE(Incursion)
};

// ==================== Loyalty Point Store ====================

/**
 * @brief LP rewards from missions and LP stores for unique items
 *
 * Tracks store inventory, player LP balances, and purchase history
 * for a faction loyalty point store.
 */
class LoyaltyPointStore : public ecs::Component {
public:
    struct StoreItem {
        std::string item_id;
        std::string name;
        std::string category;
        int lp_cost = 0;
        float isc_cost = 0.0f;
        int tier = 1;
        bool in_stock = true;
        int times_purchased = 0;
    };

    struct LPBalance {
        std::string player_id;
        int balance = 0;
        int total_earned = 0;
        int total_spent = 0;
    };

    std::string store_id;
    std::string faction_id;
    std::vector<StoreItem> items;
    std::vector<LPBalance> balances;
    int max_items = 50;
    int max_players = 100;
    int total_purchases = 0;
    float isc_collected = 0.0f;
    bool active = true;

    COMPONENT_TYPE(LoyaltyPointStore)
};

// ==================== Contract Auction ====================

/**
 * @brief Auction-based contract system for marketplace bidding
 *
 * Manages auction listings with bids, buyout prices, time-based expiry,
 * seller/buyer tracking, and bid history for an in-game marketplace.
 */
class ContractAuction : public ecs::Component {
public:
    enum class AuctionState { Pending, Active, Sold, Expired, Cancelled };

    struct Bid {
        std::string bidder_id;
        float amount = 0.0f;
        float timestamp = 0.0f;
    };

    struct AuctionListing {
        std::string listing_id;
        std::string seller_id;
        std::string item_name;
        std::string category;       // Ship, Module, Blueprint, Material, Misc
        float starting_price = 0.0f;
        float buyout_price = 0.0f;  // 0 = no buyout
        float current_bid = 0.0f;
        std::string highest_bidder;
        float duration = 3600.0f;   // seconds
        float elapsed = 0.0f;
        AuctionState state = AuctionState::Pending;
        std::vector<Bid> bid_history;
        int bid_count = 0;
    };

    std::vector<AuctionListing> listings;
    int max_listings = 50;
    int total_sold = 0;
    int total_expired = 0;
    int total_bids = 0;
    float total_revenue = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ContractAuction)
};

// ==================== Trade Route Analytics ====================

/**
 * @brief Trade route performance tracking with volume, revenue, and congestion
 *
 * Monitors trade route metrics for visible supply/demand cycles.
 * Tracks per-route volume, revenue, profit margins, and congestion levels
 * with periodic snapshot intervals for trend analysis.
 */
class TradeRouteAnalytics : public ecs::Component {
public:
    struct TradeRoute {
        std::string route_id;
        std::string origin_system;
        std::string dest_system;
        std::string commodity;
        float volume = 0.0f;
        float revenue = 0.0f;
        float cost = 0.0f;
        int trips = 0;
        float congestion = 0.0f;    // 0.0 = clear, 1.0 = fully congested
        bool active = true;
    };

    std::vector<TradeRoute> routes;
    int max_routes = 20;
    float snapshot_interval = 60.0f;  // seconds between snapshots
    float snapshot_timer = 0.0f;
    int total_snapshots = 0;
    float total_volume = 0.0f;
    float total_revenue = 0.0f;
    bool active = true;

    COMPONENT_TYPE(TradeRouteAnalytics)
};

/**
 * @brief Persistent player wallet with transaction logging
 */
class PlayerWallet : public ecs::Component {
public:
    struct Transaction {
        std::string transaction_id;
        std::string description;
        double amount = 0.0;        // positive = credit, negative = debit
        double balance_after = 0.0;
        float timestamp = 0.0f;
    };

    double balance = 0.0;
    double lifetime_earned = 0.0;
    double lifetime_spent = 0.0;
    double tax_rate = 0.05;          // 5% default tax
    int max_transactions = 100;
    float elapsed = 0.0f;
    bool active = true;
    std::vector<Transaction> transactions;

    COMPONENT_TYPE(PlayerWallet)
};

/**
 * @brief Player wallet with ISC transaction ledger
 *
 * Tracks player balance with deposit/withdrawal operations and
 * maintains a transaction history for audit trail. Supports the
 * full trade loop in the vertical slice.
 */
class WalletLedger : public ecs::Component {
public:
    struct Transaction {
        std::string tx_id;
        std::string description;
        std::string category;  // "Trade", "Bounty", "Insurance", "Mission", "Tax", "Transfer"
        double amount = 0.0;   // positive = credit, negative = debit
        double balance_after = 0.0;
        float timestamp = 0.0f;
    };

    double balance = 0.0;
    double total_earned = 0.0;
    double total_spent = 0.0;
    std::vector<Transaction> transactions;
    int max_transactions = 100;
    int total_tx_count = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(WalletLedger)
};

/**
 * @brief Station-side repair service state
 *
 * Tracks ongoing repair jobs for entities docked at a station.
 * Shield, armor, and hull are restored over time at a cost proportional
 * to the missing HP and the station's repair rate.
 */
class StationRepairService : public ecs::Component {
public:
    enum class RepairPhase { Idle, RepairingShield, RepairingArmor, RepairingHull, Complete };

    RepairPhase phase = RepairPhase::Idle;
    std::string station_id;
    float shield_to_repair = 0.0f;
    float armor_to_repair = 0.0f;
    float hull_to_repair = 0.0f;
    float repair_rate = 50.0f;       // HP per second
    float cost_per_hp = 1.5f;
    float total_cost = 0.0f;
    float total_hp_repaired = 0.0f;
    float elapsed = 0.0f;
    bool auto_repair = false;        // if true, begin repair immediately on dock
    bool active = true;

    COMPONENT_TYPE(StationRepairService)
};

/**
 * @brief Tracks active cargo transfers between docked entities
 *
 * When two entities are docked at the same station, cargo can be moved
 * between them at a configurable transfer speed.  Each tick the system
 * advances active transfers and marks them complete when done.
 */
class CargoTransfer : public ecs::Component {
public:
    struct TransferJob {
        std::string target_id;
        std::string item_type;
        float amount = 0.0f;
        float transferred = 0.0f;
        float transfer_speed = 100.0f;  // units per second
        bool completed = false;
    };

    std::vector<TransferJob> jobs;
    int max_concurrent_transfers = 3;
    int total_transfers_completed = 0;
    float total_units_transferred = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CargoTransfer)
};

// ==================== Mining Belt Populator ====================

/**
 * @brief Asteroid belt population management with depletion and respawn
 *
 * Spawns and tracks asteroid entities within a mining belt.  Each
 * asteroid has an ore type, initial quantity, remaining quantity and
 * an optional richness bonus.  Depleted asteroids are tracked and
 * respawned after a configurable interval.
 */
class MiningBeltPopulator : public ecs::Component {
public:
    struct AsteroidEntry {
        std::string asteroid_id;
        std::string ore_type;
        float initial_quantity = 1000.0f;
        float remaining_quantity = 1000.0f;
        float richness = 1.0f;         // yield multiplier
        bool depleted = false;
    };

    std::vector<AsteroidEntry> asteroids;
    float respawn_interval = 3600.0f;   // seconds between respawn cycles
    float respawn_timer = 0.0f;
    int max_asteroids = 30;
    int total_mined = 0;
    int total_respawned = 0;
    float total_ore_extracted = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(MiningBeltPopulator)
};

// ==================== Station Service Registry ====================

/**
 * @brief Station service availability, pricing, and usage tracking
 *
 * Manages the set of services available at a station (repair, refine,
 * market, clone bay, etc.).  Each service has a cost, an availability
 * flag, and an optional cooldown.  Usage is tracked per service for
 * analytics.
 */
class StationServiceRegistry : public ecs::Component {
public:
    enum class ServiceCategory { Repair, Refine, Market, CloneBay, Insurance, Fitting, Reprocessing };

    struct ServiceEntry {
        std::string service_id;
        std::string name;
        ServiceCategory category = ServiceCategory::Repair;
        float cost = 0.0f;
        float cooldown = 0.0f;         // seconds between uses, 0 = no cooldown
        float cooldown_remaining = 0.0f;
        int times_used = 0;
        bool available = true;
    };

    std::vector<ServiceEntry> services;
    int max_services = 20;
    int total_uses = 0;
    float total_revenue = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(StationServiceRegistry)
};

/**
 * @brief Ore compression facility for efficient transport
 *
 * Compresses raw ore into compressed blocks at a configurable ratio,
 * reducing cargo volume for hauling.  Each ore type may have a different
 * compression ratio.  Processing takes time and consumes a small ISC fee.
 */
class OreCompression : public ecs::Component {
public:
    enum class CompressionState { Idle, Compressing, Complete, Failed };

    struct OreType {
        std::string ore_name;
        float compression_ratio = 10.0f;   // 10 raw → 1 compressed
        float process_time = 5.0f;         // seconds per batch
        double cost_per_batch = 50.0;      // ISC per batch
    };

    std::vector<OreType> ore_types;
    CompressionState state = CompressionState::Idle;
    std::string current_ore;
    int raw_units_queued = 0;
    int compressed_units_produced = 0;
    float process_timer = 0.0f;
    double total_isc_spent = 0.0;
    int total_batches_processed = 0;
    int total_raw_consumed = 0;
    int total_compressed_produced = 0;
    int max_queue = 1000;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(OreCompression)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_ECONOMY_COMPONENTS_H
