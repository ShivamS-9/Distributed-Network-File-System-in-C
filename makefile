CFLAGS = -g

main: all
all: client storage_server naming_server

client:
	gcc $(CFLAGS) Client/client_main.c Client/access_file.c Client/copy_files.c Client/create_delete.c Client/list_paths.c Client/send_to_naming_server.c -o client.out

naming_server:
	gcc $(CFLAGS) NM/nm.c NM/Trie.c NM/lru.c NM/log.c -o nm.out

ns: naming_server
nm: naming_server

storage_server:
	gcc $(CFLAGS) SS/client_handler.c SS/helper.c SS/init.c SS/nm_handler.c -o ss.out

storage_server_1:
	gcc $(CFLAGS) SS_1/client_handler.c SS_1/helper.c SS_1/init.c SS_1/nm_handler.c -o SS_1/ss.out

storage_server_2:
	gcc $(CFLAGS) SS_2/client_handler.c SS_2/helper.c SS_2/init.c SS_2/nm_handler.c -o SS_2/ss.out

storage_server_3:
	gcc $(CFLAGS) SS_3/client_handler.c SS_3/helper.c SS_3/init.c SS_3/nm_handler.c -o SS_3/ss.out

ss: storage_server storage_server_1 storage_server_2 storage_server_3

clean:
	rm -f *.out Tries/*.out Client/*.out NM/*.out SS/*.out

clean_test_cases:
	rm -rf test_cases

clean_log:
	rm -rf log_file.txt

reset: clean clean_test_cases clean_log