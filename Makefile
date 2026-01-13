SRC_DIR := src
INCLUDE_DIR := include
SFML_INCLUDE := C:/SFML/include
SFML_LIB := C:/SFML/lib
BUILD_DIR := build
EXE_NAME := main

CXX := g++
CC  := gcc

CXXFLAGS := -I$(SFML_INCLUDE) -I$(INCLUDE_DIR) -std=c++20
CFLAGS   := -I$(INCLUDE_DIR)

CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
C_FILES   := $(wildcard $(SRC_DIR)/*.c)

CPP_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_FILES))
C_OBJ_FILES   := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_FILES))

OBJ_FILES := $(CPP_OBJ_FILES) $(C_OBJ_FILES)

all: precompile compile link_dynamic clean run

precompile: $(BUILD_DIR) $(BUILD_DIR)/resource.o

$(BUILD_DIR)/resource.o:
	@echo IDI_ICON1 ICON "C:/SFML/examples/assets/logo.ico" > resource.rc
	@windres resource.rc -o $@
	@del /Q resource.rc

compile: $(OBJ_FILES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

link_dynamic:
	$(CXX) -L$(SFML_LIB) $(OBJ_FILES) -o "$(EXE_NAME).exe" $(BUILD_DIR)/resource.o -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

build_static: CXXFLAGS += -O2
build_static: precompile compile link_static clean run

static:
	@make build_static -B

link_static:
	$(CXX) -static -L$(SFML_LIB) $(OBJ_FILES) -o "$(EXE_NAME).exe" $(BUILD_DIR)/resource.o -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lfreetype -lvorbis -lvorbisfile -lflac -logg -mwindows -lopengl32 -lgdi32 -lwinmm -static-libstdc++ -static-libgcc

clean:
	@strip "$(EXE_NAME).exe"

run:
	@.\"$(EXE_NAME)"
	cls