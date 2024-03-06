CONFIG ?= config.default
-include $(CONFIG)


BINARY    ?= wolf4sdl
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)

INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 555 -s
INSTALL_MAN     ?= $(INSTALL) -m 444
INSTALL_DATA    ?= $(INSTALL) -m 444

ifeq ($(SDL_MAJOR_VERSION),1)
	SDL_CONFIG  ?= sdl-config
else
	SDL_CONFIG  ?= sdl2-config
endif
CFLAGS_SDL  ?= $(shell $(SDL_CONFIG) --cflags)
LDFLAGS_SDL ?= $(shell $(SDL_CONFIG) --libs)


CFLAGS += $(CFLAGS_SDL)

#CFLAGS += -Wall
#CFLAGS += -W
CFLAGS += -g
CFLAGS += -Wpointer-arith
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings
CFLAGS += -Wcast-align

ifdef GPL
    CFLAGS += -DUSE_GPL
endif


CCFLAGS += $(CFLAGS)
CCFLAGS += -std=gnu99
CCFLAGS += -Werror-implicit-function-declaration
CCFLAGS += -Wimplicit-int
CCFLAGS += -Wsequence-point

CXXFLAGS += $(CFLAGS)

LDFLAGS += $(LDFLAGS_SDL)
ifeq ($(SDL_MAJOR_VERSION),1)
	LDFLAGS += -lSDL_mixer
else
	LDFLAGS += -lSDL2_mixer
endif
ifneq (,$(findstring MINGW,$(shell uname -s)))
LDFLAGS += -static-libgcc
endif

SRCS :=
ifndef GPL
    SRCS += mame/fmopl.c
else
    SRCS += dosbox/dbopl.cpp
endif
SRCS += id_ca.c
SRCS += id_in.c
SRCS += id_pm.c
SRCS += id_sd.c
SRCS += id_us.c
SRCS += id_vh.c
SRCS += id_vl.c
SRCS += signon.c
SRCS += wl_act1.c
SRCS += wl_act2.c
SRCS += wl_agent.c
SRCS += wl_atmos.c
SRCS += wl_cloudsky.c
SRCS += wl_debug.c
SRCS += wl_draw.c
SRCS += wl_game.c
SRCS += wl_inter.c
SRCS += wl_main.c
SRCS += wl_menu.c
SRCS += wl_parallax.c
SRCS += wl_plane.c
SRCS += wl_play.c
SRCS += wl_scale.c
SRCS += wl_shade.c
SRCS += wl_state.c
SRCS += wl_text.c
SRCS += wl_utils.c

DEPS = $(filter %.d, $(SRCS:.c=.d) $(SRCS:.cpp=.d))
OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o))

.SUFFIXES:
.SUFFIXES: .c .cpp .d .o

Q ?= @

all: $(BINARY)

ifndef NO_DEPS
depend: $(DEPS)

ifeq ($(findstring $(MAKECMDGOALS), clean depend Data),)
-include $(DEPS)
endif
endif

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC) $(CCFLAGS) -c $< -o $@

.cpp.o:
	@echo '===> CXX $<'
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

.c.d:
	@echo '===> DEP $<'
	$(Q)$(CC) $(CCFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@

.cpp.d:
	@echo '===> DEP $<'
	$(Q)$(CXX) $(CXXFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(DEPS) $(OBJS) $(BINARY) $(BINARY).exe

install: $(BINARY)
	@echo '===> INSTALL'
	$(Q)$(INSTALL) -d $(PREFIX)/bin
	$(Q)$(INSTALL_PROGRAM) $(BINARY) $(PREFIX)/bin
