# compiler
CXX ?= g++

# source files directory
SRC_DIR = ./src

# program name
PROG = lv2bm

# default install paths
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# default compiler and linker flags
CFLAGS = -O3 -Wall -Wextra -c
LDFLAGS = -s

# debug mode compiler and linker flags
ifeq ($(DEBUG), 1)
   CFLAGS = -O0 -g -Wall -Wextra -c -DDEBUG
   LDFLAGS =
endif

# library links
LIBS = -lpthread -lrt `pkg-config --libs lilv-0` `pkg-config --libs glib-2.0` -lsndfile

# additional include paths
INCS = `pkg-config --cflags lilv-0` `pkg-config --cflags glib-2.0`

# remove command
RM = rm -f

# source and object files
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:.cpp=.o)

# linking rule
$(PROG): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(PROG) $(LIBS)

# meta-rule to generate the object files
%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCS) -o $@ $<

# install rule
install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(PROG) $(DESTDIR)$(BINDIR)

# clean rule
clean:
	$(RM) $(SRC_DIR)/*.o $(PROG)
