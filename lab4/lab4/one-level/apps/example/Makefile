default:
	cd makeprocs; make
	cd hello_world_0; make;cd ../hello_world_1;make;cd ../hello_world_2;make;cd ../hello_world_3;make;cd ../hello_world_4;make;cd ../hello_world_5;make;

clean:
	cd makeprocs; make clean
	cd hello_world_0; make clean;cd ../hello_world_1;make clean;cd ../hello_world_2;make clean;cd ../hello_world_3;make_clean;cd ../hello_world_4;make clean;cd ../hello_world_5;make clean;

run:
	cd ../../bin; dlxsim  -x os.dlx.obj -a  -u makeprocs.dlx.obj 1;ee469_fixterminal;  

