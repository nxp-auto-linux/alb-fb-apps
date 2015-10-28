######################
#	COMPILER
######################
CC=$(CROSS_COMPILE)gcc
LINUX_INCLUDE_DIR:="replace/this/with/the/path/to/your/linux-headers/dir"
CFLAGS=-I$(LINUX_INCLUDE_DIR)

######################
#	PATHS
######################
BUILD=build
SRC_APP=src/app
SRC_LIB=src/lib

######################
#	RULES
######################
all: $(BUILD)/fb_chess $(BUILD)/fb_gradient $(BUILD)/fb_transition $(BUILD)/fb_chromakey $(BUILD)/fb_display

$(BUILD):
	mkdir $(BUILD)

$(BUILD)/fb_chess: $(SRC_APP)/fb_chess.c $(BUILD)/fb_helper.o | $(BUILD)
	$(CC) $(CFLAGS) $(BUILD)/fb_helper.o $(SRC_APP)/fb_chess.c -o $(BUILD)/fb_chess

$(BUILD)/fb_gradient: $(SRC_APP)/fb_gradient.c $(BUILD)/fb_helper.o | $(BUILD)
	$(CC) $(CFLAGS) $(BUILD)/fb_helper.o $(SRC_APP)/fb_gradient.c -o $(BUILD)/fb_gradient

$(BUILD)/fb_transition: $(SRC_APP)/fb_transition.c $(BUILD)/fb_helper.o | $(BUILD)
	$(CC) $(CFLAGS) $(BUILD)/fb_helper.o $(SRC_APP)/fb_transition.c -o $(BUILD)/fb_transition

$(BUILD)/fb_chromakey: $(SRC_APP)/fb_chromakey.c $(BUILD)/fb_helper.o | $(BUILD)
	$(CC) $(CFLAGS) $(BUILD)/fb_helper.o $(SRC_APP)/fb_chromakey.c -o $(BUILD)/fb_chromakey
	
$(BUILD)/fb_display: $(SRC_APP)/fb_display.c $(BUILD)/fb_helper.o | $(BUILD)
	$(CC) $(CFLAGS) $(BUILD)/fb_helper.o $(SRC_APP)/fb_display.c -o $(BUILD)/fb_display

$(BUILD)/fb_helper.o: $(SRC_LIB)/fb_helper.c $(SRC_LIB)/fb_helper.h | $(BUILD)
	$(CC) -c $(CFLAGS) $(SRC_LIB)/fb_helper.c -o $(BUILD)/fb_helper.o

clean:
	rm -rf $(BUILD)/*
