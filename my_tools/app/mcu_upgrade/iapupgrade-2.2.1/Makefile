CFLAGS =  -Wall -O2 -g -DDEBUG -Wl,-Map,gen_img.map
RELEASE = ./release/
TEST = ./test/
CC = gcc 
CPPFLAGS = 
INCLUDES = -I.
LDFLAGS = -L./libs/32 -lstdc++ -lacfa
TARGET :=  iapupgrade
OBJECTS =  xmodem.c iapupgrade.c


all: $(TARGET) gen_img
$(TARGET): $(OBJECTS)
	@echo "Build $(TARGET)..."
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $(RELEASE)$(TARGET)  
	@echo ""
%.o : /%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@ 
gen_img: clean_gen_img
	@echo "Build $(TARGET)..."
	$(CC) $(CFLAGS) $(INCLUDES)  xmodem.c gen_img.c  xmodem.h gen_img.h aes.h aes.c -o $(TEST)$@  $(LDFLAGS)
	@echo ""

clean:
	rm -f *.o  $(TARGET) $(TEST)gen_img $(RELEASE)*
clean_gen_img:
	@rm -f gen_img

