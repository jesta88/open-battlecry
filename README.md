# Open Battlecry

Open source rewrite of the Warlords Battlecry series by Infinite Interactive.

## Project Structure

**Core**: Static library. Logging, debugging, maths, common OS stuff like threading and IO.  
**Game**: Dynamic library. Gameplay logic and physics.  
**GameClient**: Executable. Window creation, OS events & input, resource loading, rendering and audio. Also sends input to server in multiplayer.  
**GameServer**: Executable. Receives input from all clients, validates game state and sends back commands to clients.  
