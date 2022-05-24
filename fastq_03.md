# 伸びる配列 - 可変長配列

計算機というものはとにかく柔らかいことや自由なことが苦手で、あらかじめはっきりと定義された文字列長やメモリ容量を確保することは問題ないのですが、
どんな長さの文字列を読み込むか事前には分からない場合や、
何個のデータを保持するかが事前には分からない場合は、開発者が汗を流してそのような仕組みを作らなければなりません。

例えば、これまではfgets()で1行単位での読み込みを行なってきましたが、1行が何文字か決まっていないファイル形式のデータを
読み込む場合には次のコードのように継ぎ足し継ぎ足し読み込んでいくことになります。
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
            cur = newbuf + capacity - 1; // 次の読み込み開始点のアドレス
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
PerlやPythonでは１行ずつ読み込む処理は簡単に書くことができますが、実際には裏ではこのような処理が走っているので、
１行が長い場合には度々メモリのアロケーションや文字列のコピーが発生して効率が悪くなるだろうということは
想像できると思います。C/C++やJavaなどの言語ではバッファサイズは自分で自由に決められるので、あらかじめ
無駄なアロケーションが頻発しない程度のバッファサイズにしておけば、数TBなどの巨大なデータでも
効率的に処理できます。

ちなみに以下のブロックはメモリを確保しなおして中の文字列をコピーする操作ですが、
これはreallocという関数でもっと簡単に書き換えることができます。
```C
    char* newbuf = (char*)malloc(capacity+bufsize);
    strncpy(newbuf, buf, capacity);
    free(buf);
    buf = newbuf;
```
```C
    buf = (char*)realloc(capacity+bufsize);
```
上記の二つは同じような動作をしますが、実際にはreallocの方はメモリの後ろの領域に余裕があれば、そのまま使用中の
メモリを拡張して割り当ててくれるので、その場合はメモリのコピーが発生しないので、より効率が良いとされています。
