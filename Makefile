#L programming language was under the MIT license.
#Copyright(c) 2021 nu11ptr team.
OUTPUT = lpp
CXX = g++
CXXFLAGS = -Os -s -Wall -D ENABLE_EXT
all : build
build :
	$(CXX) $(CXXFLAGS) $(LDFLAGS) lpp.cpp -o $(OUTPUT)
clean :
	rm $(OUTPUT)
