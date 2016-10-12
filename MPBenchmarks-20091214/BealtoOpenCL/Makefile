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
OBJS = $(patsubst %,$(OBJDIR)/%.o, CLContext CLError CLProgram )
DEPS = $(patsubst $(OBJDIR)/%.o, $(DEPDIR)/%.d, $(OBJS))

LIB = $(LIBDIR)/libBealtoOpenCL.a

VPATH = src

# OpenCL SDK directory
# OPENCL_DIR = /opt/cuda/include
OPENCL_DIR = /opt/ati-stream-sdk-v2.0-beta4-lnx64

# flags
CXXFLAGS = -DLinux -DCONFIG_USE_QT=0 -O2 -mtune=nocona -msse3 -Wall -I./include -I$(OPENCL_DIR)/include
LDFLAGS = -L$(OPENCL_DIR)/lib/x86_64

lib: $(LIB)

$(LIB): $(OBJS)
	@[ -d $(LIBDIR) ] || mkdir -p $(LIBDIR)
	ar rvs $(LIB) $(OBJS)

dos2unix:
	dos2unix Makefile src/*.cpp include/*.h

clean:
	/bin/rm -f *.o *~ $(LIB)
	/bin/rm -rf vs2008/Release-* vs2008/Debug-* $(OBJDIR) $(DEPDIR) lib-32 lib-64

archive: clean
	@echo "ARCHIVE $(CURRENT_DATE)"
	tar czf "../BealtoOpenCL-$(CURRENT_DATE).tar.gz" -C.. --exclude=".svn" $(CURRENT_DIR)

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
