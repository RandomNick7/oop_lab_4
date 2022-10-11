## Laboratory 4 - OOP

### Description

This simulation is supposed to represent some simple natural environment with generalized elements  
The simulation will mostly rely on interactions between the different entities

#### Entities:

* Tile - The most essential entity, most other entities reside on it. Has 3 main attributes:  
	- Water - The quanitity of water in the area. Sustains life.  
	- Temperature - Influences water evaporation  
	- Altitude - Influences water flow and precipitation  
Tiles also store the information regarding what kind of entity is currently on it  
* Cloud - An entity that helps redistribute water across Tiles, behaves differently to the Tile's water flow  
* Plant - The most common lifeform, grows on tiles with enough water and proper heat  
* Tree - A "grown up" plant, generates food for other animals and a small amount of clouds via transpiration  
* Fungi - Similar to Plants, require more humidity and can feed on the dead  
* Corpses - The remains of a deceased animal. Can be used as a food source for other entities  
* Animals:  
	- Detritivores - Animals that feed from the dead  
	- Herbivores - Animals that feed from plants  
	- Carnivores - Animals that feed from other animals, tend to be more aggressive  
	- Omnivores - Animals that feed from any of the above, less aggressive than carnivores but can still attack other animals  

This is the current class structure within the project:  
![Class Diagram](/images/UML_Diagram.png)  

### Progress:  

Fatal Bug: Segfaults may occasionally occur during simulation when Creatures (Animals/Trees/Corpses) are meant to be erased. **A fix is underway.**  
 
`Corpse` is now a descendant of `Creature` instead.  
Polymorphism was implemented only on 1 virtual method `multiply` from an abstract class `Grower`.  
Classes that are not instiated, such as `Entity`,`Environment`,`Life`,`Creature` hold no methods, so they cannot be abstract. A minimum of 1 virtual function is required for that (in C++).  
Although all the descendants of the `Animal` class have a function named `target`, they all use different parameters, making it unviable for polymorphism.  

The world has been enlarged to the point it barely fits on-screen.  
A timer has been added to the top, counting each iteration of the simulation.  
Water distribution now also takes into account altitude. Slightly more realistic results.  

Animals are now spawned randomly, depending on the quantity of things they consume present on the map:  
* Crimson = Carnivore  
* Amber = Omnivore  
* Honey = Detritivore  
* Lime = Herbivore  

Animals will seek out food within some radius of them and roam randomly if none is present. 
Animals die by random chance due to aforementioned bug.   
![Program execution screenie](/images/Terminal.png)    

The intensity of the colors above directly correspond to an increase in quantity of something.  
Different colors correspond to different entities on the last "screen".  
The colors have been added via ANSI escape codes.  

**Caution!** Make sure the program output fits inside the terminal window, or you will have a nasty trace left on-screen!  
For now, the program can only end execution with Ctrl+C.  
If I have enough time, I might finally use `ncurses` to eliminate the above 2 inconveiences.  
