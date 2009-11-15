# BUILD SETTINGS ###################################
DEBUG := 1
PLATFORM := UNIX
USE_PRECOMPILED := 0

# END SETTINGS #####################################

TARGET := lem3edit
OBJS := cmp.o del.o editor.o lem3edit.o level.o raw.o style.o tribe.o

ifeq ($(DEBUG), 1)
	DEBUG_FLAGS := -g3 -O0
else
	DEBUG_FLAGS := -O3 -DNDEBUG
endif

ifeq ($(USE_PRECOMPILED), 1)
        PRECOMPILED_H_GCH := src/precompiled.hpp.gch
        INCLUDE_PRECOMPILED := -include "precompiled.hpp"
else
        PRECOMPILED_H_GCH :=
        INCLUDE_PRECOMPILED :=
endif

SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

CXXFLAGS := --std=c++98 -pedantic -Wall -Wextra -Wno-unused -Werror -I$(CURDIR)/src/ $(DEBUG_FLAGS) $(SDL_CFLAGS)
LDFLAGS := $(SDL_LDFLAGS)

#CXXFLAGS += -pg
#LDFLAGS += -pg

####################################################

all : $(TARGET)

OBJS := $(foreach obj, $(OBJS), obj/$(obj))

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:.o=.d)
endif

$(PRECOMPILED_H_GCH) : src/precompiled.hpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

obj/%.d : obj/%.o
obj/%.o : src/%.cpp $(PRECOMPILED_H_GCH)
	@mkdir -p "$(dir $@)"
	$(CXX) -o $@ -MMD -c $(CXXFLAGS) $(INCLUDE_PRECOMPILED) $< 

.PHONY : clean

clean :
	rm -rf obj/* $(PRECOMPILED_H_GCH)
	rm -f $(TARGET)
