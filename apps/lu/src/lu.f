	program lu

	include 'jiaf.h'

	parameter  (n=256)
	integer i, j
        integer  MAXRAND
	common /shared/ pa
	pointer (pa, a)
	character*30 luerr
        common /res/ old(0:n-1,0:n-1)
  	real t1, t2, temp, a(0:n-1,0:n-1)

        call jiaf_init()

        print *, 'jiahosts = ', jiahosts
c	call luinit(%val(n),%val(jiahosts))
        pa = jia_alloc(%val(4*n*n), %val(n*4),0)

	call jia_barrier()
        MAXRAND = 32768
	if (jiapid .eq. 0) then
	  do j = 0, n-1
	    do i = 0, n-1
	      a(i,j) =1.0*(rand(0)/MAXRAND)
	      if (i.eq.j) then
                 a(i,j)=a(i,j)*10.0
              endif
              old(i,j) = a(i,j)
	    end do
	   end do
	endif
        print *, ' pass initialization'
	call jia_barrier()

	luerr = 'Matrix is sigular . . .'
	t1 = jia_clock()

	temp=lua()
        if (temp.eq.-1) then 
	   call jia_error(luerr)
        endif
         
	call jia_barrier()
	t2 = jia_clock()

	if (jiapid .eq. 0) then
		print *, 'Elapsed time is', t2-t1, ' seconds'
	endif

        temp = checka(1)
        if (temp.eq.-1) then 
           luerr = 'Incorrect results '
	   call jia_error(luerr)
        else 
           print *, ' Happily pass check!'
        endif
	call jia_exit()

	end

      function lua()
        include 'jiaf.h'
	integer begin
        real temp
	parameter  (n=256)
	common /shared/ pa
	pointer (pa, a)
  	real a(0:n-1,0:n-1)

        begin = j-j/jiahosts*jiahosts
        print *,'begin = ', begin
        do 20 j = 0,n-1
        if((j-j/jiahosts*jiahosts).eq.jiapid) then
            if (abs(a(j,j)).gt.EPSILON) then
              temp = a(j,j)
	      DO i = j+1, n-1 
	        a(i,j)=a(i,j)/temp
              enddo
 	    else 
              lua = -1
	      goto 100
            endif 
         endif 
       
	 call jia_barrier() 
         print *, 'pass column j=', j 
         begin=(j+1)/jiahosts*jiahosts+jiapid
         if (begin.lt.(j+1)) then
            begin=jiahosts+begin
	 endif
	 do 10 k=begin,n-1,jiahosts
           temp=a(j,k)
 	   do i=j+1, n-1
             a(i,k) = a(i,k)-a(i,j)*temp
	   enddo
10       continue  
20      continue
        lua =0
100     return 
	end

	function checka(check)
        integer check
        real temp, EPSILON
        parameter  (n=256)
        common /shared/ pa
        common /jia/ jiapid, jiahosts
        pointer (pa, a)
        real a(0:n-1,0:n-1),new(0:n-1,0:n-1)
	common /res/ old(0:n-1,0:n-1)
         
        EPSILON = 1e-5 
          if (jiapid.eq.0) then
             if (check.eq.1) then
               print *, 'Begin Check the result'
               
	       do 110 i = 0,n-1 
                 do 120 j= 0,n-1
                   temp = 0.0
                   do 130 k = 0, min(i,j) 
                     if (i.eq.k) then
                       temp = temp+a(k,j)
	             else 
                       temp = temp+a(i,k)*a(k,j)
                     endif
130                continue
                   new(i,j) = temp
                   if (abs(old(i,j)-new(i,j)).gt. EPSILON) then
                      print *, 'Incorrect!'
                      print *,'old(',i,',',j,')=',old(i,j) 
                      print *,'new(',i,',',j,')=',new(i,j)
                      checka = -1
                      goto 140
                   endif
120              continue
110            continue
              endif 
            endif
	    checka =0
140         return 
	    end
