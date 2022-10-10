//BUG: Death of Animals/Trees/Bodies cause Segfauls & illegal pointers occasionally...
//A Refactor would be in order, but that would take quite some time

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

using namespace std;

#define W 50		//World Width
#define H 17		//World Height (2D)

int timer = 0;

#include "class.h"
#include "helper_fun.h"
#include "world_fun.h"

int main(){
	srand(time(0));
	Tile world[H][W];
	Cloud sky[H][W];
	Plant grass[H][W];
	Fungi shrooms[H][W];
	vector<Tree> trees;
	vector<Corpse> bodies;
	vector<Herbivore> a_herb;
	vector<Carnivore> a_carn;
	vector<Omnivore> a_omni;
	vector<Detritivore> a_detr;
	
	smoothen(world);
	
	while(true){
		water_transfer(world);
		form_weather(&world[0],sky);
		grow_grass(&world[0],&grass[0],&trees);
		grow_trees(&world[0],&sky[0],&trees);
		grow_shrooms(&world[0],&shrooms[0]);
		animal_gen(&world[0],trees,bodies,&a_herb,&a_carn,&a_omni,&a_detr);
		animal_logic(world,&a_herb,&a_carn,&a_omni,&a_detr,&trees,&bodies);
		show_world(world,sky,grass,shrooms);
		timer++;
		//Update 10 times per second
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}
