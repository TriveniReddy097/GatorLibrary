CXX = g++
CXXFLAGS = -std=c++11 -Wall

TARGET = gatorLibrary
SRCS = gatorLibrary.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) *_output_file.txt

default: generate_output_filename

generate_output_filename:
	input_file="$(INPUT_FILE)"; \
	output_file="$${input_file%.*}_output_file.txt"; \
	./$(TARGET) "$(INPUT_FILE)" > "$$output_file"; \
	echo "Output file '$$output_file' created"; \