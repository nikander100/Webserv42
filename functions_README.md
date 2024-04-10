Process Management

    execve: Executes a new program. This could be used in a web server to run CGI scripts or other external programs that generate dynamic content.
        Parameters: const char *pathname, char *const argv[], char *const envp[]
        Returns: int (0 on success, -1 on failure with errno set)
    fork: Creates a new process by duplicating the calling process. Useful for handling new connections concurrently.
        Parameters: None
        Returns: pid_t (process ID of the child process to the parent, 0 to the child, -1 on error)
    waitpid: Waits for a child process to change state (useful to clean up zombie processes).
        Parameters: pid_t pid, int *status, int options
        Returns: pid_t (ID of the child, -1 on error)
    kill: Sends a signal to a process. This can be used to manage child processes or to handle shutdown signals.
        Parameters: pid_t pid, int sig
        Returns: int (0 on success, -1 on failure)

Networking

    socket: Creates a socket for network communication.
        Parameters: int domain, int type, int protocol
        Returns: int (socket descriptor)
    bind: Associates a socket with a local address.
        Parameters: int sockfd, const struct sockaddr *addr, socklen_t addrlen
        Returns: int (0 on success, -1 on failure)
    listen: Marks the socket as a passive socket to accept incoming connection requests.
        Parameters: int sockfd, int backlog
        Returns: int (0 on success, -1 on failure)
    accept: Blocks the caller until a connection is present and accepts the connection.
        Parameters: int sockfd, struct sockaddr *addr, socklen_t *addrlen
        Returns: int (non-negative file descriptor on success, -1 on failure)
    connect: Initiates a connection on a socket (client-side).
        Parameters: int sockfd, const struct sockaddr *addr, socklen_t addrlen
        Returns: int (0 on success, -1 on failure)
    send, recv: Send and receive data over a network.
        Parameters (send/recv): int sockfd, const void *buf, size_t len, int flags
        Returns: ssize_t (number of bytes sent/received, -1 on failure)
    socketpair: Creates a pair of connected sockets.
        Parameters: int domain, int type, int protocol, int sv[2]
        Returns: int (0 on success, -1 on failure)
    getaddrinfo, freeaddrinfo: Provides network address and setting up socket addresses, and frees the memory allocated.
        Parameters (getaddrinfo): const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res
        Returns (getaddrinfo): int (0 on success, non-zero on failure)
    getsockname: Gets the current address to which the socket sockfd is bound.
        Parameters: int sockfd, struct sockaddr *addr, socklen_t *addrlen
        Returns: int (0 on success, -1 on failure)
    getprotobyname: Maps protocol name to protocol number.
        Parameters: const char *name
        Returns: struct protoent * (pointer to protoent structure)

I/O Multiplexing

    select, poll, epoll (including epoll_create, epoll_ctl, epoll_wait): These functions are used to monitor multiple file descriptors to see if I/O is possible on any of them.
        Parameters (vary based on function, generally include arrays or lists of file descriptors, timeout values, event lists)
        Returns: int (number of descriptors ready, 0 if timed out, -1 on error)
    kqueue, kevent: Similar to epoll but specific to BSD systems, used for event notification.
        Parameters: Varies
        Returns: Varies

File and Directory Management

    chdir: Changes the current working directory. Useful if the server needs to change the directory context.
        Parameters: const char *path
        Returns: int (0 on success, -1 on failure)
    stat: Retrieves information about the file (like size, permissions).
        Parameters: const char *pathname, struct stat *statbuf
        Returns: int (0 on success, -1 on failure)
    open, read, write, close: Basic file operations to open, read from, write to, and close files.
        Parameters: Varies by function but generally include file path, flags, mode (for open), buffer and count (for read/write)
        Returns: Varies by function (generally file descriptor for open, byte count for read/write, success/failure indication for close)
    opendir, readdir, closedir: Manage directory streams, list files in a directory.
        Parameters: Directory path (opendir), directory stream pointer (readdir, closedir)
        Returns: Directory stream pointer (opendir), directory entry structure (readdir), status code (closedir)

Error Handling and Miscellaneous

    errno, strerror, gai_strerror: Used for error reporting. errno is set by system calls and some library functions in the event of an error to indicate what went wrong.
    dup, dup2: Duplicate a file descriptor, can be used to redirect I/O.
    pipe: Creates a unidirectional data channel that can be used for inter-process communication.
    fcntl: Perform various operations on a file descriptor, such as setting it to non-blocking mode.
    setsockopt: Set options on the socket, such as SO_REUSEADDR, which allows a socket to forcibly bind to a port in use by another socket.
    signal: Installs a signal handler. This is useful for cleanly shutting down the server on interrupt.
    access: Checks the file's accessibility. Useful for verifying if a file exists and can be accessed before trying to open it.