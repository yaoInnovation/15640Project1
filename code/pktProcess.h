int openPkt(char* buf, int* err);
int writePkt(char* buf, int* err);
int closePkt(char* buf, int* err);
int readPkt(char* buf, int* err, int * payloadSize, char** content);
int unlinkPkt(char* buf, int* err);
int lseekPkt(char* buf, int* err);
int statPkt(char* buf, int* err, int* payloadSize, char** payload);
int __xstatPkt(char* buf, int* err, int* payloadSize, char** payload);
int gendirentriesPkt(char* buf, int* err, int* payloadSize, char** payload);
char* rePktGen(int val, int err);
char* re_PLDPkt_Gen(int val, int err, int size, char* payload);