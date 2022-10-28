all: build

FLAGS = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations 		\
		-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion	\
		-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wmissing-field-initializers		\
		-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel  	\
		-Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_SIDE

build:  obj/main.o obj/list.o obj/generals.o obj/log_errors.o 
	g++ obj/main.o obj/list.o obj/generals.o obj/log_errors.o  -o list


obj/list.o: list.cpp list.h config_list.h
	g++ list.cpp -c -o obj/list.o $(FLAGS)

obj/main.o: main.cpp list.h
	g++ main.cpp -c -o obj/main.o $(FLAGS)


obj/log_errors.o: src/log_info/log_errors.h src/log_info/log_errors.cpp
	g++ src\log_info\log_errors.cpp -c -o obj/log_errors.o $(FLAGS)


obj/generals.o: src\Generals_func\generals.cpp
	g++ src\Generals_func\generals.cpp -c -o obj/generals.o $(FLAGS)


.PHONY: cleanup mkdirectory

mkdirectory:
	 mkdir -p obj

cleanup:
	rm *.o list