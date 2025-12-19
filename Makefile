CC := clang
CFLAGS := -g -Wall -Wno-unused-function -Wno-unused-variable `pkg-config MagickWand --cflags` -fno-openmp
LDFLAGS:= -lm `pkg-config MagickWand --libs`
all: p2pchat magick-demo

clean:
	rm -f p2pchat

p2pchat: p2pchat.c ui.c ui.h message.c message.h chathelper.c chathelper.h encryption.c encryption.h bruteforce.c bruteforce.h image.c image.h
	$(CC) $(CFLAGS) -o p2pchat p2pchat.c ui.c message.c chathelper.c encryption.c bruteforce.c image.c -lform -lncurses -lpthread $(LDFLAGS)

magick-demo: magick-demo.c image.c image.h
	$(CC) $(CFLAGS) -o magick-demo magick-demo.c image.c $(LDFLAGS)

bruteforce: bruteforce.c bruteforce.h encryption.c encryption.h chathelper.c chathelper.h test_bruteforce.c
	$(CC) $(CFLAGS) -o bruteforce bruteforce.c encryption.c chathelper.c test_bruteforce.c -lpthread
	
chathelper: chathelper.c chathelper.h encryption.c encryption.h
	$(CC) $(CFLAGS) -o chathelper chathelper.c encryption.c bruteforce.c -lncurses
zip:
	@echo "Generating p2pchat.zip file to submit to Gradescope..."
	@zip -q -r p2pchat.zip . -x .git/\* .vscode/\* .clang-format .gitignore p2pchat
	@echo "Done. Please upload p2pchat.zip to Gradescope."
