all: bus comptroller station-manager mystation

mystation: mystation.c
	gcc -o mystation mystation.c -pthread

station-manager: station_manager.c
	gcc -o station-manager station_manager.c -pthread

comptroller: comptroller.c
	gcc -o comptroller comptroller.c -pthread

bus: bus.c
	gcc -o bus bus.c -pthread

clean:
	rm -f mystation station-manager comptroller bus
