/* Copyright 2019 HuaTsai */
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define TCP_FLAG 0b0001
#define UDP_FLAG 0b0010
#define IPV4_FLAG 0b0100
#define IPV6_FLAG 0b1000

struct connection4 {
  char proto[5];
  struct in_addr local_ip;
  in_port_t local_port;
  struct in_addr remote_ip;
  in_port_t remote_port;
  pid_t pid;
  char program[256];
  uint32_t inode;
};

typedef struct contnode4 {
  struct connection4 cont;
  struct contnode4 *next;
} ContNode4;

struct connection6 {
  char proto[5];
  struct in6_addr local_ip;
  in_port_t local_port;
  struct in6_addr remote_ip;
  in_port_t remote_port;
  pid_t pid;
  char program[256];
  uint32_t inode;
};

typedef struct contnode6 {
  struct connection6 cont;
  struct contnode6 *next;
} ContNode6;

void HandleParameter(int argc, char **argv, int *mode, char *filter);
void ReadConnections(void **head, const int mode);
void FindPIDAndProgramName(void *head, const int mode);
void ShowHeader(const int mode);
void ListConnectionsWithFilter(void *head, const int mode, char *filter);
void FreeMemory(void *head, const int mode);

int main(int argc, char *argv[]) {
  int mode = 0;
  char filter[256];
  HandleParameter(argc, argv, &mode, filter);
  ContNode4 *tcp4_head = NULL;
  ContNode4 *udp4_head = NULL;
  ContNode6 *tcp6_head = NULL;
  ContNode6 *udp6_head = NULL;
  if (mode & TCP_FLAG) {
    ReadConnections((void **)&tcp4_head, TCP_FLAG | IPV4_FLAG);
    ReadConnections((void **)&tcp6_head, TCP_FLAG | IPV6_FLAG);
    FindPIDAndProgramName(tcp4_head, IPV4_FLAG);
    FindPIDAndProgramName(tcp6_head, IPV6_FLAG);
    ShowHeader(TCP_FLAG);
    ListConnectionsWithFilter(tcp4_head, IPV4_FLAG, filter);
    ListConnectionsWithFilter(tcp6_head, IPV6_FLAG, filter);
    printf("\n");
    FreeMemory(tcp4_head, IPV4_FLAG);
    FreeMemory(tcp6_head, IPV6_FLAG);
  }
  if (mode & UDP_FLAG) {
    ReadConnections((void **)&udp4_head, UDP_FLAG | IPV4_FLAG);
    ReadConnections((void **)&udp6_head, UDP_FLAG | IPV6_FLAG);
    FindPIDAndProgramName(udp4_head, IPV4_FLAG);
    FindPIDAndProgramName(udp6_head, IPV6_FLAG);
    ShowHeader(UDP_FLAG);
    ListConnectionsWithFilter(udp4_head, IPV4_FLAG, filter);
    ListConnectionsWithFilter(udp6_head, IPV6_FLAG, filter);
    printf("\n");
    FreeMemory(udp4_head, IPV4_FLAG);
    FreeMemory(udp6_head, IPV6_FLAG);
  }

  return 0;
}

void HandleParameter(int argc, char **argv, int *mode, char *filter) {
  struct option opts[] = {{"tcp", no_argument, NULL, 't'},
                          {"udp", no_argument, NULL, 'u'}};
  int c, t = 0, u = 0;
  while ((c = getopt_long(argc, argv, "tu", opts, NULL)) != -1) {
    switch (c) {
      case 't':
        t = 1;
        break;
      case 'u':
        u = 1;
        break;
      case '?':
        printf("Usage: ./hw1 [-t|--tcp] [-u|--udp] [filter-string]\n");
        exit(EXIT_FAILURE);
        break;
      default:
        printf("Something Wrong!");
        exit(EXIT_FAILURE);
    }
  }
  if (t == 1 && u == 0) {
    *mode = TCP_FLAG;
  } else if (t == 0 && u == 1) {
    *mode = UDP_FLAG;
  } else {
    *mode = TCP_FLAG | UDP_FLAG;
  }
  if (argc == optind) {
    memcpy(filter, ".*", 3);
  } else if (argc - optind == 1) {
    snprintf(filter, 256, "%s", argv[optind]);
  } else {
    printf("Usage: ./hw1 [-t|--tcp] [-u|--udp] [filter-string]\n");
    printf("Please input only one filter string\n");
    exit(EXIT_FAILURE);
  }
}

void ReadConnections(void **head, const int mode) {
  int ipv4 = mode & IPV4_FLAG;
  int tcp = mode & TCP_FLAG;
  char file[15], str[200];
  snprintf(file, sizeof(file), "/proc/net/%s%s", tcp ? "tcp" : "udp",
           ipv4 ? "" : "6");
  FILE *fp = fopen(file, "r");
  if (fp == NULL) {
    printf("Fail to open file %s", file);
    return;
  }
  fgets(str, sizeof(str), fp);  // drop header
  int is_first = 1;
  if (ipv4) {
    ContNode4 *cur;
    while (fgets(str, sizeof(str), fp) != NULL) {
      if (is_first) {
        *head = (void *)malloc(sizeof(ContNode4));
        cur = (ContNode4 *)*head;
        cur->next = NULL;
        is_first = 0;
      } else {
        cur->next = (ContNode4 *)malloc(sizeof(ContNode4));
        cur = cur->next;
        cur->next = NULL;
      }
      cur->cont.pid = 0;
      memcpy(cur->cont.program, "<not found>", 12);
      memcpy(cur->cont.proto, (tcp ? "tcp" : "udp"), 4);
      sscanf(str, "%*s%x:%" SCNx16 "%x:%" SCNx16 "%*s%*s%*s%*s%*s%*s%" SCNu32,
             &cur->cont.local_ip.s_addr, &cur->cont.local_port,
             &cur->cont.remote_ip.s_addr, &cur->cont.remote_port,
             &cur->cont.inode);
    }
  } else {
    ContNode6 *cur;
    while (fgets(str, sizeof(str), fp) != NULL) {
      if (is_first) {
        *head = (void *)malloc(sizeof(ContNode6));
        cur = (ContNode6 *)*head;
        cur->next = NULL;
        is_first = 0;
      } else {
        cur->next = (ContNode6 *)malloc(sizeof(ContNode6));
        cur = cur->next;
        cur->next = NULL;
      }
      cur->cont.pid = 0;
      memcpy(cur->cont.program, "<not found>", 12);
      memcpy(cur->cont.proto, (tcp ? "tcp6" : "udp6"), 5);
      sscanf(str,
             "%*s%8x%8x%8x%8x:%" SCNx16 "%8x%8x%8x%8x:%" SCNx16
             "%*s%*s%*s%*s%*s%*s%" SCNu32,
             &cur->cont.local_ip.s6_addr32[0], &cur->cont.local_ip.s6_addr32[1],
             &cur->cont.local_ip.s6_addr32[2], &cur->cont.local_ip.s6_addr32[3],
             &cur->cont.local_port, &cur->cont.remote_ip.s6_addr32[0],
             &cur->cont.remote_ip.s6_addr32[1],
             &cur->cont.remote_ip.s6_addr32[2],
             &cur->cont.remote_ip.s6_addr32[3], &cur->cont.remote_port,
             &cur->cont.inode);
    }
  }
  fclose(fp);
}

void FindPIDAndProgramName(void *head, const int mode) {
  int ipv4 = mode & IPV4_FLAG;
  DIR *pdir = opendir("/proc");
  struct dirent *pdirent;
  regex_t regnum, regsocket1, regsocket2;
  regmatch_t pmatch[1];
  regcomp(&regnum, "^[[:digit:]]+$", REG_EXTENDED);
  regcomp(&regsocket1, "^socket:\\[[[:digit:]]+\\]$", REG_EXTENDED);
  regcomp(&regsocket2, "^\\[0+\\]:[[:digit:]]+$", REG_EXTENDED);
  while ((pdirent = readdir(pdir)) != NULL) {
    if (regexec(&regnum, pdirent->d_name, 1, pmatch, 0) == 0) {
      char fd[265];
      snprintf(fd, sizeof(fd), "/proc/%s/fd", pdirent->d_name);
      DIR *fddir = opendir(fd);
      if (fddir == NULL) {
        continue;
      }
      struct dirent *fddirent;
      while ((fddirent = readdir(fddir)) != NULL) {
        char linkfile[257], link[256];
        if (regexec(&regnum, fddirent->d_name, 1, pmatch, 0) == 0) {
          snprintf(linkfile, sizeof(linkfile), "%s/%s", fd, fddirent->d_name);
          ssize_t end = readlink(linkfile, link, sizeof(link) - 1);
          link[end] = '\0';
          uint32_t inode;
          if (regexec(&regsocket1, link, 1, pmatch, 0) == 0) {
            sscanf(link, "%*[^:]:[%" SCNu32 "]", &inode);
          } else if (regexec(&regsocket2, link, 1, pmatch, 0) == 0) {
            sscanf(link, "%*[^:]:%" SCNu32, &inode);
          } else {
            continue;
          }
          if (ipv4) {
            ContNode4 *cur = (ContNode4 *)head;
            while (cur != NULL) {
              if (cur->cont.inode == inode) {
                sscanf(pdirent->d_name, "%d", &cur->cont.pid);
                char cmd[256], cmdfile[270];
                snprintf(cmdfile, sizeof(cmdfile), "/proc/%s/cmdline",
                         pdirent->d_name);
                FILE *cmdfp = fopen(cmdfile, "r");
                if (cmdfp != NULL) {
                  fgets(cmd, sizeof(cmd), cmdfp);
                  cmd[strlen(cmd) - 1] = '\0';
                  snprintf(cur->cont.program, sizeof(cur->cont.program), "%s",
                           cmd);
                }
                fclose(cmdfp);
              }
              cur = cur->next;
            }
          } else {
            ContNode6 *cur = (ContNode6 *)head;
            while (cur != NULL) {
              if (cur->cont.inode == inode) {
                sscanf(pdirent->d_name, "%d", &cur->cont.pid);
                char cmd[256], cmdfile[270];
                snprintf(cmdfile, sizeof(cmdfile), "/proc/%s/cmdline",
                         pdirent->d_name);
                FILE *cmdfp = fopen(cmdfile, "r");
                if (cmdfp != NULL) {
                  fgets(cmd, sizeof(cmd), cmdfp);
                  cmd[strlen(cmd) - 1] = '\0';
                  snprintf(cur->cont.program, sizeof(cur->cont.program), "%s",
                           cmd);
                }
                fclose(cmdfp);
              }
              cur = cur->next;
            }
          }
        }
      }
      closedir(fddir);
    }
  }
  regfree(&regnum);
  regfree(&regsocket1);
  regfree(&regsocket2);
  closedir(pdir);
}

void ShowHeader(const int mode) {
  printf("List of %s connections:\n", (mode & TCP_FLAG) ? "TCP" : "UDP");
  printf(
      "Proto "               // 6 chars
      "Local Address%22s"    // 13 + 22 chars
      "Foreign Address%20s"  // 15 + 20 chars
      "PID/Program name and arguments\n",
      "", "");
}

void ListConnectionsWithFilter(void *head, const int mode, char *filter) {
  int ipv4 = mode & IPV4_FLAG;
  regex_t reg;
  regmatch_t pmatch[1];
  regcomp(&reg, filter, REG_EXTENDED);
  char local_ip_port[30], local_ip[30];
  char remote_ip_port[30], remote_ip[30];
  if (ipv4) {
    ContNode4 *cur = (ContNode4 *)head;
    while (cur != NULL) {
      inet_ntop(AF_INET, &cur->cont.local_ip, local_ip, sizeof(local_ip));
      inet_ntop(AF_INET, &cur->cont.remote_ip, remote_ip, sizeof(remote_ip));
      snprintf(local_ip_port, sizeof(local_ip_port), "%s:%" PRIu16, local_ip,
               cur->cont.local_port);
      snprintf(remote_ip_port, sizeof(remote_ip_port), "%s:%" PRIu16, remote_ip,
               cur->cont.remote_port);
      if (cur->cont.local_port == 0) {
        local_ip_port[strlen(local_ip_port) - 1] = '*';
      }
      if (cur->cont.remote_port == 0) {
        remote_ip_port[strlen(remote_ip_port) - 1] = '*';
      }
      char result[512];
      snprintf(result, sizeof(result), "%-6s%-35s%-35s%d/%s", cur->cont.proto,
               local_ip_port, remote_ip_port, cur->cont.pid, cur->cont.program);
      if (regexec(&reg, result, 1, pmatch, 0) == 0) {
        printf("%s\n", result);
      }
      cur = cur->next;
    }
  } else {
    ContNode6 *cur = (ContNode6 *)head;
    while (cur != NULL) {
      inet_ntop(AF_INET6, &cur->cont.local_ip, local_ip, sizeof(local_ip));
      inet_ntop(AF_INET6, &cur->cont.remote_ip, remote_ip, sizeof(remote_ip));
      snprintf(local_ip_port, sizeof(local_ip_port), "%s:%" PRIu16, local_ip,
               cur->cont.local_port);
      snprintf(remote_ip_port, sizeof(remote_ip_port), "%s:%" PRIu16, remote_ip,
               cur->cont.remote_port);
      if (cur->cont.local_port == 0) {
        local_ip_port[strlen(local_ip_port) - 1] = '*';
      }
      if (cur->cont.remote_port == 0) {
        remote_ip_port[strlen(remote_ip_port) - 1] = '*';
      }
      char result[512];
      snprintf(result, sizeof(result), "%-6s%-35s%-35s%d/%s", cur->cont.proto,
               local_ip_port, remote_ip_port, cur->cont.pid, cur->cont.program);
      if (regexec(&reg, result, 1, pmatch, 0) == 0) {
        printf("%s\n", result);
      }
      cur = cur->next;
    }
  }
  regfree(&reg);
}

void FreeMemory(void *head, const int mode) {
  int ipv4 = mode & IPV4_FLAG;
  if (ipv4) {
    ContNode4 *cur = (ContNode4 *)head;
    ContNode4 *free_ptr;
    while (cur) {
      free_ptr = cur;
      cur = cur->next;
      free(free_ptr);
    }
  } else {
    ContNode6 *cur = (ContNode6 *)head;
    ContNode6 *free_ptr;
    while (cur) {
      free_ptr = cur;
      cur = cur->next;
      free(free_ptr);
    }
  }
}
