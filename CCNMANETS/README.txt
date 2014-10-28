Be sure to clean the project by "make clean".

Unit test includes:

make testcontentdirectory // test the ranking mechanism

In class DataLayer.cpp, a posix-compatible platform-dependent API is used. To port the project to other platform, e.g., windows, extra efforts are needed.

To test the whole system, a simple command sequence can be like this:

All timestamp should be UTC time, to save trouble, e.g., dst

(1) create network adhoc
(2) create device a
(3) setgps a 50 -30
(4) create device b
(5) setgps b 50 -30.0017
(6) create device c
(7) setgps c 50 -30.0013
(8) create device d
(9) connect a
(10) connect b
(11) connect c
(12) neighbor a
(13) neighbor b
(14) neighbor c
(15) directory a
(16) publish a ./media/beautiful.mp3 test.mp3
(17) directory a
(18) directory b
(19) directory c
(20) retrieve c test.mp3 ./meida/copy.mp3



emulation in batch mode: 5km x 5km spaces.

(33 40 12 N ,112 11 07 W)
latitude: 33.67011111111111
longitude: -112.18530555555556

latitude: 33.67011111111111
longitude: -112.23944444444444

latitude: 33.625
longitude: -112.23944444444444

latitude: 33.625
longitude: -112.18530555555556
