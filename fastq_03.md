# 伸びる配列 - 可変長配列

計算機というものはとにかく柔らかいことや自由なことが苦手で、あらかじめはっきりと定義された文字列長やメモリ容量を確保することは問題ないのですが、
どんな長さの文字列を読み込むか事前には分からない場合や、
何個のデータを保持するかが事前には分からない場合は、開発者が汗を流してそのような仕組みを作らなければなりません。

## 文字の配列(文字列)の場合
例えば、これまではfgets()で1行単位での読み込みを行なってきましたが、バッファサイズは固定長にしていました。
1行が何文字か決まっていないファイル形式のデータを
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

ちなみに以下のブロックはメモリを確保しなおして中の文字列をコピーする操作で、
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
上記の二つは同じような動作をするものの実際にはreallocの方はメモリの後ろの領域に余裕があれば、そのまま使用中の
メモリを拡張して割り当ててくれるので、その場合はメモリのコピーが発生しないので、より効率が良いとされています。

## ポインタの配列の場合

これまでは12本までのリードしか読み込めないプログラムでしたが、以下のようにrealloc()でメモリを確保しなおせば
もっとたくさんのリードを読み込むことができます。下記の例では最初はバッファサイズは2ですが、足りなくなると
自動的に２倍に拡張する仕組みにしています。
```C
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;

int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char buf[256];
    FastQ** list = (FastQ**)malloc(sizeof(FastQ*)*2);
    int capacity = 2;
    int length = 0;
    while(1) {
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            break;
        }
        if(length >= capacity){
            capacity = capacity * 2;
            list = (FastQ**)realloc(list, sizeof(FastQ*)*capacity);
        }

        list[length] = (FastQ*)malloc(sizeof(FastQ));

        strcpy(list[length]->name, buf);
        // seq
        fgets(buf, 256, fp);
        strcpy(list[length]->seq, buf);
        // +
        fgets(buf, 256, fp);
        // qual
        fgets(buf, 256, fp);
        strcpy(list[length]->qual, buf);

        length++;
    }

    for(int i = 0; i<length; i++){
        printf("%s", list[i]->name);
        printf("%s", list[i]->seq);
        printf("+\n");
        printf("%s", list[i]->qual);
    }

    for(int i = 0; i<length; i++){
        free(list[i]);
    }
    free(list);

    return 0;
}
```
今回も前回とのコードの変更点をVisualStudio codeで差分として確認してみましょう。

![img](images/diff_main3_main4.png)
