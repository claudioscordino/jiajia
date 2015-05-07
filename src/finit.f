
	subroutine jiaf_init
	external jia_init !$pragma C(jia_init)
	external ident
	parameter (n = 16)
	dimension argv(n), argument(n)
	integer i, j, argc, argv
	character*128 argument

	argc = iargc()
	do i = 0, argc
	    call getarg(i, argument(i+1))
	    argv(i+1) = loc(argument(i+1))
	end do

	do i = 1, argc+1
	    do j = 1, 128
		if (argument(i)(j:j) .eq. '') then
		    argument(i)(j:j) = '\0'
		    goto 10
		endif
	    enddo
	    argument(i)(128:128) = '\0'
	    print*, 'Too long command line argument!'
 10	enddo

	call jia_init(%val(argc+1), argv)
	call ident()
	return
	end


      subroutine jia_distask(parbegin,parend,seqbegin,seqend)
      integer parbegin,parend,seqbegin,seqend
      common /jia/ jiapid, jiahosts
      integer temp

      temp = seqend-seqbegin+1
      if ((temp mod jiahosts) .ne. 0)  then
        temp = (temp/jiahosts+1)*jiahosts
      endif
      parbegin = temp/jiahosts*jiapid+seqbegin
      parend   = parbegin+temp/jiahosts-1
      if (jiapid .eq. (jiahosts-1)) parend = seqend
      return
      end

