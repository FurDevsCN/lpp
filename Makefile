#L programming language was under the MIT license.
#Copyright(c) 2021 nu11ptr team.
OUTPUT = lpp
build :
	$(CXX) $(CXXFLAGS) $(LDFLAGS) lpp.cpp -o $(OUTPUT)
clean :
	rm $(OUTPUT)
