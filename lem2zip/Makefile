# BUILD SETTINGS ###################################
DEBUG := 1
# END SETTINGS #####################################

TARGET := lem2zip
OBJS := lem2zip.o compress.o decompress.o

ifeq ($(DEBUG), 1)
	EXTRA_CFLAGS := -g3 -O0
else
	EXTRA_CFLAGS := -O2 -DNDEBUG
endif

ALL_CFLAGS := -D_GNU_SOURCE --std=c99 -pedantic -Wall -I$(CURDIR)/src/ $(EXTRA_CFLAGS) $(CFLAGS)

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
	$(CC) -o $@ -MMD -c $(ALL_CFLAGS) $<

.PHONY : clean

clean :
	rm -f obj/*.o obj/*.d
	rm -f $(TARGET)
