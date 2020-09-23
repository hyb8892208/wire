all:m35 ec20 sim6320c uc15

m35:
	make -C m35_upgrade

ec20:
	make -C ec20_upgrade

sim6320c:
	make -C sim6320c_upgrade

uc15:
	make -C uc15_upgrade

clean:
	make clean -C m35_upgrade
	make clean -C ec20_upgrade
	make clean -C sim6320c_upgrade
	make clean -C uc15_upgrade
