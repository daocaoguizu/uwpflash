BIN2ARRAY	:= bin2array
UWPFLASH	:= uwpflash
TARGET		:= $(BIN2ARRAY) $(UWPFLASH)
DIST_TARGET := /usr/bin/uwpflash
LD_FLAGS	:= -lpthread

UWPFLASH_objs  := uwpflash.o command.o uart.o download.o interface.o crc.o crc16.o
BIN2ARRAY_objs := bin2array.o

all : $(TARGET)

$(BIN2ARRAY) : $(BIN2ARRAY_objs)
	$(CC) -o $(BIN2ARRAY) $(BIN2ARRAY_objs)

fdl.h : fdl.bin
	./$(BIN2ARRAY)

$(UWPFLASH) : $(UWPFLASH_objs) fdl.h
	$(CC) -o $(UWPFLASH) $(UWPFLASH_objs) $(LD_FLAGS)

.PHONY:clean
clean :
	@ $(RM) -rf $(TARGET) $(UWPFLASH_objs) $(BIN2ARRAY_objs)

install:
	install $(TARGET) $(DIST_TARGET)
