
SVN_ROOT=$(shell svn info | grep "Working" | cut -d" " -f 5)

include $(SVN_ROOT)/global/global.mk

USER_CXX_FLAGS=-I../




sources=src/timestamp.cpp src/datestamp.cpp
objects=$(sources:.cpp=.o)
depends=$(sources:.cpp=.d)

USER_LD_FLAGS=-l rt


test_exes=test/test1 test/test_timestamp test/test2 test/test_rdtsc test/test_clock_gettime \
    test/test_gettimeofday test/test_gettimeofday1 test/test7 \
    test/test_get_day_of_year test/test_show_local_gmt_time

-include $(depends)



all:	$(objects)



clean:	test_clean
	$(RM) $(objects)
	$(RM) $(depends)
	$(RM) src/*.d.[0-9]*
	$(RM) src/*.o


test_clean:
	$(RM) $(test_exes) test/*.o


	    

test_programs:	$(test_exes)


test/test1:	                    test/test1.cpp
test/test2:	                    test/test2.cpp $(objects) 
test/test_rdtsc:                test/test_rdtsc.cpp $(objects) 
test/test_timestamp:	        test/test_timestamp.cpp $(objects)
test/test_clock_gettime:        test/test_clock_gettime.cpp
test/test_gettimeofday:         test/test_gettimeofday.cpp
test/test_gettimeofday1:        test/test_gettimeofday1.cpp
test/test7:	                    test/test7.cpp $(objects) 
test/test_get_day_of_year:      test/test_get_day_of_year.cpp $(objects)
test/test_show_local_gmt_time:  test/test_show_local_gmt_time.cpp $(objects)


