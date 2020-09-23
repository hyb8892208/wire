targent=update_grppolicy_dl
objs=update_grppolicy_dl.o

all:$(targent)

update_grppolicy_dl:$(objs)
	${CC} -g -o $@ $(objs)
	cp $@ ../release

%.o:%.c
	${CC} -g -c $^ -o $@

clean:
	rm -rf $(targent) $(objs)

