###########################################################################
# user-configurable section
###########################################################################

DEBUG=1

# common locations for the OpenEXR libraries; may need to be updated
# for unusual installation locations
HAVE_EXR=1
EXR_INCLUDES=-I/usr/local/include/OpenEXR -I/usr/include/OpenEXR -I/opt/local/include/OpenEXR 
EXR_LIBDIR=-L/usr/local/lib -L/opt/local/lib

HAVE_LIBTIFF=0
TIFF_INCLUDES=-I/usr/local/include -I/opt/local/include
TIFF_LIBDIR=-L/usr/local/lib -L/opt/local/lib

HAVE_DTRACE=0

# remove -DPBRT_HAS_OPENEXR to build without OpenEXR support
DEFS=-DPBRT_HAS_OPENEXR

# 32 bit
#MARCH=-m32 -msse2 -mfpmath=sse

# 64 bit
MARCH=-m64

# change this to -g3 for debug builds
#OPT=-O2
OPT=-g3
# comment out this line to enable assertions at runtime
DEFS += -DNDEBUG

#########################################################################
# nothing below this line should need to be changed (usually)
#########################################################################

ARCH = $(shell uname)

LEX=flex
YACC=bison -d -v -t
ifeq ($(ARCH),OpenBSD)
    LEXLIB = -ll
else
    LEBLIB = -lfl
endif

ifeq ($(HAVE_DTRACE),1)
    DEFS += -DPBRT_PROBES_DTRACE
else
    DEFS += -DPBRT_PROBES_NONE
endif

EXRLIBS=$(EXR_LIBDIR) -Bstatic -lIex -lIlmImf -lIlmThread -lImath -lIex -lHalf -Bdynamic
ifeq ($(ARCH),Linux)
  EXRLIBS += -lpthread
endif
ifeq ($(ARCH),OpenBSD)
  EXRLIBS += -lpthread
endif
ifeq ($(ARCH),Darwin)
  EXRLIBS += -lz
endif

CC=gcc
CXX=g++
LD=$(CXX) $(OPT) $(MARCH)
SRC=src
INCLUDE=-I$(SRC) -I$(SRC)/core -I$(SRC)/core/opencl $(EXR_INCLUDES) $(TIFF_INCLUDES)
WARN=-Wall
CWD=$(shell pwd)
DEPS= -MM -MF $@.d
CXXFLAGS=$(OPT) $(MARCH) $(INCLUDE) $(WARN) $(DEFS)
CCFLAGS=$(CXXFLAGS)
LIBS=$(LEXLIB) $(EXR_LIBDIR) $(EXRLIBS) -lm -lOpenCL -llog4cxx

ifeq ($(DEBUG), 1)
	CXXFLAGS += -DDEBUG -g -ggdb
endif

LIB_CSRCS=$(SRC)/core/targa.c
LIB_CXXSRCS  = $(wildcard $(SRC)/core/*.cpp) $(SRC)/core/pbrtlex.cpp $(SRC)/core/pbrtparse.cpp
LIB_CXXSRCS += $(wildcard $(SRC)/accelerators/*.cpp $(SRC)/cameras/*.cpp $(SRC)/film/*.cpp $(SRC)/filters/*.cpp )
LIB_CXXSRCS += $(wildcard $(SRC)/integrators/*.cpp $(SRC)/lights/*.cpp $(SRC)/materials/*.cpp $(SRC)/renderers/*.cpp )
LIB_CXXSRCS += $(wildcard $(SRC)/samplers/*.cpp $(SRC)/shapes/*.cpp $(SRC)/textures/*.cpp $(SRC)/volumes/*.cpp)
LIB_CXXSRCS += $(wildcard $(SRC)/core/opencl/*.cpp)

LIBOBJS  = $(addprefix objs/, $(subst /,_,$(subst $(SRC)/,,$(LIB_CSRCS:.c=.o))))
LIBOBJS += $(addprefix objs/, $(subst /,_,$(subst $(SRC)/,,$(LIB_CXXSRCS:.cpp=.o))))

HEADERS = $(wildcard $(SRC)/*/*.h)
HEADERS += $(wildcard $(SRC)/core/opencl/*.h)

TOOLS = bin/bsdftest bin/exravg bin/exrdiff bin/oclcheck
ifeq ($(HAVE_LIBTIFF),1)
    TOOLS += bin/exrtotiff
endif

all: default

default: dirs bin/pbrt $(TOOLS)

bin/%: dirs

pbrt: bin/pbrt

dirs:
	/bin/mkdir -p bin objs

#$(LIBOBJS): $(HEADERS)

.PHONY: dirs tools 

objs/libpbrt.a: $(LIBOBJS)
	@echo "Building the core rendering library (libpbrt.a)"
	@ar rcs $@ $(LIBOBJS)

objs/accelerators_%.o: $(SRC)/accelerators/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/cameras_%.o: $(SRC)/cameras/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/core_%.o: $(SRC)/core/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/core_%.o: $(SRC)/core/%.c
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CC) $(CCFLAGS) -o $@ -c $<

objs/core_opencl_%.o: $(SRC)/core/opencl/%.cpp $(SRC)/core/opencl/host.h
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -D__CL_ENABLE_EXCEPTIONS -o $@ -c $<

objs/film_%.o: $(SRC)/film/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/filters_%.o: $(SRC)/filters/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/integrators_%.o: $(SRC)/integrators/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/lights_%.o: $(SRC)/lights/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/main_%.o: $(SRC)/main/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/materials_%.o: $(SRC)/materials/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/renderers_%.o: $(SRC)/renderers/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/samplers_%.o: $(SRC)/samplers/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/shapes_%.o: $(SRC)/shapes/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/textures_%.o: $(SRC)/textures/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/volumes_%.o: $(SRC)/volumes/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/pbrt.o: $(SRC)/main/pbrt.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

objs/tools_%.o: $(SRC)/tools/%.cpp
	@echo "Building object $@"
	@$(CXX) $(CCFLAGS) -c $< -MM -MF $@.d
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

bin/pbrt: objs/main_pbrt.o objs/libpbrt.a
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

bin/%: objs/tools_%.o objs/libpbrt.a 
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

bin/exrtotiff: objs/tools_exrtotiff.o 
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(TIFF_LIBDIR) -ltiff $(LIBS) 

$(SRC)/core/pbrtlex.cpp: $(SRC)/core/pbrtlex.ll $(SRC)/core/pbrtparse.cpp
	@echo "Lex'ing pbrtlex.ll"
	@$(LEX) -o$@ $(SRC)/core/pbrtlex.ll

$(SRC)/core/pbrtparse.cpp: $(SRC)/core/pbrtparse.yy
	@echo "YACC'ing pbrtparse.yy"
	@$(YACC) -o $@ $(SRC)/core/pbrtparse.yy
	@if [ -e $(SRC)/core/pbrtparse.cpp.h ]; then /bin/mv $(SRC)/core/pbrtparse.cpp.h $(SRC)/core/pbrtparse.hh; fi
	@if [ -e $(SRC)/core/pbrtparse.hpp ]; then /bin/mv $(SRC)/core/pbrtparse.hpp $(SRC)/core/pbrtparse.hh; fi

ifeq ($(HAVE_DTRACE),1)
$(SRC)/core/dtrace.h: $(SRC)/core/dtrace.d
	/usr/sbin/dtrace -h -s $^ -o $@

$(LIBOBJS): $(SRC)/core/dtrace.h
endif

$(RENDERER_BINARY): $(RENDERER_OBJS) $(CORE_LIB)

clean:
	rm -f objs/* bin/* $(SRC)/core/pbrtlex.[ch]* $(SRC)/core/pbrtparse.[ch]*

-include $(LIBOBJS:.o=.o.d)