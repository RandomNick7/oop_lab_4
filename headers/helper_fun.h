//Helper functions for copying auxiliary arrays into original ones and vice versa
void cpy_world(Tile a[H][W],Tile b[H][W]){
	for (int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			a[i][j] = b[i][j];
		}
	}
}

void cpy_sky(Cloud a[H][W],Cloud b[H][W]){
	for (int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			a[i][j] = b[i][j];
		}
	}
}
