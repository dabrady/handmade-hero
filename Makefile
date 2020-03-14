BUILD  := ./build
CODE   := ./code
TARGET := handmade
SRC    := $(CODE)/sdl_handmade.cpp

run: handmade
	[[ -d $(BUILD) ]] && $(BUILD)/$(TARGET)

handmade: clean $(BUILD)/$(TARGET)

clean:
	$(RM) -r $(BUILD)

$(BUILD)/$(TARGET): $(BUILD)
	@c++ -g -o $(BUILD)/$(TARGET) $(SRC) `sdl2-config --cflags --libs`

$(BUILD):
	mkdir -p $(BUILD)

PHONY: handmade clean run
.SILENT: $(BUILD) clean run
