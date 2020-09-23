all:
	g++ genlicense.cpp -lcryptopp   -lpthread -o genlicense
	g++ genmasterpair.cpp -lcryptopp  -lpthread -o genmasterpair
	g++ gensecondarypair.cpp -lcryptopp  -lpthread -o gensecondarypair
	g++ verifylicense.cpp -lcryptopp   -lpthread -o verifylicense
	g++ genxmllicense.cpp -lcryptopp  -lpthread  -o genxmllicense
	g++ -I/usr/include/libxml2 verifyxmllicense.cpp -lcryptopp -lxml2  -lpthread  -o verifyxmllicense
clean:
	rm -rf genlicense genmasterpair gensecondarypair verifylicense genxmllicense verifyxmllicen *.o *.d 
