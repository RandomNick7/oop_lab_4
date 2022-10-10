//KNOWN BUG: If animals die (generating corpses) while corpses finish decaying (removing corpses), pointers might fuck up

//ALL elements have nothing in common! The "Root" class remains empty.
class Entity{};

//Represents non-living things, such as Tiles and Clouds
class Environment: public Entity{
	public:
		float water = 0;
};

// The building block of the world. Things can exist on them
class Tile: public Environment{
	public:
		int altit = rand()%(3000+1)-500;		//[-500,2500]
		float water = rand()%(6000+1)/10+300;	//[300,900]
		float temp = rand()%(420+1)/10-11;		//[-11,31]
		int entity = 0;
};

// Entity used for water storage and transportation across the world
class Cloud: public Environment{
	public:
		//When over a tile, rain down some amount of water (max 25%);
		void rain(Tile* ground){
			//The higher the altitude and the colder it is, the more it should rain
			float qty = water/4*(ground->altit+500)/3000*(35-ground->temp)/50;
			if((qty > 10) && (ground->water+qty)<=1000){
				water-=qty;
				ground->water+=qty;
			}
		}
};

//Anything that can contain energy is regarded to as "living"
class Life: public Entity{
	public:
		int energy;
};

//Includes living things that can grow anywhere and are numerically contained to a single tile, such as Plants and Fungi
class Grower: public Life{
	public:
		int num=0;
		virtual void multiply(Tile* ground) = 0;
		void absorb(Tile* ground){
			if(ground->water-num>20 && energy<10){
				ground->water-=num/5+0.5;
				energy+=num;
			}else{
				if(energy>0){
					energy--;
				}else if(num>0){
					num--;
					ground->water+=3;
					energy++;
				}
			}
		}
};

//Anything with health and which can die is regarded as a "Creature". This includes Trees and Animals
class Creature: public Life{
	protected:
		int id;
	public:
		int index;
		int health;
		int x;
		int y;
};

//A grown up plant. Generates clouds through transpiration and food for animals
class Tree: public Creature{
	public:
		int food;
		
		Tree(int i, int j, int s){
			id = 1;
			health = 15;
			energy = 20;
			index = s;
			x=j;
			y=i;
		};
		
		//If the tree has no health, remove it
		void death(Tile* ground, vector<Tree>* trees){
			if(health <= 0){
				ground->entity = 0;
				ground->water += 8+food;
				for(int i=index;i<(*trees).size();i++){
					(*trees)[i].index--;
				}
				trees->erase(trees->begin()+index);
			}
		}
		
		//Convert water to energy 
		void absorb(Tile* ground, vector<Tree>* trees){
			if(ground->water-8>50 && energy<30){
				ground->water-=8;
				energy++;
			}else{
				if(energy>0){
					energy--;
				}else{
					health--;
					death(ground,trees);
				}
			}
		}
		
		//Generate food that can be eaten by other animals
		void fruit(){
			if(energy>10 && food<4){
				energy-=4;
				food++;
			}
		}
		
		//Convert energy to potential water
		void transpirate(Cloud* sky){
			if(energy>0 && sky->water<1000){
				sky->water+=12;
				energy--;
			}
		}
};

//The primordial "living" entity. Uses water to grow into trees. Serves as a food source for some animals 
class Plant: public Grower{
	public:
		//Grow based on existing energy and waterity
		void multiply(Tile* ground){
			if(energy>5 && num<=10 && rand()%100<(ground->water-300)/100 && ground->entity==0){
				num++;
				energy-=3+num/5;
			}
		}
		
		//Evolve into a tree once a threshold has been reached
		void grow(Tile* ground, vector<Tree>* trees, int i, int j){
			if(num>=10 && energy>=20 && ground->entity==0){
				num-= 10;
				energy-=20;
				ground->entity = 1;
				(*trees).push_back(*(new Tree(i,j,(*trees).size())));
			}
		}
};

//The Plant's evil twin. Grows in water spots and near corpses. Serves as a food source.
class Fungi: public Grower{
	public:
		void multiply(Tile* ground){
			if(energy>3+num && num<=5 && rand()%100<(ground->water-400)/100){
				num++;
				energy-=3+num;
			}
		}
};

//An animal's remains. Stationary entity that acts as a limited-time food source
class Corpse: public Creature{
	private:
		int id = 2;
	public:
		Corpse(Tile* ground, int n, int h, int pos_x, int pos_y){
			index = n;
			health = h;
			ground->entity=id;
			x = pos_x;
			y = pos_y;
		}
		
		void decay(Tile* ground, vector<Corpse>* bodies){
			if(health <= 0){
				ground->water+=20;
				ground->entity=0;
				//Remove this body from the vector storing them
				for(int i=index;i<bodies->size();i++){
					(*bodies)[i].index--;
				}
				bodies->erase(bodies->begin()+index);
			}
		}
};

class Animal: public Creature{
	protected:
		int range;
		//Implementing A* would be overkill and would take a lot of time, so we're going with the budget "greedy" version instead
		//Here, when looking for a path, it will go in the shortest direction available
		//If the path straight ahead is occupied, it will look for free neighboring tiles to that destination
		void move(Tile map[H][W], int id, int* x, int* y, int dist_x, int dist_y){
			int x_new = *x;
			int y_new = *y;
			if(dist_x == 0){
				//Horizontal movement
				y_new = (dist_y/abs(dist_y)+*y)%H;
				if(map[y_new][x_new].entity!=0){
					//If tile is occupied, move to the right
					x_new = (x_new+1)%H;
				}
				if(map[y_new][x_new].entity!=0){
					//If that tile is occupied too, move to the left instead
					x_new = (x_new-2)%H;
				}
			}else if(dist_y == 0){
				//Vertical movement
				x_new=(dist_x/abs(dist_x)+*x)%W;
				if(map[y_new][x_new].entity!=0){
					//If tile is occupied, move above it
					y_new = (y_new+1)%H;
				}
				if(map[y_new][x_new].entity!=0){
					//If that tile is occupied too, move below instead
					y_new = (y_new-2)%H;
				}
			}else{
				//Diagonal movement
				int sgn_x = dist_x/abs(dist_x);
				int sgn_y = dist_y/abs(dist_y);
				x_new = (sgn_x+*x)%W;
				y_new = (sgn_y+*y)%H;
				if(map[y_new][x_new].entity!=0){
					y_new = (y_new-sgn_y)%H;
				}
				if(map[y_new][x_new].entity!=0){
					y_new = (y_new+sgn_y)%H;
					x_new = (x_new+sgn_x)%W;
				}
			}
			if(map[y_new][x_new].entity!=0){
				//If all potential tiles are occupied, don't move
				x_new = *x;
				y_new = *y;
			}
			//Remove self from current tile
			map[*y][*x].entity = 0;
			//Place self on new tile
			*x = x_new;
			*y = y_new;
			map[*y][*x].entity = id;
		}
	public:
		int attack;
};

//Animals that consume flora
class Herbivore: public Animal{
	public:
		Herbivore(int i,int pos_x, int pos_y){
			id = 3;
			health = 20;
			attack = 2;
			range = 3;
			x = pos_x;
			y = pos_y;
			index = i;
		}
		//Consume adjacent trees. If none nearby, look for one within 3 tiles distance
		void target(Tile map[H][W], vector<Tree>* trees){
			int target_found = 0;
			for(int i=0;i<trees->size();i++){
				if(abs(x-(*trees)[i].x)<=1 && abs(y-(*trees)[i].y)<=1){
					if((*trees)[i].energy<=0){
						(*trees)[i].health-=attack*12;
					}else{
						(*trees)[i].energy-=attack*12;
					}
					target_found = 1;
					break;
				}
			}
			if(target_found == 0){
				for(int i=0;i<trees->size();i++){
					if(abs(x-(*trees)[i].x)<=3 && abs(y-(*trees)[i].y)<=3){
						move(map,id,&x,&y,(*trees)[i].x-x,(*trees)[i].y-y);
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				move(map,id,&x,&y,rand()%3-1,rand()%3-1);
			}
		}
};

//Animals that primarily consume dead organic material
class Omnivore: public Animal{
	public:
		Omnivore(int i,int pos_x, int pos_y){
			id = 5;
			health = 50;
			attack = 6;
			range = 6;
			x = pos_x;
			y = pos_y;
			index = i;
		}
		//Consume food from adjacent trees. If none nearby, consume nearby herbivores. If none found, look for trees within 4 tiles. If none found, look for herbivores within 6 tiles.
		void target(Tile map[H][W],vector<Herbivore>* a_herb, vector<Tree>* trees){
			int target_found = 0;
			for(int i=0;i<trees->size();i++){
				if(abs(x-(*trees)[i].x)<=1 && abs(y-(*trees)[i].y)<=1){
					if((*trees)[i].food>0){
						(*trees)[i].food-=2;
						target_found = 1;
					}else{
						target_found = 0;
					}
					break;
				}
			}
			if(target_found == 0){
				for(int i=0;i<a_herb->size();i++){
					if(abs(x-(*a_herb)[i].x)<=1 && abs(y-(*a_herb)[i].y)<=1){
						(*a_herb)[i].health-=attack;
						health-=(*a_herb)[i].attack;
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				for(int i=0;i<a_herb->size();i++){
					if(abs(x-(*trees)[i].x)<=range-2 && abs(y-(*trees)[i].y)<=range-2){
						move(map,id,&x,&y,(*trees)[i].x-x,(*trees)[i].y-y);
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				for(int i=0;i<a_herb->size();i++){
					if(abs(x-(*a_herb)[i].x)<=range && abs(y-(*a_herb)[i].y)<=range){
						move(map,id,&x,&y,(*a_herb)[i].x-x,(*a_herb)[i].y-y);
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				move(map,id,&x,&y,rand()%3-1,rand()%3-1);
			}
		}
};

//Animals that consumes other animals
class Carnivore: public Animal{
	public:
		Carnivore(int i,int pos_x, int pos_y){
			id = 4;
			health = 40;
			attack = 8;
			range = 5;
			x = pos_x;
			y = pos_y;
			index = i;
		}
		//Consume adjacent herbivores. If none, consume adjacent omnivores. If none found, look for herbivores within 6 tiles. Otherwise, Omnivores within 5 tiles.
		void target(Tile map[H][W],vector<Herbivore>* a_herb, vector<Omnivore>* a_omni){
			int target_found = 0;
			for(int i=0;i<a_herb->size();i++){
				if(abs(x-(*a_herb)[i].x)<=1 && abs(y-(*a_herb)[i].y)<=1){
					(*a_herb)[i].health-=attack;
					health-=(*a_herb)[i].attack;
					target_found = 1;
					break;
				}
			}
			if(target_found == 0){
				for(int i=0;i<a_omni->size();i++){
					if(abs(x-(*a_omni)[i].x)<=1 && abs(y-(*a_omni)[i].y)<=1){
						(*a_omni)[i].health-=attack;
						health-=(*a_omni)[i].attack;
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				for(int i=0;i<a_herb->size();i++){
					if(abs(x-(*a_herb)[i].x)<=range+1 && abs(y-(*a_herb)[i].y)<=range+1){
						move(map,id,&x,&y,(*a_herb)[i].x-x,(*a_herb)[i].y-y);
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				for(int i=0;i<a_omni->size();i++){
					if(abs(x-(*a_omni)[i].x)<=range && abs(y-(*a_omni)[i].y)<=range){
						move(map,id,&x,&y,(*a_omni)[i].x-x,(*a_omni)[i].y-y);
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				move(map,id,&x,&y,rand()%3-1,rand()%3-1);
			}
		}
};

//Animals that consume the dead
class Detritivore: public Animal{
	public:
		Detritivore(int i,int pos_x, int pos_y){
			id = 6;
			health = 20;
			attack = 4;
			range = 4;
			x = pos_x;
			y = pos_y;
			index = i;
		}
		//Consume adjacent bodies. If none found, look for them within 4 tiles away.
		void target(Tile map[H][W],vector<Detritivore>* a_detr, vector<Corpse>* bodies){
			int target_found = 0;
			for(int i=0;i<bodies->size();i++){
				if(abs(x-(*bodies)[i].x)<=1 && abs(y-(*bodies)[i].y)<=1){
					(*bodies)[i].health-=attack*5;
					(*bodies)[i].decay(&map[(*bodies)[i].y][(*bodies)[i].x],bodies);
					target_found = 1;
					break;
				}
			}
			if(target_found == 0){
				for(int i=0;i<bodies->size();i++){
					if(abs(x-(*bodies)[i].x)<=range+1 && abs(y-(*bodies)[i].y)<=range+1){
						move(map,id,&x,&y,(*bodies)[i].x-x,(*bodies)[i].y-y);
						target_found = 1;
						break;
					}
				}
			}
			if(target_found == 0){
				move(map,id,&x,&y,rand()%3-1,rand()%3-1);
			}
		}
};
