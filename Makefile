#
#
TEST_EXE = cppes_test
LIB_NAME = cppes
BINDIR = bin

RM :=rm -f 

LIB_SRCS = $(wildcard src/*.cpp src/json/*.cpp src/*.cpp)
LIB_OBJS = $(patsubst %.cpp,%.o,$(LIB_SRCS) )
EXE_SRCS = $(wildcard test/*.cpp)
EXE_OBJS = $(patsubst %.cpp,%.o,$(EXE_SRCS) )

CXXFLAGS = -g -finline-functions -Wno-inline -Wall  -D_GLIBCXX_USE_CXX11_ABI=0 -rdynamic -ldl -lrt
CPPFLAGS = -I./src -I./deps -I./include
LIBS =-L./deps/lib -llut -L./lib -lcurl -lidn -lssl -lcrypto

all: libs test

dir:
	if [ ! -d $(BINDIR) ]; then mkdir $(BINDIR) ; fi;

libs:  dir $(LIB_NAME) 
	
$(LIB_NAME): $(LIB_OBJS)
	ar -cr $(BINDIR)/lib$@.a  $^
	
test:   $(EXE_OBJS)
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $(BINDIR)/$(TEST_EXE)  $^ -L$(BINDIR)  -l$(LIB_NAME) $(LIBS)
	
clean:
	$(RM) $(LIB_OBJS) $(EXE_OBJS)
	$(RM) $(BINDIR)/$(TEST_EXE) $(BINDIR)/lib$(LIB_NAME).a
#
#

