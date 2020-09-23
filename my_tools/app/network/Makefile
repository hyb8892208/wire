targent=release script_s chat_s curl quectel-CM_s

all:$(targent)


CURL_DIR=curl-curl-7_38_0

release:
	test -d release||mkdir -p release

script_s:
	cp script/* ./release/

chat_s:
	make -C chat
	cp chat/chat ./release/

curl:
	test -d $(CURL_DIR) || unzip -o $(CURL_DIR).zip > /dev/null
	cd $(CURL_DIR);cmake .;cd -
	make -C $(CURL_DIR)
	cp $(CURL_DIR)/src/curl release/

quectel-CM_s:
	make -C quectel-CM

clean:
	rm -rf $(CURL_DIR)
	rm release -rf
	make clean -C chat
	make clean -C quectel-CM

