# Using nmake

# Compiler & linker
CXX = cl
LD = link

# Paths
INCLUDEDIR = "C:/Program Files (x86)/Teknic/ClearView/sdk/inc"
LIBDIR = "C:/Program Files (x86)/Teknic/ClearView/sdk/lib/win/release/x64"
BINDIR = bin

# Source and target
SRCS = test.cpp
OBJS = $(BINDIR)/test.obj
TARGET = $(BINDIR)/test.exe
PDB = $(BINDIR)/test.pdb

# Build rules
all: $(TARGET)

# Linking step
$(TARGET): $(OBJS)
	$(LD) /OUT:$(TARGET) /PDB:$(PDB) /LIBPATH:$(LIBDIR) sFoundation20.lib $(OBJS)

# Create bin directory if it doesn't exist and compile
$(BINDIR)\%.obj: %.cpp
	@if not exist $(BINDIR) mkdir $(BINDIR)
	@echo Compiling $<
	$(CXX) /nologo /EHsc /I"$(INCLUDEDIR)" /c $< /Fo$@


# Clean
clean:
	del /Q $(BINDIR)\*.obj $(BINDIR)\*.exe $(BINDIR)\*.pdb