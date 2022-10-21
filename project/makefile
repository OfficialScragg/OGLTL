main:
	gcc src/project.c src/glad.c -Wall -I include -g $(pkg-config --cflags opengl glfw3) -lOpenGL -lm -lglfw -ldl -o bin/project

run: main
	./bin/project

clean:
	rm ./bin/project
