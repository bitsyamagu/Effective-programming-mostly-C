これまで見てきたFastQファイルはリード長は最大255塩基としてきました。
一方、実際のshort readのシーケンスは150塩基以下のデータが多いので、
約40%ほどメモリを無駄に消費していることになります。またリード名は
それよりも短いことも多いので、50%程度は無駄かも知れません。

この無駄を無くすには以下のようなFastQ構造体を
```C
typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;
```
次のように修正して、任意の長さの文字列へのポインタとして、
個別にメモリをアロケーションしてやるのが最も効率的です。
```C
typedef struct {
    char* name;
    char* seq;
    char* qual;
} FastQ;
```

main5.c次のように修正してmain6.cを作成します。上のようなFastQ構造体の修正も
合わせて行って下さい。

![img](images/diff_main5_main6.png)

変更箇所の要点は以下の通りです。
- 構造体のメンバーを文字配列から文字列のポインタに変更
- FastQの生成がやや複雑になるのでcreate_FastQ関数に処理を移動
- create_FastQ関数では以下の処理を行います
  - 文字列の長さを調べ
  - 必要量のメモリを割り当て
  - 一時バッファから割り当てたメモリにデータをコピー、ただし改行文字は除く
  - 行末の改行文字のあった位置にヌル文字を入れる
- 出力時に改行文字の出力を追加

main6.c
```C
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
    char* name;
    char* seq;
    char* qual;
} FastQ;

#define INITIAL_FQ_ARRAY_CAPACITY 2
typedef struct {
    FastQ** buf;
    int capacity;
    int length;
} FastQArray;

void FastQArray_init(FastQArray* p){
    p->buf = (FastQ**)malloc(sizeof(FastQ*)*INITIAL_FQ_ARRAY_CAPACITY);
    p->capacity = INITIAL_FQ_ARRAY_CAPACITY;
    p->length = 0;
}
void FastQArray_ensure(FastQArray* p, int index){
    if(index >= p->capacity){
        int new_capacity = p->capacity * 2;
        p->buf = (FastQ**)realloc(p->buf, sizeof(FastQ*)*new_capacity);
        fprintf(stderr, "old: %d, new %d\n", p->capacity, new_capacity);
        p->capacity = new_capacity;
    }
}
void FastQArray_push_back(FastQArray* p, FastQ* fq){
    FastQArray_ensure(p, p->length+1);
    p->buf[p->length] = fq;
    p->length++;
}
void FastQArray_finish(FastQArray* p){
    for(int i = 0; i<p->length; i++){
        free(p->buf[p->length]);
    }
    free(p->buf);
}
FastQ* create_FastQ(char* name, char* seq, char* qual){
    FastQ* p = (FastQ*)malloc(sizeof(FastQ));
    int name_len = strlen(name);
    int seq_len = strlen(seq);
    int qual_len = strlen(qual);
    p->name = (char*) malloc(sizeof(char)*name_len);
    p->seq = (char*) malloc(sizeof(char)*seq_len);
    p->qual = (char*) malloc(sizeof(char)*qual_len);
    strncpy(p->name, name, name_len-1);
    strncpy(p->seq, seq, seq_len-1);
    strncpy(p->qual, qual, qual_len-1);
    p->name[name_len] = '\0';
    p->seq[seq_len] = '\0';
    p->qual[qual_len] = '\0';

    return p;
}

int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char buf_name[256];
    char buf_seq[256];
    char buf_qual[256];
    FastQArray array;
    FastQArray_init(&array);

    while(1) {
        // name
        char* p = fgets(buf_name, 256, fp);
        if(p == NULL){
            break;
        }
        fgets(buf_seq, 256, fp);
        fgets(buf_qual, 256, fp); // dummy
        fgets(buf_qual, 256, fp);
        FastQ* fq = create_FastQ(buf_name, buf_seq, buf_qual);
        FastQArray_push_back(&array, fq);
    }

    for(int i = 0; i<array.length; i++){
        printf("%s\n", array.buf[i]->name);
        printf("%s\n", array.buf[i]->seq);
        printf("+\n");
        printf("%s\n", array.buf[i]->qual);
    }
    FastQArray_finish(&array);

    return 0;
}
```

これで一時バッファを除けば、文字の配列はほとんどなくなって、
データのほとんどがヒープ上のポインタとして保持するように
なりました。

このようなメモリ上でのデータの持ち方は、PythonやJavaでは
最も基本的なものです。

上記のmain6.cと同等のコードをPythonとJavaで書くと以下のようになります。

Pyton版 main6.py
```python
import os

class FastQ:
    def __init__(this, name, seq, qual):
        this.name = name
        this.seq = seq
        this.qual = qual

list = []

with open("test.fastq", "r") as fp:
    while True:
        name = fp.readline().rstrip(os.linesep)
        if name == "":
            break
        seq = fp.readline().rstrip(os.linesep)
        dummy = fp.readline().rstrip(os.linesep)
        qual = fp.readline().rstrip(os.linesep)
        list.append(FastQ(name, seq, qual))

for x in list:
    print(x.name)
    print(x.seq)
    print("+")
    print(x.qual)
```

Java版 Main6.java
```Java
import java.util.ArrayList;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class Main6 {
    static public class FastQ {
        String name;
        String seq;
        String qual;
        public FastQ(String n, String s, String q){
            name = n;
            seq = s;
            qual = q;
        }
    }
    public static void main(String[] argv){
        ArrayList<FastQ> list = new ArrayList<>();
        try  {
            BufferedReader br = new BufferedReader(new FileReader(argv[0]));
            while(true){
                String name = br.readLine();
                String seq = br.readLine();
                br.readLine();
                String qual = br.readLine();
                if(name == null){
                    break;
                }
                FastQ fq = new FastQ(name, seq, qual);
                list.add(fq);
            }
            for(FastQ fq: list){
                System.out.println(fq.name + "\n" + fq.seq + "\n+\n" + fq.qual + "\n");
            }
        }catch(Exception e){
            e.printStackTrace();
        }
    }
}
```
