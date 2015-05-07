      program main

      include 'jiaf.h'

      real*16  PI25DT
      parameter (PI25DT = 3.141592653589793238462643d0)
     
      real*16  pi, h, sum, x, f, a
      integer i
      integer begin, end
      real startt, endt
      pointer (pp, pi)
      
      f(a) = 4.d0 / (1.d0 + a*a)
      n = 10000
      
      call jiaf_init()
      pp = jia_alloc(%val(16))
      call jia_barrier()
      print *, 'Process ', jiapid, ' of ', jiahosts, ' is alive'

 10   if ( jiapid .eq. 0 ) then
	 pi = 0.0d0
      endif
      
      call jia_barrier()
      startt= jia_clock()
      h = 1.0d0/n

      sum  = 0.0d0
      begin = n/jiahosts*jiapid+1
      end = n/jiahosts*(jiapid+1)
      print *, 'begin = ', begin, 'end= ',end
      do 20 i = begin, end
         x = h * (dble(i) - 0.5d0)
         sum = sum + f(x)
 20   continue
      sum = h * sum

      print *, ' pass local calculation'
      call jia_lock(%val(1))
         pi = pi + sum
      call jia_unlock(%val(1))
      call jia_barrier()
      endt = jia_clock()


c                                 node 0 prints the answer.
      if (jiapid .eq. 0) then
         write(6, 97) pi, abs(pi-PI25DT)
 97      format(' pi is ', F27.25, ' +- ', F27.25)
         print*, 'Elapsed time is ', endt-startt, 'seconds'
      endif
	
      call jia_exit()

30    stop
      end




