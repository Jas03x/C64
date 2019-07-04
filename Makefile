PATH = /mnt/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.21.27702/bin/Hostx64/x64

CC   = "$(PATH)/cl.exe"
LINK = "$(PATH)/link.exe"

INC_PATH  = /I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/ucrt"
INC_PATH += /I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.21.27702/include"
INC_PATH += /I ./src/parser/

LIB_PATH  = /LIBPATH:"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.21.27702/lib/x64"
LIB_PATH += /LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x64"
LIB_PATH += /LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/ucrt/x64"

CC_OPTIONS = /D_HAS_EXCEPTIONS=0

BIN = ./bin
SRC = ./src

EXE = C64.exe

ROOT_OBJECTS   = $(patsubst $(SRC)/%.cpp,        $(BIN)/%.o,        $(wildcard $(SRC)/*.cpp))
UTIL_OBJECTS   = $(patsubst $(SRC)/util/%.cpp,   $(BIN)/util/%.o,   $(wildcard $(SRC)/util/*.cpp))
PARSER_OBJECTS = $(patsubst $(SRC)/parser/%.cpp, $(BIN)/parser/%.o, $(wildcard $(SRC)/parser/*.cpp))

COMPILE_OBJ = $(CC) $(CC_OPTIONS) $(INC_PATH) /Fo:$@ /c

$(BIN)/%.o: $(SRC)/%.cpp
	$(COMPILE_OBJ) $^

$(BIN)/util/%.o: $(SRC)/util/%.cpp
	$(COMPILE_OBJ) $^

$(BIN)/parser/%.o: $(SRC)/parser/%.cpp
	$(COMPILE_OBJ) $^

$(EXE): $(ROOT_OBJECTS) $(UTIL_OBJECTS) $(PARSER_OBJECTS)
	$(LINK) $(LIB_PATH) $^ /OUT:$(BIN)/$@

.PHONY: clean
clean:
	/bin/rm -f $(BIN)/*.o
	/bin/rm -f $(BIN)/parser/*.o
	/bin/rm -f $(BIN)/$(EXE)
