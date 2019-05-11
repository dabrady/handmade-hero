PHONY: clean build

build: clean
	c++ -g -o handmade code/osx_handmade.cpp

clean:
	$(RM) build/*
	$(RM) -r handmade.dSYM
	$(RM) handmade
