# MMO Space - Unreal Engine 5.8 RTS/4X Strategy Plugin

Complete RTS/4X space strategy game engine for Unreal Engine 5.8. Inspired by OGame and Stellaris.

## Architecture

### Systems (All C++ with full Blueprint exposure)

| System | Class | Description |
|--------|-------|-------------|
| **Database** | `UMMOSQLDatabaseSubsystem` | SQL persistence layer, JSON file fallback |
| **Galaxy** | `UMMOGalaxySubsystem` | Procedural universe generation, 9 galaxies x 499 systems |
| **Empire** | `UMMOEmpireSubsystem` | Empire leveling, 10 government types, turns system |
| **Fleet** | `UMMOFleetSubsystem` | 10 ship classes, fleet dispatch/return, travel times |
| **Combat** | `UMMOCombatEngine` | 6-round combat, 5 formations, debris fields |
| **Research** | `UMMOResearchSubsystem` | 19+ technology tree across 15 categories |
| **Economy** | `UMMOEconomySubsystem` | Resource production, building costs, marketplace |

### Core Types (MMOSpaceTypes.h)

- **Resources**: Metal, Crystal, Deuterium, Energy, Food, Water, Credits, Dark Matter
- **Ships**: Light Fighter -> Ascended Warship (10 classes)
- **Buildings**: Metal Mine -> Shield Dome (14 types)
- **Research**: Weapons -> Economics (15 categories)
- **Government**: Anarchy -> Galactic Empire (10 types)
- **Planets**: Terran -> Artificial (13 types)

## Game Features

- **Procedural Galaxy**: 9 galaxies, 499 systems each, 15 planets per system
- **Real-time Economy**: Building-based resource production with energy balance
- **Fleet Combat**: Attack/defense matrix, formation bonuses, explosive ratios
- **Research Tree**: Prerequisite-based technology progression
- **Empire System**: Leveling, prestige, 10 government types with distinct bonuses
- **Market/Trading**: Buy/sell orders, dynamic pricing
- **SQL Persistence**: Full save/load with JSON fallback

## Setup

1. Enable plugin in `.uproject` Plugins list
2. Set game mode to `AMMOSpaceGameMode`
3. Configure `FUniverseConfig` in game mode defaults
4. Call `InitializeGameWorld()` on begin play

## Dependencies

- MMOCore (base engine plugin)
- GameplayTags, OnlineSubsystem (engine modules)
