/* Copyright 2019 HuaTsai */
/* open(2) */
#include <fcntl.h>

/* stat(2), mkdir(2), chmod(2) */
#include <sys/stat.h>

/* read(2), write(2), close(2), dup(2)
 * chdir(2), chown(2), rmdir(2), pread(2)
 * link(2), unlink(2), readlink(2), symlink(2) */
#include <unistd.h>

/* opendir(3), readdir(3), closedir(3) */
#include <dirent.h>

/* fopen(3), fclose(3), remove(3), rename(2)
 * fgetc(3), fread(3), scanf(3), printf(3) */
#include <stdio.h>

/* Get Original Function */
#include <dlfcn.h>

/* Other headers */
#include <ctype.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define PATH_SIZE 256
#define STAT_SIZE 30
#define NAME_SIZE 30

#define GET_ORIGIN_FUNCTION(name, return_type, ...)     \
  static return_type (*name##_ori)(__VA_ARGS__) = NULL; \
  if (name##_ori == NULL) {                             \
    name##_ori = dlsym(handle, #name);                  \
  }

#define GET_RESULT(name, return_type, ...) \
  return_type result = name##_ori(__VA_ARGS__);

#define PRINT_GENERAL(name, input_format, output_format, ...)             \
  fprintf_ori(out, "# " #name "(" input_format ") = " output_format "\n", \
              __VA_ARGS__);

#define PRINT_RESULT(name, input_format, output_format, ...)  \
  PRINT_GENERAL(name, input_format, output_format, __VA_ARGS__, result)

#define RETURN_RESULT  \
  return result;

int (*fprintf_ori)(FILE *, const char *, ...) = NULL;
ssize_t (*readlink_ori)(const char *, char *, size_t) = NULL;
FILE *(*fopen_ori)(const char *, const char *) = NULL;
int (*fclose_ori)(FILE *) = NULL;
void *handle;
FILE *out;

/******** constructor and destructor ********/
__attribute__((constructor)) void prework() {
  handle = dlopen("libc.so.6", RTLD_LAZY);
  if (handle == NULL) {
    printf("cannot get libc.so.6 library\n");
    exit(1);
  }
  fprintf_ori = dlsym(handle, "fprintf");
  readlink_ori = dlsym(handle, "readlink");
  fopen_ori = dlsym(handle, "fopen");
  fclose_ori = dlsym(handle, "fclose");
  char *fpstr = getenv("MONITOR_OUTPUT");
  if (fpstr == NULL) {
    out = stderr;
  } else {
    out = fopen_ori(fpstr, "w");
  }
}

__attribute__((destructor)) void postwork() {
  fclose_ori(out);
}

/************* common functions *************/
void fd_to_filename(int fd, char *filename) {
  if (fd < 0) {
    memcpy(filename, "<ERROR>", 8);
  } else if (fd == 0) {
    memcpy(filename, "<STDIN>", 8);
  } else if (fd == 1) {
    memcpy(filename, "<STDOUT>", 9);
  } else if (fd == 2) {
    memcpy(filename, "<STDERR>", 9);
  } else {
    char linkpath[PATH_SIZE];
    snprintf(linkpath, sizeof(linkpath), "/proc/self/fd/%d", fd);
    ssize_t end = readlink_ori(linkpath, filename, PATH_SIZE);
    filename[end] = '\0';
  }
}

void dirp_to_directory(DIR *dirp, char *filename) {
  int fd = dirfd(dirp);
  fd_to_filename(fd, filename);
}

void fp_to_filename(FILE *fp, char *filename) {
  int fd = fileno(fp);
  fd_to_filename(fd, filename);
}

void stat_to_str(struct stat *buf, char *str) {
  snprintf(str, 30, "{mode=%#o, size=%ld}", buf->st_mode, buf->st_size);
}

void uid_to_name(uid_t uid, char *username) {
  struct passwd *user = getpwuid(uid);
  memcpy(username, user->pw_name, NAME_SIZE);
}

void gid_to_name(gid_t gid, char *groupname) {
  struct group *grp = getgrgid(gid);
  memcpy(groupname, grp->gr_name, NAME_SIZE);
}

/****************** fcntl.h *****************/
int creat(const char *path, mode_t mode) {
  GET_ORIGIN_FUNCTION(creat, int, const char *, mode_t)
  GET_RESULT(creat, int, path, mode)
  char filename[PATH_SIZE];
  fd_to_filename(result, filename);
  PRINT_GENERAL(creat, "\"%s\", %#o", "\"%s\"", path, mode, filename)
  RETURN_RESULT
}

int open(const char *path, int flags, ...) {
  GET_ORIGIN_FUNCTION(open, int, const char *, int, ...)
  va_list arg;
  va_start(arg, flags);
  int result;
  if (flags & O_CREAT) {
    mode_t mode = va_arg(arg, mode_t);
    result = open_ori(path, flags, mode);
    PRINT_RESULT(open, "\"%s\", %d, %#o", "%d", path, flags, mode)
  } else {
    result = open_ori(path, flags);
    PRINT_RESULT(open, "\"%s\", %d", "%d", path, flags)
  }
  va_end(arg);
  RETURN_RESULT
}

/****************** stat.h ******************/
int __xstat(int ver, const char *filename, struct stat *stat_buf) {
  GET_ORIGIN_FUNCTION(__xstat, int, int, const char *, struct stat *)
  GET_RESULT(__xstat, int, ver, filename, stat_buf)
  char statstr[STAT_SIZE];
  stat_to_str(stat_buf, statstr);
  PRINT_RESULT(__xstat, "%d, \"%s\", %p %s", "%d", ver,
               filename, stat_buf, statstr)
  RETURN_RESULT
}

int __lxstat(int ver, const char *filename, struct stat *stat_buf) {
  GET_ORIGIN_FUNCTION(__lxstat, int, int, const char *, struct stat *)
  GET_RESULT(__lxstat, int, ver, filename, stat_buf)
  char statstr[STAT_SIZE];
  stat_to_str(stat_buf, statstr);
  PRINT_RESULT(__lxstat, "%d, \"%s\", %p %s", "%d", ver,
               filename, stat_buf, statstr)
  RETURN_RESULT
}

int mkdir(const char *path, mode_t mode) {
  GET_ORIGIN_FUNCTION(mkdir, int, const char *, mode_t)
  GET_RESULT(mkdir, int, path, mode)
  PRINT_RESULT(mkdir, "\"%s\", %#o", "%d", path, mode)
  RETURN_RESULT
}

int chmod(const char *path, mode_t mode) {
  GET_ORIGIN_FUNCTION(chmod, int, const char *, mode_t)
  GET_RESULT(chmod, int, path, mode)
  PRINT_RESULT(chmod, "\"%s\", %#o", "%d", path, mode)
  RETURN_RESULT
}

/***************** unistd.h *****************/
ssize_t read(int fd, void *buf, size_t count) {
  GET_ORIGIN_FUNCTION(read, ssize_t, int, void *, size_t)
  GET_RESULT(read, ssize_t, fd, buf, count)
  char filename[PATH_SIZE];
  fd_to_filename(fd, filename);
  PRINT_RESULT(read, "\"%s\", \"%s\", %ld", "%ld", filename, buf, count)
  RETURN_RESULT
}

ssize_t write(int fd, const void *buf, size_t count) {
  GET_ORIGIN_FUNCTION(write, ssize_t, int, const void *, size_t)
  GET_RESULT(write, ssize_t, fd, buf, count)
  char filename[PATH_SIZE];
  fd_to_filename(fd, filename);
  PRINT_RESULT(write, "\"%s\", \"%s\", %ld", "%ld", filename, buf, count)
  RETURN_RESULT
}

int close(int fd) {
  GET_ORIGIN_FUNCTION(close, int, int)
  char filename[PATH_SIZE];
  fd_to_filename(fd, filename);
  GET_RESULT(close, int, fd)
  PRINT_RESULT(close, "\"%s\"", "%d", filename)
  RETURN_RESULT
}

int dup(int oldfd) {
  GET_ORIGIN_FUNCTION(dup, int, int)
  char filename[PATH_SIZE];
  fd_to_filename(oldfd, filename);
  GET_RESULT(dup, int, oldfd)
  PRINT_GENERAL(dup, "%d", "%d (dup \"%s\")", oldfd, result, filename)
  RETURN_RESULT
}

int dup2(int oldfd, int newfd) {
  GET_ORIGIN_FUNCTION(dup2, int, int, int)
  char filename[PATH_SIZE];
  fd_to_filename(oldfd, filename);
  GET_RESULT(dup2, int, oldfd, newfd)
  PRINT_GENERAL(dup2, "%d, %d", "%d (dup \"%s\")",
                oldfd, newfd, result, filename)
  RETURN_RESULT
}

int chdir(const char *path) {
  GET_ORIGIN_FUNCTION(chdir, int, const char *)
  GET_RESULT(chdir, int, path)
  PRINT_RESULT(chdir, "\"%s\"", "%d", path)
  RETURN_RESULT
}

int chown(const char *path, uid_t owner, gid_t group) {
  GET_ORIGIN_FUNCTION(chown, int, const char *, uid_t, gid_t)
  GET_RESULT(chown, int, path, owner, group)
  char ownername[NAME_SIZE];
  char groupname[NAME_SIZE];
  uid_to_name(owner, ownername);
  gid_to_name(group, groupname);
  PRINT_RESULT(chown, "\"%s\", \"%s\", \"%s\"", "%d",
               path, ownername, groupname)
  RETURN_RESULT
}

int rmdir(const char *path) {
  GET_ORIGIN_FUNCTION(rmdir, int, const char *)
  GET_RESULT(rmdir, int, path)
  PRINT_RESULT(rmdir, "\"%s\"", "%d", path)
  RETURN_RESULT
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
  GET_ORIGIN_FUNCTION(pwrite, ssize_t, int, const void *, size_t, off_t)
  GET_RESULT(pwrite, ssize_t, fd, buf, count, offset)
  char filename[PATH_SIZE];
  fd_to_filename(fd, filename);
  PRINT_RESULT(pwrite, "\"%s\", \"%s\", %ld, %ld", "%ld",
               filename, buf, count, offset)
  RETURN_RESULT
}

int link(const char *oldpath, const char *newpath) {
  GET_ORIGIN_FUNCTION(link, int, const char *, const char *)
  GET_RESULT(link, int, oldpath, newpath)
  PRINT_RESULT(link, "\"%s\", \"%s\"", "%d", oldpath, newpath)
  RETURN_RESULT
}

int unlink(const char *path) {
  GET_ORIGIN_FUNCTION(unlink, int, const char *)
  GET_RESULT(unlink, int, path)
  PRINT_RESULT(unlink, "\"%s\"", "%d", path)
  RETURN_RESULT
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz) {
  GET_ORIGIN_FUNCTION(readlink, ssize_t, const char *, char *, size_t)
  GET_RESULT(readlink, ssize_t, path, buf, bufsiz)
  PRINT_RESULT(readlink, "\"%s\", \"%s\", %ld", "%ld", path, buf, bufsiz)
  RETURN_RESULT
}

int symlink(const char *target, const char *linkpath) {
  GET_ORIGIN_FUNCTION(symlink, int, const char *, const char *)
  GET_RESULT(symlink, int, target, linkpath)
  PRINT_RESULT(symlink, "\"%s\", \"%s\"", "%d", target, linkpath)
  RETURN_RESULT
}

/***************** dirent.h *****************/
DIR *opendir(const char *name) {
  GET_ORIGIN_FUNCTION(opendir, DIR *, const char *)
  GET_RESULT(opendir, DIR *, name)
  char dirname[PATH_SIZE];
  dirp_to_directory(result, dirname);
  PRINT_GENERAL(opendir, "\"%s\"", "\"%s\"", name, dirname)
  RETURN_RESULT
}

struct dirent *readdir(DIR *dirp) {
  GET_ORIGIN_FUNCTION(readdir, struct dirent *, DIR *)
  char dirname[PATH_SIZE];
  dirp_to_directory(dirp, dirname);
  GET_RESULT(readdir, struct dirent *, dirp)
  if (result) {
    PRINT_GENERAL(readdir, "\"%s\"", "%p {\"%s\"}",
                  dirname, result, result->d_name)
  } else {
    PRINT_RESULT(readdir, "\"%s\"", "%p", dirname, result)
  }
  RETURN_RESULT
}

int closedir(DIR *dirp) {
  GET_ORIGIN_FUNCTION(closedir, int, DIR *)
  char dirname[PATH_SIZE];
  dirp_to_directory(dirp, dirname);
  GET_RESULT(closedir, int, dirp)
  PRINT_RESULT(closedir, "\"%s\"", "%d", dirname)
  RETURN_RESULT
}

/***************** stdio.h *****************/
FILE *fopen(const char *pathname, const char *mode) {
  GET_ORIGIN_FUNCTION(fopen, FILE *, const char *, const char *)
  GET_RESULT(fopen, FILE *, pathname, mode)
  char fpname[PATH_SIZE];
  fp_to_filename(result, fpname);
  PRINT_GENERAL(fopen, "\"%s\", \"%s\"", "\"%s\"",
                pathname, mode, fpname)
  RETURN_RESULT
}

int fclose(FILE *stream) {
  GET_ORIGIN_FUNCTION(fclose, int, FILE *)
  char filename[PATH_SIZE];
  fp_to_filename(stream, filename);
  GET_RESULT(fclose, int, stream)
  PRINT_RESULT(fclose, "\"%s\"", "%d", filename)
  RETURN_RESULT
}

int remove(const char *path) {
  GET_ORIGIN_FUNCTION(remove, int, const char *)
  GET_RESULT(remove, int, path)
  PRINT_RESULT(remove, "\"%s\"", "%d", path)
  RETURN_RESULT
}

int rename(const char *oldpath, const char *newpath) {
  GET_ORIGIN_FUNCTION(rename, int, const char *, const char *)
  GET_RESULT(rename, int, oldpath, newpath)
  PRINT_RESULT(rename, "\"%s\", \"%s\"", "%d", oldpath, newpath)
  RETURN_RESULT
}

int fgetc(FILE *stream) {
  GET_ORIGIN_FUNCTION(fgetc, int, FILE *)
  GET_RESULT(fgetc, int, stream)
  char filename[PATH_SIZE];
  fp_to_filename(stream, filename);
  if (isalnum(result)) {
    PRINT_RESULT(fgetc, "%p {\"%s\"}", "'%c'", stream, filename)
  } else if (result == '\n') {
    PRINT_GENERAL(fgetc, "%p {\"%s\"}", "'\\n'", stream, filename)
  } else if (result == EOF) {
    PRINT_GENERAL(fgetc, "%p {\"%s\"}", "<EOF>", stream, filename)
  } else {
    PRINT_GENERAL(fgetc, "%p {\"%s\"}", "<NOT ALNUM/NEWLINE/EOF>",
                  stream, filename)
  }
  RETURN_RESULT
}

char *fgets(char *s, int size, FILE *stream) {
  GET_ORIGIN_FUNCTION(fgets, char *, char *, int, FILE *)
  GET_RESULT(fgets, char *, s, size, stream)
  PRINT_RESULT(fgets, "\"%s\", %d, %p", "\"%s\"", s, size, stream)
  RETURN_RESULT
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  GET_ORIGIN_FUNCTION(fread, size_t, void *, size_t, size_t, FILE *)\
  GET_RESULT(fread, size_t, ptr, size, nmemb, stream)
  char filename[PATH_SIZE];
  fp_to_filename(stream, filename);
  PRINT_RESULT(fread, "%p, %ld, %ld, \"%s\"", "%ld", ptr, size, nmemb, filename)
  RETURN_RESULT
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  GET_ORIGIN_FUNCTION(fwrite, size_t, const void *, size_t, size_t, FILE *)
  GET_RESULT(fwrite, size_t, ptr, size, nmemb, stream)
  char filename[PATH_SIZE];
  fp_to_filename(stream, filename);
  PRINT_RESULT(fwrite, "%p, %ld, %ld, \"%s\"", "%ld",
               ptr, size, nmemb, filename)
  RETURN_RESULT
}

int fscanf(FILE *stream, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  int result = vfscanf(stream, format, arg);
  va_end(arg);
  char filename[PATH_SIZE];
  fp_to_filename(stream, filename);
  PRINT_RESULT(fscanf, "\"%s\", \"%s\", ...", "%d", filename, format)
  return result;
}

int fprintf(FILE *stream, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  int result = vfprintf(stream, format, arg);
  va_end(arg);
  char filename[PATH_SIZE];
  fp_to_filename(stream, filename);
  PRINT_RESULT(fprintf, "\"%s\", \"%s\", ...", "%d", filename, format)
  return result;
}
