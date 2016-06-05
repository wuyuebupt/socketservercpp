/* author : wuyue
*  email: wuyuebupt@gmail.com
*/

// socket
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
// socket 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;


int handle_session (int session_fd);

int
main (int argc, char **argv)
{
  int server_fd, portno;
  socklen_t chilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;

  // portno = 9001;
  portno = 9003;

  server_fd = socket (AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
    {
      cout << "error" << endl;
      exit (1);
    }

  // set the reuse addr to avoid waiting for time-wait 
  int reuseaddr = 1;
  if (setsockopt
      (server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
       sizeof (reuseaddr)) == -1)
    {
      exit (100);
    }

  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons (portno);


  if (bind (server_fd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) ==
      -1)
    {
      printf ("%s", strerror (errno));
      exit (1);
    }


  if (listen (server_fd, SOMAXCONN))
    {
      printf ("failed to listen for connections (errno=%d)", errno);
      exit (2);
    }


// avoid the defunct problem
  signal (SIGCHLD, SIG_IGN);

  for (;;)
    {
      cout << "start listening" << endl;
      int session_fd = accept (server_fd, 0, 0);
      if (session_fd == -1)
	{
	  if (errno == EINTR)
	    continue;
	  printf ("failed to accept connection (errno=%d)", errno);
	  exit (3);
	}
      pid_t pid = fork ();
      if (pid == -1)
	{
	  printf ("failed to create child process (errno=%d)", errno);
	  exit (4);
	}
      else if (pid == 0)
	{
	  close (server_fd);
	  int flag = handle_session (session_fd);
	  // cout << flag << endl;
	  cout << "finished handling the session" << endl;
	  close (session_fd);
	  _exit (0);
	}
      else
	{
	  close (session_fd);
	}
    }

  return -1;
}

// read function
void
ReadXBytes (int socket, unsigned int x, void *buffer)
{
  int bytesRead = 0;
  int result;
  while (bytesRead < x)
    {
      // cout << bytesRead << " " << x << endl;
      result = read (socket, buffer + bytesRead, x - bytesRead);
      // cout << char(buffer[bytesRead]) << endl;
      if (result < 1)
	{
	  cout << "result<1" << endl;
	  break;
	  // Throw your error.
	}

      bytesRead += result;
    }
}

// handle session for each request
int
handle_session (int session_fd)
{
// read the input
  unsigned int length1 = 0;
  char *buffer1 = 0;
  cout << "read length" << endl;
// we assume that sizeof(length) will return 4 here.
  ReadXBytes (session_fd, sizeof (length1), (void *) (&length1));
  cout << length1 << endl;
  buffer1 = new char[length1];
  cout << "read content" << endl;
  ReadXBytes (session_fd, length1, (void *) buffer1);
  cout << buffer1 << endl;

// split the strings
  istringstream iss (buffer1);

  string imagepath1;
  iss >> imagepath1;
  cout << imagepath1 << endl;
// end split the string


  string return_name = "return";
// return the current time and do nothing
  time_t now = time (0);
  char buffer[80];
  size_t length = strftime (buffer, sizeof (buffer), "%a %b %d %T %Y\r\n",
			    localtime (&now));
  // length = result_name.length();
  length = return_name.length ();
  if (length == 0)
    {
      snprintf (buffer, sizeof (buffer), "Error: buffer overflow\r\n");
    }

  size_t index = 0;

  cout << index << " " << length << endl;
  while (index < length)
    {
      // ssize_t count = write (session_fd, buffer + index, length - index);
      // ssize_t count = write (session_fd, result_name.c_str() + index, length - index);
      ssize_t count =
	write (session_fd, return_name.c_str () + index, length - index);
      if (count < 0)
	{
	  if (errno == EINTR)
	    continue;
	  printf ("failed to write to socket (errno=%d)", errno);
	}
      else
	{
	  index += count;
	}
    }
  return -1;
}
