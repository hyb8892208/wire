all:
	mkdir -p release
	make -C src
	cp drivers/*.ko release/
	
clean:
	make clean -C src
	rm -rf release
