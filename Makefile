TARGET		:= uwpflash
DIST_TARGET := /usr/bin/uwpflash
LD_FLAGS	:= -lpthread

objects := $(patsubst %.c,%.o,$(wildcard *.c))

$(TARGET) : $(objects)
	$(CC) -o $(TARGET) $(objects) $(LD_FLAGS)

.PHONY:clean
clean :
	@ $(RM) -rf $(TARGET) $(objects)

install:
	install $(TARGET) $(DIST_TARGET)
