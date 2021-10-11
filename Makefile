#L++ programming language was under the MIT license.
#Copyright(c) 2021 nu11ptr team.
OUTPUT = lpp
CXX = g++
CXXFLAGS = -Os -s -Wall -D ENABLE_EXT
all : build
build :
	@echo "[CXX] lpp.cpp -> $(OUTPUT)"
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) lpp.cpp -o $(OUTPUT)
	@echo "-- DONE --"
debug :
	@echo "[CXX] lpp.cpp -> $(OUTPUT)"
	@$(CXX) -g -Wall -D ENABLE_EXT $(LDFLAGS) lpp.cpp -o $(OUTPUT)
	@echo "-- DONE --"
clean :
	@echo "[RM] $(OUTPUT)"
	@rm $(OUTPUT)
	@echo "-- DONE --"
