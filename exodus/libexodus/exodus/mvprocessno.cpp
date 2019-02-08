#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

namespace exodus {

int getprocessno(const char* filename, int* fd)
{
	struct flock fl;

	*fd = open(filename, O_RDWR | O_CREAT);
	if (*fd == -1)
		printf("getprocessno file %s doesnt exist\n",filename);


	for (int ii=1;ii<1000;ii++) {

		/* Make a non-blocking request to place a write lock
		on bytes 100-109 of testfile */

		fl.l_type = F_WRLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = ii;
		fl.l_len = 1;
		if (fcntl(*fd, F_SETLK, &fl) == -1) {

			if (errno == EACCES || errno == EAGAIN) {
				//printf("Already locked by another process\n");
        		} else {
				/* Handle unexpected error */;
			}

		} else {

			//printf("Locked was granted for %i\n", ii);
			//char c=getchar();
			//DONT close fd otherwise lock will be released.
			return ii;

			//dont unlock it
			//fl.l_type = F_UNLCK;
			//fl.l_whence = SEEK_SET;
			//fl.l_start = 100;
			//fl.l_len = 10;
			//if (fcntl(*fd, F_SETLK, &fl) == -1) {
            		//	/* Handle error */;
			//}
		}
	}

	//close if cannot get 1st 1000 numbers
	printf("getprocessno tried all possible locks and failed\n");
	close (*fd);
	*fd=0;

	return 0;
}

//should be called in mvenvironment destructor
void releaseprocess(int* fd) {
	close(*fd);
	*fd=0;
}

}//namespace

