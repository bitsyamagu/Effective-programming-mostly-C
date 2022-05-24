# 伸びる配列 - 可変長配列

計算機というものはとにかく柔らかいことや自由なことが苦手で、あらかじめはっきりと定義された文字列長やメモリ容量を確保することは問題ないのですが、どんな長さの文字列を読み込むか事前には分からない場合や、
何個のデータを保持するかが事前には分からない場合は、開発者が汗を流してそのような仕組みを作ることになります。

例えば、これまではfgets()で1行単位での読み込みを行なってきましたが、1行が何文字か事前には決められない場合は、以下の例のようにmemchrなどで行末まで読み込めているかどうかを
チェックしながら読み込んでいく必要があります。おまけに読み込んだデータの長さはまちまちで、それをつなげて長い文字列としてメモリ上に保持しておくのであれば

```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_line(FILE* fp){
    int bufsize = 16;
    int capacity = bufsize;
    char* buf = (char*)malloc(bufsize);
    char* cur = buf;
    do {
        char* ret = fgets(cur, bufsize, fp);
        if(ret == NULL){
            break;
        }
        if(memchr(cur, '\0', bufsize) == NULL){ // if incomplete
            char* newbuf = (char*)malloc(capacity+bufsize);
            strncpy(newbuf, buf, capacity);
            free(buf);
            buf = newbuf;
            cur = newbuf + capacity - 1;
            capacity = capacity + bufsize;
        }else {
            return buf;
        }
    }while(1);

    return NULL;
}
int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char* line;
    while(NULL != (line = read_line(fp))){
        printf("%s", line);
        free(line);
    }
    return 0;
}
```
