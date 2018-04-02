




compile:
	$ make

run:
diskinfo:       $ ./diskinfo [.img file ]
disklist:       $ ./disklist [.img file]
diskget:        $ ./diskget [.img file] [output_file] [input_file]
diskput:	$ ./diskput [.img file] [input_file]


due to some memory allocation problem, output file from ./diskput does not return the exactly same file(some random marks follow the last char)
if the ./disklist does not return the right file type, it is because i hard coded file type in diskput.c and modify time.
the output file from diskget goes to a output folder.


