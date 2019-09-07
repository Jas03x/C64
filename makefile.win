PATH = /mnt/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.22.27905/bin/Hostx64/x64

CC   = "$(PATH)/cl.exe"
LINK = "$(PATH)/link.exe"

DBG = "/mnt/c/Program Files (x86)/Windows Kits/10/Debuggers/x64/windbg.exe"

INC_PATH  = /I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/ucrt"
INC_PATH += /I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.22.27905/include"
INC_PATH += /I ./src/misc/
INC_PATH += /I ./src/parser/

LIB_PATH  = /LIBPATH:"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.22.27905/lib/x64"
LIB_PATH += /LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64"
LIB_PATH += /LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/ucrt/x64"

BIN = ./bin
SRC = ./src

PDB = C64.pdb
EXE = C64.exe

CC_OPTIONS   = /Zi /D_HAS_EXCEPTIONS=0 /Fd:$(BIN)/$(PDB)
DBG_OPTIONS  = -y "C:/Users/Jas/Documents/C64/bin/" -srcpath "C:/Users/Jas/Documents/C64/src;C:/Users/Jas/Documents/C64/src/misc;C:/Users/Jas/Documents/C64/src/parser"
LINK_OPTIONS = /DEBUG /PDB:$(BIN)/$(PDB)

ROOT_OBJECTS   = $(patsubst $(SRC)/%.cpp,        $(BIN)/%.o,        $(wildcard $(SRC)/*.cpp))
MISC_OBJECTS   = $(patsubst $(SRC)/misc/%.cpp,   $(BIN)/misc/%.o,   $(wildcard $(SRC)/misc/*.cpp))
PARSER_OBJECTS = $(patsubst $(SRC)/parser/%.cpp, $(BIN)/parser/%.o, $(wildcard $(SRC)/parser/*.cpp))

COMPILE_OBJ = $(CC) $(CC_OPTIONS) $(INC_PATH) /Fo:$@ /c

$(BIN)/%.o: $(SRC)/%.cpp
	$(COMPILE_OBJ) $^

$(BIN)/misc/%.o: $(SRC)/misc/%.cpp
	$(COMPILE_OBJ) $^

$(BIN)/parser/%.o: $(SRC)/parser/%.cpp
	$(COMPILE_OBJ) $^

$(EXE): $(ROOT_OBJECTS) $(MISC_OBJECTS) $(PARSER_OBJECTS)
	$(LINK) $(LINK_OPTIONS) $(LIB_PATH) $^ /OUT:$(BIN)/$@

debug:
	$(DBG) $(DBG_OPTIONS) $(BIN)/$(EXE) ./test/main.c

.PHONY: test
test:
	$(BIN)/$(EXE) ./test/main.c

.PHONY: clean
clean:
	/bin/rm -f $(BIN)/*.o
	/bin/rm -f $(BIN)/misc/*.o
	/bin/rm -f $(BIN)/parser/*.o
	/bin/rm -f $(BIN)/$(EXE)
	/bin/rm -f $(BIN)/$(PDB)
	/bin/rm -f $(BIN)/*.ilk
