# ==================================================================
#  Makefile - dung cho Linux / macOS (hoac Windows co mingw32-make)
#    make          -> bien dich ra file des_aes
#    make results  -> chay tat ca test vector, ghi tung file vao thu muc results/
#    make clean    -> xoa file chay
# ==================================================================
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -O2 -static -Iinclude
SRC      = src/main.cpp src/utils.cpp src/des.cpp src/aes.cpp
TARGET   = des_aes

$(TARGET): $(SRC) include/*.h
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

results: $(TARGET)
	mkdir -p results
	./$(TARGET) export

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: results clean
