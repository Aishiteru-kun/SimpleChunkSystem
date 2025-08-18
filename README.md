# SimpleChunkSystem

SimpleChunkSystem is a lightweight Unreal Engine plugin that provides a basic framework for grid-based world streaming and chunk management. It exposes a set of C++ classes and Blueprint nodes that make it easy to create and manipulate chunks of the game world.

## Features
- **ChunkSubsystem** – a `GameInstance` subsystem that lets you create and manage chunk managers keyed by name.
- **ChunkManagerBase** – an abstract manager class that encapsulates a concrete chunk system implementation.
- **ChunkSystemBase** – a template system that maintains a set of chunks and handles coordinate transforms and serialization.
- **Dynamic data support** – sample implementations `UChunkManager_DynamicData` and `FChunkSystem_DynamicData` allow storing custom structures in chunk cells.

## Installation
1. Copy the `SimpleChunkSystem` folder into your project's `Plugins` directory.
2. Open the project and let the engine compile the plugin.

## Usage
1. Add `ChunkSubsystem` to your `GameInstance`.
2. Use Blueprint or C++ to create chunk managers via `CreateChunkManager` or `FindOrCreateChunkManager`.
3. Use the managers to spawn or remove chunks and to work with dynamic data.

## Development
- The plugin is written in C++17 and uses standard Unreal Engine macros.
- Unit tests are located in `Source/SimpleChunkSystem/Public/UnitTest`.

## License
The project currently has no specified license.
