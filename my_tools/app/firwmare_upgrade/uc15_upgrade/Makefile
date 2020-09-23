all:release build

release:
	test -d release||mkdir -p release

build:
	make -C src

clean:
	rm release -rf
	make clean -C src
