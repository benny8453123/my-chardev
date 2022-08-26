#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define CHAR_DEV_PATH "/dev/my_chardev"
#define CHAR_DEV_BUF_SIZE 64

int main()
{
	char buf[CHAR_DEV_BUF_SIZE] = {0};
	int fd;
	int ret;
	size_t len;
	char message[] = "Hello World! This is FIFO buf";
	char *read_buffer;

	len = sizeof(message);

	fd = open(CHAR_DEV_PATH, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Open device %s failed!\n", CHAR_DEV_PATH);
		return -1;
	}

	ret = write(fd, message, len);
	if (ret != len) {
		fprintf(stderr, "Write message to %s failed!\n", CHAR_DEV_PATH);
		return -1;
	}

	read_buffer = malloc(len * 2);
	memset(read_buffer, 0, len * 2);
	printf("read_buffer = %s\n", read_buffer);
#ifdef USE_KFIFO
	printf("don't need to close device\n");
#else
	printf("close device\n");
	close(fd);

	fd = open(CHAR_DEV_PATH, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Reopen device %s failed!\n", CHAR_DEV_PATH);
		return -1;
	}
#endif /* USE_KFIFO */
	ret = read(fd, read_buffer, len * 2);
	printf("read %d bytes\n", ret);
	printf("read_buffer = %s\n", read_buffer);

	close(fd);

	return 0;
}
