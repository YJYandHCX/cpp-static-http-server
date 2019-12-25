cc = g++
prom = a.out
deps = epoll.h http_req.h threadpool.h Timer_list.h
obj = main.o Timer_list.o http_req.o epoll.o 
$(prom): $(obj)


$(prom): $(obj)
	$(cc) -o $(prom) $(obj) -std=c++11 -lpthread

%.o: %.cpp $(deps)
	$(cc) -c $< -o $@ -std=c++11 -lpthread
clean:
	rm -rf $(obj) $(prom)
