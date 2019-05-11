PHONY: clean build

build: clean
	c++ -o handmade code/osx_handmade.cpp `sdl2-config --cflags --libs`

dbuild: clean
	c++ -g -o handmade code/osx_handmade.cpp `sdl2-config --cflags --libs`

clean:
	$(RM) build/*
	$(RM) -r handmade.dSYM
	$(RM) handmade
