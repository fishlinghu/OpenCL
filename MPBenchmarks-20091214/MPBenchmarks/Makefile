# OpenCL multiprecision CPU+GPU benchmarks
# EB Nov 2009

# archive data
CURRENT_DATE := $(shell date +%Y%m%d_%Hh%M)
CURRENT_DIR := $(notdir $(shell /bin/pwd))
CURRENT_MACHINE := $(shell uname -m)

# output directories
OBJDIR := objs
DEPDIR := deps
ifeq ($(CURRENT_MACHINE),x86_64)
LIBDIR := lib-64
else
LIBDIR := lib-32
endif

# targets
OBJS = $(patsubst %,$(OBJDIR)/%.o, GPUTask MemoryTasks ArithmeticTasks appMain )
DEPS = $(patsubst $(OBJDIR)/%.o, $(DEPDIR)/%.d, $(OBJS))

# OpenCL SDK directory
# OPENCL_DIR = /opt/cuda/include
OPENCL_DIR = /opt/ati-stream-sdk-v2.0-beta4-lnx64
# Bealto OpenCL library path
BEALTO_OPENCL_DIR = ../BealtoOpenCL
BEALTO_OPENCL_LIB = $(BEALTO_OPENCL_DIR)/$(LIBDIR)/libBealtoOpenCL.a

# flags
CXXFLAGS = -DLinux -DCONFIG_USE_QT=0 -O2 -mtune=nocona -msse3 -Wall -I/usr/include/qt4 -I$(OPENCL_DIR)/include -I$(BEALTO_OPENCL_DIR)/include
LDFLAGS = -L/usr/lib/qt4 -L$(OPENCL_DIR)/lib/x86_64

all: tocl

tocl: $(OBJS) $(BEALTO_OPENCL_LIB)
	g++ $(LDFLAGS) -o $@ $^ $(BEALTO_OPENCL_LIB) -lpthread -lOpenCL # -lQtCore -lQtGui 

$(BEALTO_OPENCL_LIB):
	make -C $(BEALTO_OPENCL_DIR) lib

t1: t1.o
	g++ -o $@ $^

dos2unix:
	dos2unix *.cpp *.h *.cl Makefile

clean:
	/bin/rm -f *.o *~ tocl t1
	/bin/rm -rf vs2008/Release-* vs2008/Debug-* $(OBJDIR) $(DEPDIR)

archive: clean
	@echo "ARCHIVE $(CURRENT_DATE)"
	tar czf "../MPBenchmarks-$(CURRENT_DATE).tar.gz" -C.. --exclude=".svn" $(CURRENT_DIR)

##### Dependencies
$(DEPDIR)/%.d: %.cpp
	@[ -d $(DEPDIR) ] || mkdir -p $(DEPDIR)
	@/bin/echo -e "DEPS \033[32m$*\033[0m"
	@$(CXX) $(CXXFLAGS) -o $@ -MM -MT '$(OBJDIR)/$*.o $@' $<

##### Compilation
$(OBJDIR)/%.o: %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@/bin/echo -e "C++  \033[34m$*\033[0m"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

##### Qt
moc_%.cpp: %.h
	@/bin/echo -e "MOC  \033[34m$*\033[0m"
	@moc -o $@ $<

-include $(DEPS)
