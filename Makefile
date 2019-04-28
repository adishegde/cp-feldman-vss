BUILD=build
BIN=bin
CC=g++ -std=c++11 -g -pthread
LLIBS=-lntl -lgmp -lm -lcrypto

$(BUILD)/s_vss.o: src/vss.cpp src/vss.hpp
	$(CC) -o $@ -c $<

$(BUILD)/t_vss.o: test/vss.cpp src/vss.hpp
	$(CC) -o $@ -c $<

$(BIN)/test: $(BUILD)/t_vss.o $(BUILD)/s_vss.o
	$(CC) -o $@ $^ $(LLIBS)

.PHONY: test clean

test: $(BIN)/test

clean:
	rm -r $(BUILD)
	rm -r $(BIN)

$(shell mkdir -p $(BUILD))
$(shell mkdir -p $(BIN))
