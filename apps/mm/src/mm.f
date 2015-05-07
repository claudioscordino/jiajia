	program matrix

	include 'jiaf.h'

	parameter  (n=1024)
	integer i, j
	common /shared/ pa, pb, pc
	pointer (pa, a), (pb, b), (pc, c)
  	real a(n,n), b(n,n), c(n,n), t1, t2

	external mminit


        call jiaf_init()

	call jia_barrier()
        print *, 'jiahosts = ', jiahosts
c	call mminit(%val(n),%val(jiahosts))
	pa = jia_alloc(%val(4*n*n),%val(4*n*n/jiahosts),%val(0))
	pb = jia_alloc(%val(4*n*n),%val(4*n*n/jiahosts),%val(0))
	pc = jia_alloc(%val(4*n*n),%val(4*n*n/jiahosts),%val(0))

	if (jiapid .eq. 0) then
		do j = 1, n
		do i = 1, n
			a(i,j) = 1.0
			b(i,j) = 1.0
		end do
		end do
	endif

	call jia_barrier()

	t1 = jia_clock()

	call worker()

	call jia_barrier()
	t2 = jia_clock()

	if (jiapid .eq. 0) then
		print *, 'Elapsed time is', t2-t1, ' seconds'
	endif

	call jia_exit()

	end

      subroutine worker()
        include 'jiaf.h'
	integer begin, end
	parameter  (n=1024)
	common /shared/ pa, pb, pc
c	common /jia/ jiapid, jiahosts
	pointer (pa, a), (pb, b), (pc, c)
  	real a(n,n), b(n,n), c(n,n)

	begin = n/jiahosts*jiapid + 1
	end = n/jiahosts*(jiapid+1)
	do j = begin, end
	  do i = 1, n
		t = 0
		do k = 1, n
			t = t + a(k,i)*b(k,j)
		end do
		c(i,j) = t
		if ((jiapid .eq. 0) .and. (i .eq. 1)) then
			print *, 'c(', i, j, ') = ', c(i,j)
		endif
	  end do
	end do
        call jia_barrier()
	return
	end
