BUILD=build
BIN=bin
CC=g++ -std=c++11 -g -pthread
LLIBS=-lntl -lgmp -lm -lcrypto

$(BUILD)/s_vss.o: src/vss.cpp src/vss.hpp
	$(CC) -o $@ -c $<

$(BUILD)/t_vss.o: test/vss.cpp src/vss.hpp
	$(CC) -o $@ -c $<

$(BUILD)/s_communicate.o: src/communicate.cpp src/communicate.hpp src/constants.hpp
	$(CC) -o $@ -c $<

$(BUILD)/init.o: init.cpp src/constants.hpp src/communicate.hpp src/vss.hpp
	$(CC) -o $@ -c $<

$(BUILD)/s_io.o: src/io.cpp src/io.hpp
	$(CC) -o $@ -c $<

$(BUILD)/dealer.o: dealer.cpp src/constants.hpp src/vss.hpp src/communicate.hpp src/io.hpp
	$(CC) -o $@ -c $<

$(BUILD)/party.o: party.cpp src/constants.hpp src/vss.hpp src/communicate.hpp src/io.hpp
	$(CC) -o $@ -c $<

$(BIN)/test: $(BUILD)/t_vss.o $(BUILD)/s_vss.o
	$(CC) -o $@ $^ $(LLIBS)

$(BIN)/init: $(BUILD)/init.o $(BUILD)/s_vss.o $(BUILD)/s_communicate.o
	$(CC) -o $@ $^ $(LLIBS)

$(BIN)/dealer: $(BUILD)/dealer.o $(BUILD)/s_vss.o $(BUILD)/s_communicate.o $(BUILD)/s_io.o
	$(CC) -o $@ $^ $(LLIBS)

$(BIN)/party: $(BUILD)/party.o $(BUILD)/s_vss.o $(BUILD)/s_communicate.o $(BUILD)/s_io.o
	$(CC) -o $@ $^ $(LLIBS)

.PHONY: test clean init dealer party clean_data proto

test: $(BIN)/test

clean:
	rm -r $(BUILD)
	rm -r $(BIN)

clean_data:
	rm gen_out.dat

init: $(BIN)/init

dealer: $(BIN)/dealer

party: $(BIN)/party

proto: dealer party init

$(shell mkdir -p $(BUILD))
$(shell mkdir -p $(BIN))
