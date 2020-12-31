# Raider.io
Hello! This is Raider.io a pvp online game up to 20 players, where each one can choose between three different classes to fight their friends on an online arena.
Killing an enemy will increase the player level, making them bigger and stronger but slower!
If you get killed, you will lose your level and start from scratch, also, your killer will steal your level for themself.
Each class has it's own unique Spell, make sure to use them as they are very powerful!

### Berserker
This class throws axes at their enemies to deal damage and can summon a whirlwind of axes to spin around them as a special ability!

### Wizard
The wizard fights in the arena with their staff, throwing magic balls of energy to deal long range damage. As special ability, they can create an explosion of projectiles from withim themself, allowing area of effect damage.

### Hunter
The last character is the hunter, who fights with their bow shooting arrows at their enemies. Their special ability is a snipe, they can charge their shoot holding the special ability key and release a devastating ice spike. At max charge this can destroy even the thoughest rivals.

# Instructions
To play Raider.io one user has to be the server. To do this you can execute the game and select server mode on the main menu, make sure to put the port address you will be using.
An important note is that the user who hosts the server, needs to activate port forwarding for UDP on his router, if you don't know how to do that make sure to find a guide for your router brand that explains how to set it up.

You can also play by yourself as LocalHost, but that's not as fun as playing with friends :)
Once the server is set up, the other players can join executing their game, inputting the correct port and the IP address of the host. At this point, you have to choose what class you will join the server with.
If everything went well, now you should be able to play Raider.io with your friends!

# Controls
* W A S D - Player movement
* Mouse Movement - Aim the weapon
* Mouse Left Click - Shoot
* SpaceBar - Special Ability

# Group Members
This game was done by [Axel Alavedra](https://github.com/AxelAlavedra "Axel's Github") and [Marc Guillen](https://github.com/Marcgs96 "Marc's Github"), under the group [Probox AM](https://github.com/ProboxAM "Probox AM's Github"), you can check more of our works there.
The following list will reflect what each member has done for this project. Note that the base implementation of the Online System was done by both of us, which will be under Common Features.

#### Common Features:
* Server/Client connection
* Data Serialization
* Connection Timeout: If the Server or the Client dont receive a Ping Packet in a set amount of time, a Timeout will be considered and a Disconnect will be executed. This works properly while dragging the game window also.
* World State Replication with Managers: Completely achieved, the Server sends replication packets to the Clients every set amount of time and the Clients Read those packets succesfully and replicate the state of the server.
* Reliability for UDP (Delivery Manager and Input Packet Redundancy): Input Packet Redundancy completely achieved, the Server will end up receiving all the Inputs even with packets lose. 
Delivery Manager completely achieved, it recognizes when a packet was or not succesfull and calls to the OnFailure or OnSuccess of the DeliveryDelegate.
* Client Side Prediction with Server Reconciliation for Movement: Completely achieved, the input of the Client feels smooth and reacts quick and when a Replication Packet is received by the Client, the state is recovered through Server Reconciliation reapplying all the inputs accordingly.
* Entity Interpolation: Completely achieved with a small bug, the Interpolation of the Weapon Angle has a minor bug when it interpolates at the end of circle, making it go quickly around the opposite way to the new angle.

#### Known bugs
* Rare bug which can make a Player never re-enable their collider after respawning.
* When two players kill each other at the same time, one will receive the Level from the other but the character's size will not increase.

### Axel's features:
#### Specific Features:
* Player Structure
* Player Movement
* Spawn Weapon and Link on Client
* Die/Respawn
* Class Selection
* Animation Serialization
* Client Side Prediction for Projectiles (Fake Projectile on Client): This creates a "Fake" projectile on the client for smoother inputs, this projectile will follow the trajectory it think the server will follow also, creating an ilusion for the client.
The fake projectile will get destroyed on contact with another Player but will not do any damage, as it is the Server Simulation of the Real Projectile who will deal with the damage event.
* Hunter Spell
* Handle Destruction of Child Network Objects of Client
* Level System
* Helped with Mouse Input Serialization
* Death Animation / Sound
* Class Helmets Art

### Marc's features:
#### Specific Features:
* Weapon Structure
* Projectile Structure
* Spawn Projectiles on Weapon Use
* Mouse Input Serialization: We Serialize Mouse Position mapped to World Coordinates, this way we can use the Mouse Position of the Client to Simulate on the Server without consistency problems.
* Weapon Rotation with Mouse Input
* Wizard Spell
* Berserker Spell
* Projectiles Art
* Weapons Art
