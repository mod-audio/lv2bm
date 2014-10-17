# compiler
CC = g++

# linker
LD = g++

# language file extension
EXT = cpp

# source files directory
SRC_DIR = ./src

# program name
PROG = lv2bm

# default install paths
PREFIX = /usr/local
INSTALL_PATH = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1/

# default compiler and linker flags
CFLAGS = -O3 -Wall -Wextra -c
LDFLAGS = -s

# debug mode compiler and linker flags
ifeq ($(DEBUG), 1)
   CFLAGS = -O0 -g -Wall -Wextra -c -DDEBUG
   LDFLAGS =
endif

# library links
LIBS = -lpthread -lrt `pkg-config --libs lilv-0` `pkg-config --libs glib-2.0`

# additional include paths
INCS = `pkg-config --cflags lilv-0` `pkg-config --cflags glib-2.0`

# remove command
RM = rm -f

# source and object files
SRC = $(wildcard $(SRC_DIR)/*.$(EXT))
OBJ = $(SRC:.$(EXT)=.o)

# linking rule
$(PROG): get_info $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(PROG) $(LIBS)
	@rm src/info.h

# meta-rule to generate the object files
%.o: %.$(EXT)
	$(CC) $(CFLAGS) $(INCS) -o $@ $<

# install rule
install:
	install -d $(INSTALL_PATH)
	install $(PROG) $(INSTALL_PATH)

# clean rule
clean:
	$(RM) $(SRC_DIR)/*.o $(PROG) src/info.h

# get info rule
get_info:
	@echo "const char version[] = {\""`git describe --tags`\""};" > src/info.h