//#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
//#include <errno.h>
#include <stdio.h>
#include <string>
#include <iostream>

namespace exodus
{

// here because unistd.h available
std::string mvgethostname()
{
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	return hostname;
}

bool processno_islocked2(int processno, int* fd)
{

	/* Make a non-blocking request to place a write lock
	on 1 byte at offset processno of testfile */

	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = processno;
	fl.l_len = 1;

	// process cannot be locked, return true to indicate processno is active
	if (fcntl(*fd, F_SETLK, &fl) == -1)
	{

		if (errno == EACCES || errno == EAGAIN)
		{
			// printf("Already locked by another process\n");
		}
		else
		{
			/* Handle unexpected error */;
		}
		return true;

		// process can be locked, return false to indicate processno is active
	}
	else
	{

		// unlock it immediately
		fl.l_type = F_UNLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = processno;
		fl.l_len = 1;
		if (fcntl(*fd, F_SETLK, &fl) == -1)
		{
			/* Handle error */;
		}
		return false;
	}
}

int getprocessno(const char* filename, int* fd)
{
	struct flock fl;

	*fd = open(filename, O_RDWR | O_CREAT, 0666);
	//*fd = open(filename, O_RDWR | O_CREAT | O_CLOEXEC, 0666);
	// *fd = open(filename, O_RDONLY, 0);
	if (*fd == -1) {
		printf("exodus getprocessno file '%s' doesnt exist\n", filename);
		return 0;
	}
	// processno starts at 1 not 0

	for (int ii = 1; ii < 1000; ii++)
	{

		/* Make a non-blocking request to place a write lock
		on 1 byte at offset processno of testfile */

		fl.l_type = F_WRLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = ii;
		fl.l_len = 1;
		if (fcntl(*fd, F_SETLK, &fl) == -1)
		{

			//			if (errno == EACCES || errno == EAGAIN) {
			//				//printf("Already locked by another
			// process\n");
			//        		} else {
			//				/* Handle unexpected error */;
			//			}
		}
		else
		{

std::cout << ii << " #############################################################################################################" << std::endl;
			// printf("Locked was granted for %i\n", ii);
			// char c=getchar();
			// DONT close fd otherwise lock will be released.
			return ii;

			// dont unlock it
			// fl.l_type = F_UNLCK;
			// fl.l_whence = SEEK_SET;
			// fl.l_start = 100;
			// fl.l_len = 10;
			// if (fcntl(*fd, F_SETLK, &fl) == -1) {
			//	/* Handle error */;
			//}
		}
	}

	// close if cannot get 1st 1000 numbers
	printf("getprocessno tried all possible locks and failed\n");
	close(*fd);
	*fd = 0;

	return 0;
}

// should be called in mvenvironment destructor
void releaseprocess(int* fd)
{
	close(*fd);
	*fd = 0;
}

} // namespace exodus
