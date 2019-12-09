all: server client


%:%.c
	gcc $< -o  $@

.PHONY:clean
clean:
	rm -rf server client
