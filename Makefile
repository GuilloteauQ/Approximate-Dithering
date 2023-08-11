PROG = make_bin_folder dithering_bw dithering_bw.s

all : $(PROG)

CC        =  gcc
CFLAGS    =  -Wall -Wextra -O3 -ffast-math
LDFLAGS   =  -lm -O3
SRC_FOLDER = src
TARGET_FOLDER = bin

make_bin_folder:
	mkdir -p bin > /dev/null

dithering_bw: $(TARGET_FOLDER)/dithering_bw.o $(TARGET_FOLDER)/Util.o $(TARGET_FOLDER)/image_lib.o
	$(CC) -o $@ $^ $(LDFLAGS)

dithering_bw.s: $(SRC_FOLDER)/dithering_bw.c
	$(CC) -S -o $@ $^ $(LDFLAGS)

$(TARGET_FOLDER)/%.o: $(SRC_FOLDER)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean :
	@rm -f *.o $(PROG) *.pgm
	@rm -rf $(TARGET_FOLDER)
