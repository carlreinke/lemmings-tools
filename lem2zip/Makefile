# BUILD SETTINGS ###################################
DEBUG := 1
# END SETTINGS #####################################

TARGET := lem2zip
OBJS := lem2zip.o compress.o decompress.o

ifeq ($(DEBUG), 1)
	DEBUG_FLAGS := -g3 -O0
else
	DEBUG_FLAGS := -O2 -DNDEBUG
endif

CFLAGS := -D_GNU_SOURCE --std=c99 -pedantic -Wall -I$(CURDIR)/src/ $(DEBUG_FLAGS)

####################################################

all : $(TARGET)

OBJS := $(foreach obj, $(OBJS), obj/$(obj))

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:.o=.d)
endif

obj/%.d : obj/%.o
obj/%.o : src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) -o $@ -MMD -c $(CFLAGS) $<

.PHONY : clean

clean :
	rm -f obj/*.o obj/*.d
	rm -f $(TARGET)
