all:
	$(CC) term_pong.c -lncurses -o term_pong

clean:
	rm -rf term_pong
