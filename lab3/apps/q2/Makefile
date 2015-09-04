default:
	cd makeprocs; make
	cd injection_n2; make; cd ../injection_h2o;make; cd ../reaction1;make;cd ../reaction2;make; cd ../reaction3;make

clean:
	cd makeprocs; make clean
	cd injection_n2; make clean;cd ../injection_h2o;make clean; cd ../reaction1; make clean; cd ../reaction2; make clean; cd ../reaction3; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 10 7; ee469_fixterminal
