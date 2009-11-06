# BUILD SETTINGS ###########################################

TARGET := lemcrunch

############################################################

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:src/%.cpp=obj/%.o)

# FLAGS ####################################################

ifneq ($(MAKECMDGOALS), release)
    EXTRA_CXXFLAGS += -g3 -O0 -Werror
else
    EXTRA_CXXFLAGS += -O2 -DNDEBUG
endif
EXTRA_CXXFLAGS += -MMD -pedantic -Wall -Wextra

ALL_CXXFLAGS += --std=c++98 -I./src $(EXTRA_CXXFLAGS) $(CXXFLAGS)
ALL_LDFLAGS += $(LDFLAGS)

# RULES ####################################################

.PHONY : all release clean

all : $(TARGET)

release : all

clean :
	rm -rf obj/* src/precompiled.hpp.d src/precompiled.hpp.gch
	rm -f $(TARGET)

ifneq ($(MAKECMDGOALS), clean)
    -include $(OBJS:.o=.d)
endif

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $(ALL_LDFLAGS) $^ $(LDLIBS)

src/precompiled.hpp.gch : src/precompiled.hpp
	-$(CXX) -c -o $@ $(ALL_CXXFLAGS) $<

obj/%.o : src/%.cpp src/precompiled.hpp.gch
	@mkdir -p "$(dir $@)"
	$(CXX) -c -o $@ $(ALL_CXXFLAGS) -include "precompiled.hpp" $< 
