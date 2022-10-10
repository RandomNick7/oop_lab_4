all:
	g++ main.cpp -Iheaders -o "Simulation"
clear:
	find . -maxdepth 1 -type f -executable -delete
