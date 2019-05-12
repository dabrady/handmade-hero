BUILD  := ./build
CODE   := ./code
TARGET := handmade
SRC    := $(CODE)/sdl_handmade.cpp

PHONY: handmade clean run
.SILENT: $(BUILD) clean run

handmade: clean $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(BUILD)
	@c++ -g -o $(BUILD)/$(TARGET) $(SRC) `sdl2-config --cflags --libs`

$(BUILD):
	mkdir -p $(BUILD)

run: handmade
	[[ -d $(BUILD) ]] && $(BUILD)/$(TARGET)

clean:
	$(RM) -r $(BUILD)
