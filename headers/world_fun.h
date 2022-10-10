//Below functions are not part of classes as they're meant to apply to the entire map at once

//Make the world terrain be more smooth
void smoothen(Tile world[H][W]){
	//For simplicity, the world loops around the edges
	Tile new_world[H][W];
	cpy_world(&new_world[0],&world[0]);
	
	for(int i=0;i<H;i++){						//Take the average values of a tile's Moore neighborhood
		for(int j=0;j<W;j++){
			for(int k=0;k<9;k++){
				if(k==4){
					continue;
				}
				Tile curr = world[(i+(k/3)-1+H)%H][(j+(k%3)-1+W)%W];
				new_world[i][j].altit += curr.altit;
				new_world[i][j].temp += curr.temp;
				new_world[i][j].water += curr.water;
			}
			new_world[i][j].altit /= 9;
			new_world[i][j].temp = (new_world[i][j].temp/9)-(new_world[i][j].altit-1000)/250;	//The higher the tile, the colder it gets: [-500,2500] -> [-4,+4]
			new_world[i][j].water /= 9;
		}
	}
	cpy_world(&world[0],&new_world[0]);
}

//Spawns in Animals to interact with the world
void animal_gen(Tile world[H][W], vector<Tree> trees, vector<Corpse> bodies, vector<Herbivore>* a_herb, vector<Carnivore>* a_carn, vector<Omnivore>* a_omni, vector<Detritivore>* a_detr){
	if(rand()%750<trees.size()){
		int x = rand()%W;
		int y = rand()%H;
		if(world[y][x].entity == 0){
			a_herb->push_back(*(new Herbivore(a_herb->size(),x,y)));
			world[y][x].entity = 3;
		}
	}
	if(rand()%3000<(a_herb->size()+a_omni->size())){
		int x = rand()%W;
		int y = rand()%H;
		if(world[y][x].entity == 0){
			a_carn->push_back(*(new Carnivore(a_carn->size(),x,y)));
			world[y][x].entity = 4;
		}
	}
	if(rand()%4000<(a_herb->size()+trees.size())){
		int x = rand()%W;
		int y = rand()%H;
		if(world[y][x].entity == 0){
			a_omni->push_back(*(new Omnivore(a_omni->size(),x,y)));
			world[y][x].entity = 5;
		}
	}
	if(rand()%250<bodies.size()){
		int x = rand()%W;
		int y = rand()%H;
		if(world[y][x].entity == 0){
			a_detr->push_back(*(new Detritivore(a_detr->size(),x,y)));
			world[y][x].entity = 6;
		}
	}
}

//Simulate water flowing from one tile to other nearby ones
void water_transfer(Tile world[H][W]){
	Tile new_world[H][W];
	cpy_world(&new_world[0],&world[0]);
	//Spread the values to a tile's Moore neighborhood
	for(int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			for(int k=0;k<9;k++){
				if(k==4){
					continue;
				}
				Tile* curr = &new_world[(i+(k/3)-1+H)%H][(j+(k%3)-1+W)%W];
				float delta = (world[i][j].water+world[i][j].altit - (curr->water+curr->altit))/5;
				if(delta > 0 && new_world[i][j].water-delta>0 && curr->water+delta<1000 && new_world[i][j].water>(2500-world[i][j].altit)/4){
					new_world[i][j].water-=delta;
					curr->water+=delta;
				}
			}
		}
	}
	cpy_world(&world[0],&new_world[0]);
}

//Start creating clouds from evaporating water
void form_weather(Tile world[H][W], Cloud sky[H][W]){
	Cloud new_sky[H][W];
	cpy_sky(&new_sky[0],&sky[0]);
	
	for(int i=0;i<H;i++){
		for(int j=0;j<W;j++){	//Tiles that are warm and water enough produce clouds above them
			if(world[i][j].water>350){
				float qty = (world[i][j].water-350)/5*(world[i][j].temp)/10;
				if(new_sky[i][j].water+qty<1000){
					new_sky[i][j].water+=qty;
					world[i][j].water-=qty;
				}
			}
		}
	}
	
	cpy_sky(&sky[0],&new_sky[0]);
	
	for(int i=0;i<H;i++){							//Spread the values to a cloud's von Neumann neighborhood
		for(int j=0;j<W;j++){
			if(sky[i][j].water>0){
				new_sky[i][j].rain(&world[i][j]);	//Let a portion of the water in the cloud rain down on the ground
				float qty = sky[i][j].water/20;
				new_sky[(i-1+H)%H][j].water+=qty;
				new_sky[(i+1)%H][j].water+=qty;
				new_sky[i][(j-1+W)%W].water+=qty;
				new_sky[i][(j+1)%W].water+=qty;
				new_sky[i][j].water-=qty*4;
			}
		}
	}
	
	cpy_sky(&sky[0],&new_sky[0]);
}

//Create, grow and multiply grass each tick
//Check if the grass can grow into trees each time
void grow_grass(Tile world[H][W], Plant grass[H][W], vector<Tree>* trees){
	for(int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			grass[i][j].absorb(&world[i][j]);
			grass[i][j].multiply(&world[i][j]);
			if(grass[i][j].num == 0 && world[i][j].water > 300 && world[i][j].temp > 0){
				grass[i][j].num++;
			}
			grass[i][j].grow(&world[i][j], trees, i, j);
		}
	}
}

//Make trees generate clouds, food and energy, if possible
void grow_trees(Tile world[H][W], Cloud sky[H][W], vector<Tree>* trees){
	for(int i=0;i<(*trees).size();i++){
		Tree t = (*trees)[i];
		t.transpirate(&sky[t.y][t.x]);
		t.absorb(&world[t.y][t.x],trees);
		t.fruit();
	}
}

//Generate and multiply mushrooms on high water Tiles
void grow_shrooms(Tile world[H][W], Fungi shrooms[H][W]){
	for(int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			shrooms[i][j].absorb(&world[i][j]);
			shrooms[i][j].multiply(&world[i][j]);
			if(shrooms[i][j].num==0 && world[i][j].water>500){
				shrooms[i][j].num++;
			}
		}	
	}
}

void animal_logic(Tile world[H][W], vector<Herbivore>* a_herb, vector<Carnivore>* a_carn, vector<Omnivore>* a_omni, vector<Detritivore>* a_detr, vector<Tree>* trees, vector<Corpse>* bodies){
	for(int i=0;i<a_herb->size();i++){
		Herbivore* h = &(*a_herb)[i];
		h->target(&world[0],trees);
	}
	if(rand()%100<a_herb->size()){
		int x = (*a_herb)[0].x;
		int y = (*a_herb)[0].y;
		(*a_herb).erase(a_herb->begin());
		world[y][x].water+=40;
		world[y][x].entity = 0;
		(*bodies).push_back(*(new Corpse(&world[y][x],(*bodies).size(),20,x,y)));
	}
	for(int i=0;i<a_carn->size();i++){
		Carnivore* c = &(*a_carn)[i];
		c->target(&world[0],a_herb,a_omni);
	}
	if(rand()%200<a_carn->size()){
		int x = (*a_carn)[0].x;
		int y = (*a_carn)[0].y;
		(*a_carn).erase(a_carn->begin());
		world[y][x].water+=70;
		world[y][x].entity = 0;
		(*bodies).push_back(*(new Corpse(&world[y][x],(*bodies).size(),35,x,y)));
	}
	
	for(int i=0;i<a_omni->size();i++){
		Omnivore* o = &(*a_omni)[i];
		o->target(&world[0],a_herb,trees);
	}
	if(rand()%175<a_omni->size()){
		int x = (*a_omni)[0].x;
		int y = (*a_omni)[0].y;
		(*a_omni).erase(a_omni->begin());
		world[y][x].water+=60;
		world[y][x].entity = 0;
		(*bodies).push_back(*(new Corpse(&world[y][x],(*bodies).size(),30,x,y)));
	}
	
	for(int i=0;i<a_detr->size();i++){
		Detritivore* d = &(*a_detr)[i];
		d->target(&world[0],a_detr,bodies);
	}
	if(rand()%150<a_detr->size()){
		int x = (*a_detr)[0].x;
		int y = (*a_detr)[0].y;
		(*a_detr).erase(a_detr->begin());
		world[y][x].water+=50;
		world[y][x].entity = 0;
	}
}

//Warning! Make sure the terminal can fit everything, lest you will have ugly persisting rows being drawn repeatedly
void show_world(Tile world[H][W], Cloud sky[H][W], Plant grass[H][W], Fungi shrooms[H][W]){
	cout << "\033[0mTimer:" << timer << endl;
	//Display 1st row of "screens"
	cout << "\033[0mTerrain: Humidity, Temperature, Altitude\n";
	for(int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			//Tile water:
			printf("\033[38;2;0;0;%dm%s",((int)world[i][j].water)*255/1000, "█");
		}
		cout << "\t";
		for(int j=0;j<W;j++){
			//Tile temperature:
			printf("\033[38;2;%d;0;0m%s",((int)world[i][j].temp+15)*255/50,"█");
		}
		cout << "\t";
		for(int j=0;j<W;j++){
			//Tile altitude:
			printf("\033[38;2;%d;%d;0m%s",(world[i][j].altit+500)*255/3000,(world[i][j].altit+500)*255/3000,"█");
		}
		cout << endl;
	}
	
	//Display 2nd row of "screens"
	cout << "\033[0mOthers: Clouds, Living Entities\n";
	for(int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			printf("\033[38;2;%d;%d;%dm%s",((int)sky[i][j].water)*255/1000,((int)sky[i][j].water)*255/1000,((int)sky[i][j].water)*255/1000, "█");
		}
		cout << "\t";
		for(int j=0;j<W;j++){
			switch(world[i][j].entity){
				case 1:
					//Trees:
					printf("\033[38;2;38;208;124m%s","█");
					break;
				case 2:
					//Bodies:
					printf("\033[38;2;75;75;75m%s","█");
					break;
				case 3:
					//Herbivores:
					printf("\033[38;2;127;255;0m%s","█");
					break;
				case 4:
					//Carnivores:
					printf("\033[38;2;220;20;60m%s","█");
					break;
				case 5:
					//Omnivores:
					printf("\033[38;2;252;127;3m%s","█");
					break;
				case 6:
					//Detritivores:
					printf("\033[38;2;255;194;0m%s","█");
					break;
				default:
					//Grass & Fungi:
					printf("\033[38;2;%d;%d;0m%s",shrooms[i][j].num*255/8,(shrooms[i][j].num+grass[i][j].num)*255/16,"░");
			}
		}
		cout << endl;
	}
	
	for(int i=0;i<=2*H+2;i++){
		//Moves cursor up to the 1st line for overwriting
		printf("\033[A");
	}
}

