CC := clang
CFLAGS := -g -Wall -Werror -Wno-unused-function -Wno-unused-variable

all: p2pchat

clean:
	rm -f p2pchat

p2pchat: p2pchat.c ui.c ui.h message.c message.h chathelper.c chathelper.h encryption.c encryption.h bruteforce.c bruteforce.h
	$(CC) $(CFLAGS) -o p2pchat p2pchat.c ui.c message.c chathelper.c encryption.c bruteforce.c -lform -lncurses -lpthread

chathelper: chathelper.c chathelper.h encryption.c encryption.h
	$(CC) $(CFLAGS) -o chathelper chathelper.c encryption.c -lncurses
zip:
	@echo "Generating p2pchat.zip file to submit to Gradescope..."
	@zip -q -r p2pchat.zip . -x .git/\* .vscode/\* .clang-format .gitignore p2pchat
	@echo "Done. Please upload p2pchat.zip to Gradescope."
