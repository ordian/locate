CXX      := g++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++11 ##-fsanitize=address -g
OFLAGS   := -Ofast
LFLAGS   := -lboost_system -lboost_filesystem -lboost_program_options -lboost_timer -lboost_thread -ltbb
SRC_DIR  := src
OBJ_DIR  := obj
SRC      := $(wildcard $(SRC_DIR)/*.cc)
INCLUDE  := $(wildcard include/*.h) 
OBJ      := $(SRC:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)
TARGET   := locate updatedb

all: $(TARGET) $(INCLUDE)

$(TARGET): $(OBJ)
	@echo "Linking"
	$(CXX) $(OBJ_DIR)/$@.o -o $@ $(LFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling"
	$(CXX) $(CXXFLAGS) $(OFLAGS) -c $? -o $@

clean:
	@echo "Cleaning..."
	rm -fr obj $(TARGET) 

.PHONY: clean
