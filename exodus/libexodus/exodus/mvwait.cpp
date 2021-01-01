#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <vector>
#include <list>
#include <string>

#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

namespace exodus {

//Args is a class to create temporary argv (char*[]) from a var string
//that will autodestruct when out of scope and not leak memory
class Args {

    //a place to store primitive c style strings
	//until object destruction
    std::list<std::string> strings;

	//a vector of char to represent char* argv[]
    std::vector<char*> argv_;

public:

	//constructor from var
	Args(const var& command) {
	    //printl(command);
	    for (var word : command) {
	        //printl(word);
	        argv_.push_back(strings.emplace_back(std::string(word ^ "\x00")).data());
	    }
	    //last arg must be nullptr
	    argv_.push_back(nullptr);
	}

	//return argc as int
	int argc() {
	    return argv_.size()-1;
	}

	//return argv_ as char*[]
	const char** argv() {
	    return (const char**) argv_.data();
	}

};

/* Read all available inotify events from the file descriptor 'fd'.
  wd is the table of watch descriptors for the directories in argv.
  argc is the length of wd and argv.
  argv is the list of watched directories.
  Entry 0 of wd and argv is unused. */

//return true if any relevent events
bool handle_events(int fd, int *wd, const int argc, const char* argv[])
{
   /* Some systems cannot read integer variables if they are not
      properly aligned. On other systems, incorrect alignment may
      decrease performance. Hence, the buffer used for reading from
      the inotify file descriptor should have the same alignment as
      struct inotify_event. */

   char buf[4096]
       __attribute__ ((aligned(__alignof__(struct inotify_event))));
   const struct inotify_event *event;
   int i;
   ssize_t len;
   char *ptr;

	//printf("Handling events\n");

   /* Loop while events can be read from inotify file descriptor. */

   for (;;) {

       /* Read some events. */

       len = read(fd, buf, sizeof buf);
       if (len == -1 && errno != EAGAIN) {
           perror("read");
           //exit(EXIT_FAILURE);
           return false;
       }

       /* If the nonblocking read() found no events to read, then
          it returns -1 with errno set to EAGAIN. In that case,
          we exit the loop. */

       if (len <= 0)
           break;

       //printf("Loop over all events in the buffer\n");

       for (ptr = buf; ptr < buf + len;
               ptr += sizeof(struct inotify_event) + event->len) {

           event = (const struct inotify_event *) ptr;

           //printf("Print event type\n");

           //if (event->mask & IN_OPEN)
           //    printf("IN_OPEN: ");
           //if (event->mask & IN_CLOSE_NOWRITE)
           //    printf("IN_CLOSE_NOWRITE: ");
			if (event->mask & IN_CLOSE_WRITE)
            	printf("IN_CLOSE_WRITE: ");
		//else if (event->mask & IN_ACCESS)
		//    printf("Data was read from file.");
		//else if (event->mask & IN_MODIFY)
		//    printf("Data was written to file.");
		//else if (event->mask & IN_ATTRIB)
		//    printf("File attributes changed.");
			else if (event->mask & IN_CLOSE_WRITE)
			    printf("File opened for write was closed.");
		//else if (event->mask & IN_CLOSE_NOWRITE)
		//    printf("File opened for read was closed.");
		//else if (event->mask & IN_CLOSE)
		//    printf("File was closed (read or write).");
		//else if (event->mask & IN_OPEN)
		//    printf("File was opened.");
			else if (event->mask & IN_MOVED_FROM)
			    printf("File was moved away from watched directory.");
			else if (event->mask & IN_MOVED_TO)
			    printf("File was moved into watched directory.");
			else if (event->mask & IN_MOVE)
			    printf("File was moved (in or out of directory).");
			else if (event->mask & IN_CREATE) {}
//			    printf("A file was created in the directory.");
//			else if (event->mask & IN_DELETE)
//			    printf("A file was deleted from the directory.");
			else if (event->mask & IN_DELETE_SELF)
			    printf("Directory or file under observation was deleted.");
			else if (event->mask & IN_MOVE_SELF)
			    printf("Directory or file under observation was moved.");

			else continue;

return true;

           printf("Print the name of the watched directory\n");

           for (i = 1; i < argc; ++i) {
               if (wd[i] == event->wd) {
                   printf("%s/", argv[i]);
                   break;
               }
           }

           /* Print the name of the file */

           if (event->len)
               printf("%s", event->name);

           /* Print type of filesystem object */

           if (event->mask & IN_ISDIR)
               printf(" [directory]\n");
           else
               printf(" [file]\n");


			return true;
       }
   }
	return false;
}

var wait_main(const int argc, const char* argv[], const int wait_time_ms)
{
   //char buf;
   int fd, i, poll_num;
   int *wd;
   nfds_t nfds;
   struct pollfd fds[2];

   if (argc < 2) {
       printf("Usage: %s PATH [PATH ...]\n", argv[0]);
       //exit(EXIT_FAILURE);
       return "";
   }

   //printf("Press ENTER key to terminate.\n");

   /* Create the file descriptor for accessing the inotify API */

   fd = inotify_init1(IN_NONBLOCK);
   if (fd == -1) {
       perror("inotify_init1");
       exit(EXIT_FAILURE);
       //exit(EXIT_FAILURE);
       return "";
   }

   /* Allocate memory for watch descriptors */

   wd = (int*)calloc(argc, sizeof(int));
   if (wd == NULL) {
       perror("calloc");
       exit(EXIT_FAILURE);
       //exit(EXIT_FAILURE);
       return "";
   }

   /* Mark directories for events
      - file was opened
      - file was closed */

   for (i = 1; i < argc; i++) {
       wd[i] = inotify_add_watch(fd, argv[i],
//                                 IN_OPEN | IN_CLOSE);
                                 IN_ALL_EVENTS);
       if (wd[i] == -1) {
           fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
           perror("inotify_add_watch");
           exit(EXIT_FAILURE);
       //exit(EXIT_FAILURE);
       return "";
       }
   }

   /* Prepare for polling */

   nfds = 2;

   /* Console input */

   fds[0].fd = STDIN_FILENO;
   fds[0].events = POLLIN;

   /* Inotify input */

   fds[1].fd = fd;
   fds[1].events = POLLIN;

   /* Wait for events and/or terminal input */

  // printf("Listening for events.\n");

   do {
       poll_num = poll(fds, nfds, wait_time_ms);
       if (poll_num == -1) {
           if (errno == EINTR)
               continue;
           perror("poll");
           exit(EXIT_FAILURE);
       //exit(EXIT_FAILURE);
       return "";
       }

       if (poll_num > 0) {

           if (fds[0].revents & POLLIN) {

               /* Console input is available. Empty stdin and quit */

               //while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
               //    continue;
               break;
           }

           if (fds[1].revents & POLLIN) {

               /* Inotify events are available */
				//quit if any relevent events
               if (handle_events(fd, wd, argc, argv))
                    break;
           }
       } else if (poll_num == 0) {
           //printf("Timeout listening for events.\n");
			break;
       }
   } while (true);

   //printf("Listening for events stopped.\n");

   /* Close inotify file descriptor */

   close(fd);

   free(wd);
   //exit(EXIT_SUCCESS);
   return "";
}

void var::oswait(const int milliseconds,const var& directory) const
{
    THISIS("void var::oswait(const int milliseconds, const var directory) const")
    // doesnt use *this - should syntax be changed to setcwd? and getcwd()?
    THISISDEFINED() // not needed if *this not used
	ISSTRING(directory)

	Args args(FM ^ directory);

	wait_main(args.argc(),args.argv(), milliseconds);

}

}//namespace
