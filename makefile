# build directory
BD=build
# c file directory
CD=./src
# header file directory
HD=./src

# complier
CC=gcc

# external libraries
LIBRARIES=-lc

CFILES:=$(wildcard $(CD)/*.c)
OFILES:=$(patsubst $(CD)%,$(BD)%,$(patsubst %.c,%.o,$(CFILES)))
HFILES:=$(wildcard $(HD)/*.h)

output: $(BD) $(OFILES) $(HFILES)
	$(CC) $(LIBRARIES) $(OFILES) -o $(BD)/$@ -g

$(OFILES): $(BD)/%.o: $(CD)/%.c
	$(CC) $(LIBRARIES) -c $< -o $@ -g

$(BD):
	mkdir $(BD)

.PHONY: clean
clean:
	rm $(BD)/output
	rm $(OFILES)
	rmdir $(BD)
