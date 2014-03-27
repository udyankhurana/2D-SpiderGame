CC = g++

TARGET = arac

COMPILEFLAGS = -lopenal -lalut -lglut -lGL -lGLU

$(TARGET): arac.o
	$(CC) -o $(TARGET) arac.o $(COMPILEFLAGS)

clean:
	rm -f $(TARGET) arac.o
