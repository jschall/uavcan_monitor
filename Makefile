include libuavcan/libuavcan/include.mk

SRC=main.cpp $(LIBUAVCAN_SRC)
OBJ=$(patsubst %,build/%.o,$(basename $(SRC)))

build/uavcan_node_manager: $(OBJ)
	mkdir -p "$(dir $@)"
	g++ -std=c++11 -o $@ $^

build/%.o: %.cpp build/dsdlc_generated
	mkdir -p "$(dir $@)"
	g++ -std=c++11 -I$(LIBUAVCAN_INC) -Ilibuavcan/libuavcan_drivers/linux/include -Ibuild/dsdlc_generated -c $< -o $@ -luavcan

.PHONY:build/dsdlc_generated
build/dsdlc_generated:
	$(LIBUAVCAN_DSDLC) libuavcan/dsdl/uavcan -O $@

.PHONY: clean
clean:
	rm -rf build
