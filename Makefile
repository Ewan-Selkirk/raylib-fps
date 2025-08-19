#CXX = g++
#CXX = clang++

EXE = first_person
SRC_DIR = src
IMGUI_DIR = lib/imgui
BUILD_DIR = build
SOURCES = $(SRC_DIR)/game.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
#SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/rlImGui.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat
LIBS = 


ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`
	LIBS += `pkg-config --static --libs raylib`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
# 	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32
	LIBS += -lraylib -lgdi32 -lwinmm

	CXXFLAGS += -I include/RayLib -I include/GLFW
# 	CXXFLAGS += `pkg-config --cflags glfw3`
	CXXFLAGS += -L lib
	CFLAGS = $(CXXFLAGS)
endif

%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $(BUILD_DIR)/$@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $(BUILD_DIR)/$@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $(BUILD_DIR)/$@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $(foreach wrd, $^, $(BUILD_DIR)/$(wrd)) $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(BUILD_DIR)/$(OBJS)
