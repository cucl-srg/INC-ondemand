/* Copyright (c) 2019, Yuta Tokusashi
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
* * Neither the name of the project, the copyright holder nor the names of its
*  contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memcached/protocol_binary.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#ifdef __FreeBSD__
#include <sys/endian.h>
#else // Linux
#include <endian.h>
#endif/* __FreeBSD__ */
#include <string>
#ifndef GENERATOR_H
#include "Generator.h"
#endif // GENERATOR_H
#include "util.h"
#include "Connection.h"
#include "AdaptiveSampler.h"
#include "ConnectionStats.h"
#include "LogHistogramSampler.h"
#include "mutilate.h"

#define KEYVAL_NUM 1000000
#define BUF_SIZE 2048
#define OP_GET 1
#define OP_SET 2
#define LOADING_NUM 1024
#define MAX_NUM 1024*32
#define SIZE_LIMIT 1500

#define CPU_MHZ_FILE        "/proc/cpuinfo"
#define CPU_MHZ_PREFIX        "cpu MHz\t\t: "

#define MHZ 3301
#define TIME_UTC 1

// Debug MACRO
#define dbg_printf() printf("%d at %s in %s\n", \
	__LINE__, __func__, __FILE__)
#define MUTILATE_TEST

int targ = 0;
// Temporal Text: from 
//    GNU Free Documentation License Version 1.3, 3 November 2008
const char tmp_char[] = "This License applies to any manual or other \
work, in any medium, that contains a notice placed by the copyright \
holder saying it can be distributed under the terms of this License. \
Such a notice grants a world-wide, royalty-free license, unlimited in \
duration, to use that work under the conditions stated herein. The \
Document, below, refers to any such manual or work. Any member of the \
public is a licensee, and is addressed as you. You accept the license \
if you copy, modify or distribute the work in a way requiring permission \
under copyright law. A Modified Version of the Document means any work \
containing the Document or a portion of it, either copied verbatim, or \
with modifications and/or translated into another language. A Secondary \
Section is a named appendix or a front-matter section of the Document \
that deals exclusively with the relationship of the publishers or \
authors of the Document to the Document's overall subject (or to related \
matters) and contains nothing that could fall directly within that \
overall subject. (Thus, if the Document is in part a textbook of \
mathematics, a Secondary Section may not explain any mathematics.) The \
relationship could be a matter of historical connection with the subject \
or with related matters, or of legal, commercial, philosophical, ethical \
or political position regarding them.\
\
The Invariant Sections are certain Secondary Sections whose titles are \
designated, as being those of Invariant Sections, in the notice that says \
that the Document is released under this License. If a section does not \
fit the above definition of Secondary then it is not allowed to be \
designated as Invariant. The Document may contain zero Invariant Sections. \
If the Document does not identify any Invariant Sections then there are \
none. The Cover Texts are certain short passages of text that are listed, \
as Front-Cover Texts or Back-Cover Texts, in the notice that says that \
the Document is released under this License. A Front-Cover Text may be at \
most 5 words, and a Back-Cover Text may be at most 25 words.\
A Transparent copy of the Document means a machine-readable copy, \
represented in a format whose specification is available to the general \
public, that is suitable for revising the document straightforwardly \
with generic text editors or (for images composed of pixels) generic \
paint programs or (for drawings) some widely available drawing editor, \
and that is suitable for input to text formatters or for automatic \
translation to a variety of formats suitable for input to text formatters. \
A copy made in an otherwise Transparent file format whose markup, or \
absence of markup, has been arranged to thwart or discourage subsequent \
modification by readers is not Transparent. An image format is not \
Transparent if used for any substantial amount of text. A copy that is \
not Transparent is called Opaque.";

//#ifdef __cplusplus
extern "C" struct Generator* c_createGenerator(char *str)
{
	return createGenerator(str);
}

extern "C" struct Generator* c_createFacebookKey(void)
{
	return createFacebookKey();
}
extern "C" struct Generator* c_createFacebookValue(void)
{
	return createFacebookValue();
}
extern "C" struct Generator* c_createFacebookIA(void)
{
	return createFacebookIA();
}
//#endif /* __cplusplus */
struct gsettings {
	int sock;
	long long int send_pkts;
	uint8_t verbose;
	uint8_t oneshot;
	int oneshot_keylen;
	int oneshot_vallen;
	char *oneshot_key;
	char *oneshot_val;
	int wait_time;
	char *hostname;
	int uport;
	float dist;
	char *key_mode;
	char *val_mode;
	char *ia_mode;
	int ia_true;
	double delay;
	int records;
	struct sockaddr_in addr;
	float update;
	int load_max;
	double freq;
};

struct shared_mem {
	long long int recv_queries;
	struct timespec start_time;
	struct timespec finish_time;
	struct timespec epoc_time[1000000000];
	unsigned char ftime_valid[1000000000];
	unsigned long long int btime[1000000000];
	unsigned long long int ftime[1000000000];
};

double *delay_arr;

struct set_pkt_t {
	protocol_binary_request_header pb;
	uint32_t extflag;
	uint32_t extexpr;
	char byte[2048];
	char *key;
	char *val;
}__attribute__((__packed__));

struct get_pkt_t {
	protocol_binary_request_header pb;
	char byte[2048];
	char *key;
}__attribute__((__packed__));

struct workload {
	uint8_t op;
	uint32_t keylen;
	uint32_t vallen;
	char byte[2048];
	char *p;
};


/*
 * Functions
 */
static void usage(void)
{
	printf("workload-gen: \n");
	printf("\t-n <num>\t: number of queries\n"); 
	printf("\t-m <dist>\t: distribution of SET:GET\n");
	printf("\t-o\t:Oneshot mode\n");
	printf("\t-u <ratio>\t: SET:GET\n");
	printf("\t-K <Key>\t: Key string\n");
	printf("\t-V <Key>\t: Value string\n");
	printf("\t-k <key dist>\t: mode <fb_key|pareto,,,>\n");
	printf("\t-l <value dist>\t: mode <fb_value|pareto,,,>\n");
	printf("\t-v\t: verbose mode\n");
	printf("---------\n");
	printf("Oneshot mode\n");
	printf("\t$ ./workload-gen -n <num> -s <server address>\n");
	printf("\t\t-p <port num> -o -K abcd -V 123456789\n");
	printf("Facebook Trace mode\n");
	printf("\t$ ./workload-gen -n <num> -s <server address>\n");
	printf("\t\t-p <port num> -k fb_key -l fb_value\n");
	exit(EXIT_SUCCESS);
}

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

unsigned int get_mhz(void)
{
    FILE *fp;
    char line[1024];
    unsigned int clock_mhz = 0;

    fp = fopen("/proc/cpuinfo", "r");
    if (!fp)
        return 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, CPU_MHZ_PREFIX, sizeof(CPU_MHZ_PREFIX)-1) == 0)
        {
            clock_mhz = atoi(line+sizeof(CPU_MHZ_PREFIX)-1);
            break;
        }
    }
    fclose(fp);
    return clock_mhz;
}


void RecvCount(int sock, struct sockaddr *from, struct shared_mem *shm)
{	
	int n;
	long long int cnt = 0;
	shm->start_time.tv_sec = 0;
	shm->start_time.tv_nsec = 0;
	socklen_t fromlen = sizeof(from);
	char buf[2048];
	protocol_binary_response_header *pb;

	memset(buf, 0, sizeof(buf));
	while (1){
		if ((n = recvfrom(sock, buf, sizeof(buf), 0, from, &fromlen)) < 0) {
			printf("Error: recvfrom\n");
		} else {
			pb = (protocol_binary_response_header *)&buf;
			int index = (int)pb->response.opaque;
			shm->ftime_valid[index] = 1;
			shm->ftime[index] = rdtsc();
			if (cnt == 0) 
				clock_gettime(CLOCK_MONOTONIC_RAW, &shm->start_time);
			else 
				clock_gettime(CLOCK_MONOTONIC_RAW, &shm->finish_time);
			cnt++;
			shm->recv_queries = cnt;
		}
	}
}

void SigHandler(int p_signame)
{
	targ = 1;
	return;
}

void SetSignal(int p_signame)
{
	if (signal(p_signame, SigHandler) == SIG_ERR) {
		printf("Error: Signal cannot be set.\n");
		exit(-1);
	}

  return;
}

void ShowConf(struct gsettings *g)
{
	printf("=================================================\n");
	printf("\tInjecting Queries     : %lld\n", g->send_pkts);
	printf("\tWaiting Time          : %d\n", g->wait_time);
	printf("\tServer Address        : %s\n", g->hostname);
	printf("\tUDP Port Number       : %d\n", g->uport);
	if (g->oneshot) {
		printf("\tInjected Key Length   : %d\n", g->oneshot_keylen);
		printf("\tInjected Value Length : %d\n", g->oneshot_vallen);
		printf("\tInjected Key          : %s\n", g->oneshot_key);
		printf("\tInjected Value        : %s\n", g->oneshot_val);
	}
	printf("=================================================\n");
}

//void ShowResult(struct gsettings *g, long long int *recv_queries)
void ShowResult(struct gsettings *g, struct shared_mem *shm)
{
	long long int dt = ((shm->finish_time.tv_sec * 1000000) + 
		(shm->finish_time.tv_nsec / 1000)) - 
		((shm->start_time.tv_sec * 1000000) + 
		(shm->start_time.tv_nsec / 1000));
	printf("=================================================\n");
	printf("\tInjecting Queries : %lld\n", g->send_pkts + 1);
	printf("\tRecieved  Queries : %lld\n", shm->recv_queries);
	printf("\tStart Time        : %ld sec %ld nsec\n", 
		shm->start_time.tv_sec, shm->start_time.tv_nsec);
	printf("\tFinish Time       : %ld sec %ld nsec\n", 
		shm->finish_time.tv_sec, shm->finish_time.tv_nsec);
	printf("\tDelta Time        : %lld usec\n", dt);
	printf("\tThroughput        : %f Queries per second\n",
		(float)((float)shm->recv_queries / (float)dt * 1000000));
	printf("=================================================\n");
	for (int i = 0; i < g->send_pkts; i++) {
		if (shm->ftime_valid[i] == 1) {
			printf("[%d] %ld.%09ld %llu %f\n", i, shm->epoc_time[i].tv_sec, shm->epoc_time[i].tv_nsec, shm->ftime[i] - shm->btime[i], (float)(shm->ftime[i] - shm->btime[i])*g->freq);
		}
	}
}

inline void WriteSETHeader(struct set_pkt_t *pkt, int keylen, int vallen, int index)
{
	pkt->pb.request.magic    = PROTOCOL_BINARY_REQ;
	pkt->pb.request.opcode   = PROTOCOL_BINARY_CMD_SET;
	pkt->pb.request.keylen   = htons(keylen); 
	pkt->pb.request.extlen   = 8;
	pkt->pb.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
	pkt->pb.request.reserved = 0;
	pkt->pb.request.bodylen  = htonl(keylen + vallen + 8); 
	pkt->pb.request.opaque   = index;
	pkt->pb.request.cas      = 0;
	// Extra Field
	pkt->extflag = 0;
	pkt->extexpr = htonl(1200);
	// Pointer Settings
	pkt->key = pkt->byte;
	pkt->val = pkt->key + keylen;
}

inline void WriteGETHeader(struct get_pkt_t *pkt, int keylen, int index)
{
	pkt->pb.request.magic    = PROTOCOL_BINARY_REQ;
	pkt->pb.request.opcode   = PROTOCOL_BINARY_CMD_GET;
	pkt->pb.request.keylen   = htons(keylen); 
	pkt->pb.request.extlen   = 0;
	pkt->pb.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
	pkt->pb.request.reserved = 0;
	pkt->pb.request.bodylen  = htonl(keylen); 
	pkt->pb.request.opaque   = index;
	pkt->pb.request.cas      = 0;
	pkt->key = pkt->byte;
}

inline void WriteDELHeader(struct get_pkt_t *pkt, int keylen, int index)
{
	pkt->pb.request.magic    = PROTOCOL_BINARY_REQ;
	pkt->pb.request.opcode   = PROTOCOL_BINARY_CMD_DELETE;
	pkt->pb.request.keylen   = htons(keylen); 
	pkt->pb.request.extlen   = 0;
	pkt->pb.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
	pkt->pb.request.reserved = 0;
	pkt->pb.request.bodylen  = htonl(keylen); 
	pkt->pb.request.opaque   = index;
	pkt->pb.request.cas      = 0;
	pkt->key = pkt->byte;
}

void start_loading(struct gsettings *g, struct workload *w) 
{
	int i;
	int loader_issued = 0;
	int keygen_seed   = 0;
	//int loader_completed = 0;
	struct Generator *valuesize = createGenerator(g->val_mode);
	struct Generator *keysize = createGenerator(g->key_mode);
	struct KeyGenerator *keygen = new KeyGenerator(keysize, g->send_pkts);
	if (g->ia_true) {
		delay_arr = (double *)malloc(sizeof(double)*g->send_pkts);
		if (delay_arr == NULL) {
			printf("Error: malloc");
			exit(-1);
		}
		struct Generator *ia = createGenerator(g->ia_mode);
		for(int l =0; l < g->send_pkts; l++) {
			delay_arr[l] = ia->generate();
		}
	} 
	string keystr;

	for (i = 0; i < g->load_max; i++) {
		if (i < g->records) {
				w[i].op = OP_SET;
				// Inserting Xus for SET request for initializing
				usleep(500);
		} else {
			w[i].op = OP_GET;
		}
		char key[1024];
		if (i < g->records)
			keystr = keygen->generate(loader_issued);
		else  {
			keystr = keygen->generate(lrand48() % g->records);
		}
    	strcpy(key, keystr.c_str());
		w[i].keylen = strlen(key);
		w[i].vallen = (uint32_t)valuesize->generate();
		while (w[i].vallen == 0 || w[i].vallen > SIZE_LIMIT) {
			w[i].vallen = (uint32_t)valuesize->generate();
		}
		w[i].p = w[i].byte;
		strncpy(w[i].p, keystr.c_str(), w[i].keylen);
		strncpy(w[i].p + w[i].keylen, tmp_char, w[i].vallen);

		if (i == g->records)
			loader_issued = 0;
		else
			loader_issued++;
	}
	if (g->verbose) {
		printf("============================================\n");
		for (i = 0; i < g->load_max; i++) 
			printf("[%d] %s keylen %u, vallen %u %s\n", 
				i, (w[i].op == OP_SET) ? "SET" : "GET", 
				w[i].keylen, w[i].vallen, w[i].byte);
	}

	return;
}

void GeneratePkt(struct gsettings *g, struct workload *w, struct shared_mem *shm) 
{
	int i;
	struct set_pkt_t pkt; 
	struct get_pkt_t gpkt;
	int nsnd = 0;
	int load_cnt = 0;
	struct Generator *ia = createGenerator(g->ia_mode);

	for (i = 0; i < g->send_pkts && !targ; i++) {
		if (load_cnt >= MAX_NUM) load_cnt = 0;
		if (w[load_cnt].op == OP_SET) {
			WriteSETHeader(&pkt, w[load_cnt].keylen, w[load_cnt].vallen, i);
			strncpy(pkt.key, w[load_cnt].byte, 
				w[load_cnt].keylen + w[load_cnt].vallen);
			do {
				timespec_get(&shm->epoc_time[i], TIME_UTC);
				shm->btime[i] = rdtsc();
				nsnd = sendto(g->sock, &pkt, 
					32 + w[load_cnt].keylen + w[load_cnt].vallen, 0, 
					(struct sockaddr *)&g->addr, sizeof(g->addr));
				usleep(100);
			} while (nsnd < 0 && (errno == EAGAIN || errno == ENOTSOCK));
		} else if (w[load_cnt].op == OP_GET) {
			WriteGETHeader(&gpkt, w[load_cnt].keylen, i);
			strncpy(gpkt.key, w[load_cnt].byte, w[load_cnt].keylen);
			do {
				timespec_get(&shm->epoc_time[i], TIME_UTC);
				shm->btime[i] = rdtsc();
				nsnd = sendto(g->sock, &gpkt, 24 + w[load_cnt].keylen, 
					0, (struct sockaddr *)&g->addr, sizeof(g->addr));
				usleep((int)delay_arr[i]);
			} while (nsnd < 0 && (errno == EAGAIN || errno == ENOTSOCK));
		}
		if (nsnd < 0) {
			printf("Error: [%d] sento %d Byte %d\n", i, errno, nsnd);
		}
		load_cnt++;
	}
	return;
}

int main(int argc, char **argv)
{
	struct gsettings g;
	g.send_pkts = 10;
	g.verbose = 0;
	g.oneshot = 0;
	g.oneshot_keylen = 0;
	g.oneshot_vallen = 0;
	g.oneshot_key = NULL;
	g.oneshot_val = NULL;
	g.wait_time = 5;
	g.hostname = NULL;
	g.uport = 11211;
	g.dist = 0.0;
	g.update = 0.0;
	g.records = LOADING_NUM;
	g.load_max = 0;
	g.ia_true = 0;
	g.freq = 1000000000.0 / (MHZ * 1000000.0);

	struct workload *w;
	int c, i;

	char req_key_str[2048] = "netfpga";
	int req_key_len = 7;
	char req_val_str[2048] = "NetFPGA-SUME";
	int req_val_len = 12;
	int index = 0;
	struct set_pkt_t pkt; 
	struct get_pkt_t gpkt;
	struct get_pkt_t dpkt;
	
	if(argc == 1) usage();
	while((c = getopt(argc, argv, "i:n:t:m:s:p:hol:k:K:r:u:V:w:v")) != -1) {
		switch(c) {
			case 'n':
				g.send_pkts = atoi(optarg);
				break;
			case 'm':
				g.dist = atof(optarg);
				break;
			case 'h':
				usage();
				break;
			case 's':
				g.hostname = optarg;
				break;
			case 'p':
				g.uport = atoi(optarg);
				break;
			case 'o':
				g.oneshot = 1;
				break;
			case 'i':
				g.ia_mode = optarg;
				if (!strcmp(g.ia_mode, "fb_ia"))
					g.ia_true = 1;
				break;
			case 'k':
				g.key_mode = optarg;
				break;
			case 'l':
				g.val_mode = optarg;
				break;
			case 'K':
				g.oneshot_key = optarg;
				g.oneshot_keylen = strlen(optarg);
				break;
			case 'r':
				g.records = atoi(optarg);
				break;
			case 'u':
				g.update = atof(optarg);
				break;
			case 'V':
				g.oneshot_val = optarg;
				g.oneshot_vallen = strlen(optarg);
				break;
			case 'v':
				g.verbose = 1;
				break;
			case 'w':
				g.wait_time = atoi(optarg);
				break;
			case '?':
				fprintf(stderr, "illegal option: %c", c);
				exit(EXIT_FAILURE);
				break;
			default :
				fprintf(stderr, "illegal option: %c", c);
				exit(EXIT_FAILURE);
				break;
		}
	}

	unsigned int mhz = get_mhz();
	g.freq = 1000000000.0 / (mhz * 1000000.0);
	//g.freq = 
	
	// Signal Handler
	SetSignal(SIGINT);
	// Shared Memory
	struct shared_mem *shm;
	shm = (struct shared_mem *)mmap(NULL, sizeof(*shm), 
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS , -1, 0);

	for (int d = 0; d < g.send_pkts; d++) {
		shm->ftime_valid[d] = 0;
	}
	/* Print Settings */
	if (g.hostname == NULL) {
		printf("Error: please specify hostname with -s option.");
		exit(-1);
	}
	
	if (g.send_pkts > MAX_NUM) {
		g.load_max = MAX_NUM;
	} else {
		g.load_max = (int)g.send_pkts;
	}
	w = (struct workload *)malloc(sizeof(struct workload) * g.load_max);
	if (w == NULL) {
		printf("Error: malloc\n");
		exit(-1);
	}
	/* UDP Socket */
	if ((g.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Error: socket\n");
	}
	g.addr.sin_family = AF_INET;
	g.addr.sin_port = htons(g.uport);
	g.addr.sin_addr.s_addr = inet_addr(g.hostname);
	if (g.oneshot) {
		if (g.oneshot_key != NULL) {
			strcpy(req_key_str, g.oneshot_key);
			req_key_len = strlen(g.oneshot_key);
		} 
		if (g.oneshot_val != NULL) {
			strcpy(req_val_str, g.oneshot_val);
			req_val_len = strlen(g.oneshot_val);
		}
	}
	if (g.verbose) ShowConf(&g);

	// replacing Setup() !?
	if (g.oneshot == 1) {
		WriteSETHeader(&pkt, req_key_len, req_val_len, 0);
		strncpy(pkt.key, req_key_str, req_key_len);
		strncpy(pkt.val, req_val_str, req_val_len);
	
		// GET Request
		WriteGETHeader(&gpkt, req_key_len, index);
		strncpy(gpkt.key, req_key_str, req_key_len);
	} else {
		if (g.val_mode == NULL || g.key_mode == NULL)
			printf("No loading\n");
		else {
			start_loading(&g, w);
		}
	}

	// Fork
	pid_t pid;
	pid = fork();
	if (pid == -1) {
		printf("Error: fork\n");
	} else if (pid == 0) {
		RecvCount(g.sock, (struct sockaddr *)&g.addr, shm);
	}

	if (g.oneshot == 1) {
		sendto(g.sock, &pkt, 32 + req_key_len + req_val_len, 
			0, (struct sockaddr *)&g.addr, sizeof(g.addr));
		sleep(10);
		for (i = 0; i < g.send_pkts; i++)
			WriteGETHeader(&gpkt, req_key_len, index);
			sendto(g.sock, &gpkt, 24 + req_key_len, 0, 
				(struct sockaddr *)&g.addr, sizeof(g.addr));
			index++;
	} else {
		GeneratePkt(&g, w, shm);
	}

	sleep(g.wait_time);
	kill(pid, SIGKILL);

	/* Print Report */
	ShowResult(&g, shm);

	free(w);
	munmap(shm, sizeof(*shm));
	close(g.sock);

	return 0;
}

