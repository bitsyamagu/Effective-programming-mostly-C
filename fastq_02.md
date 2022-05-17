# ヒープメモリを使う

ヒープメモリからメモリを確保するためにmalloc()を使用して書き直したコードは以下のようになります。

main2.c
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
    FastQ* list[12];  // (1)
    int index = 0;
    while(1) {
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            break;
        }
        list[index] = (FastQ*)malloc(sizeof(FastQ)); // (2)

        strcpy(list[index]->name, buf); // (3)
        // seq
        fgets(buf, 256, fp);
        strcpy(list[index]->seq, buf);
        // +
        fgets(buf, 256, fp);
        // qual
        fgets(buf, 256, fp);
        strcpy(list[index]->qual, buf);

        index++;
    }

    for(int i = 0; i<12; i++){
        printf("%s", list[i]->name);
        printf("%s", list[i]->seq);
        printf("+\n");
        printf("%s", list[i]->qual);
    }

    for(int i = 0; i<index; i++){  // (4)
        free(list[i]);
    }

    return 0;
}
```
VisualStudio codeを利用して変更点だけを見ると次のような箇所だけです。
![img](images/diff_main_main2.png)

1. FastQ data[12] はポインタの配列 FastQ* list[12]に
1. list[index] = (FastQ\*)malloc(sizeof(FastQ))で1個ずつメモリを確保
1. list[index].nameは、list[index]->nameに
1. free(list[i])で構造体用のメモリを解放

ではこれなら400万本のリードでも大丈夫でしょうか？

実はまだ問題があります。FastQ\* list[4000000]を宣言すると、ポインタは1要素あたり8バイトのメモリを使用するのでポインタの配列だけでも合計32MBがスタック上に必要になってしまいます。この問題も解決するには、ポインタの配列もヒープから取得するしかありません。

main3.c
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
    FastQ** list = (FastQ**)malloc(sizeof(FastQ*)*12); // (1)
    int index = 0;
    while(1){
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            break;
        }
        list[index] = (FastQ*)malloc(sizeof(FastQ));

        strcpy(list[index]->name, buf);
        // seq
        fgets(buf, 256, fp);
        strcpy(list[index]->seq, buf);
        // +
        fgets(buf, 256, fp);
        // qual
        fgets(buf, 256, fp);
        strcpy(list[index]->qual, buf);

        index++;
    }

    for(int i = 0; i<12; i++){
        printf("%s", list[i]->name);
        printf("%s", list[i]->seq);
        printf("+\n");
        printf("%s", list[i]->qual);
    }

    for(int i = 0; i<index; i++){
        free(list[i]);
    }
    free(list);

    return 0;
}
```
これもVisualStudio codeを利用して変更点だけを見ると次のような箇所だけです。
![img](images/diff_main2_main3.png)

FastQ\* list[12]はポインタの配列でしたが、今回はポインタの配列のメモリを確保するので、FastQ\*型へのポインタ、つまりFastQ\*\*をmalloc()で取得することになります。

```C
FastQz** list = (FastQ**)malloc(sizeof(FastQ*)*12);
```
この例ではlistに96バイトのメモリがヒープから割り当てられ、スタックにはFastQ*配列の先頭へのポインタが格納されるので、スタックの消費は8バイトだけになります。あとは、12個を400万個に増やしたとしてもスタックの消費量は8バイトのままです。

さて、これで400万本のリードを処理できそうになりました。malloc(sizeof(FastQ\*)\*4000000)にするだけです。でも、ちょっと考えてみてください。いつも400万個分のスペースを確保するのは無駄すぎるようにも見えます。プログラムのあちこちで400万個分のスペースを確保していたら、ヒープメモリがいくらたくさん使えるといっても、小さいデータを処理する場合には無駄にメモリを使ってしまいます。実行するときにコマンドラインオプションで最大の件数を指定しますか？それは件数を調べるのも面倒です。

できれば最初は小さなメモリから計算を始めて、必要に応じて適度な量のメモリを確保していく方が良いはずです。

このような問題を解決するため、次回は長さを自由に変えられる配列、つまり可変長配列についてご紹介します。

----

## おまけ

ヒープやスタックというメモリの種類は、C言語やJavaではよく聞かれますが、PythonやPerlではあまり聞かれないかもしれません。
ヒープメモリやスタックメモリを管理しているのはOSなので、malloc()などでメモリを確保するということは
OSからメモリの領域を払い出してもらうことを意味しますし、free()はそれをOSに返すことを意味しています。

ヒープは日本人にはあまり聞きなれない単語かもしれませんが、イメージとしては乱雑に積み上げられた本とか
衣類のようなイメージで、スタックの方は揃えて積み上げられた本のようなイメージの単語です。

スタックは前にも書きましたように関数への出入りの際に積み下ろしされ、自動的にアロケーションと
解放が行われますが、ヒープについては自分達でアロケーションと解放をお世話しなければならないという
違いがあります。

C/C++が難しくてPythonやJavaが簡単と言われるのは、このメモリの確保や解放をPythonやJavaは
プログラミング言語の処理系や仮想マシンがやってくれるのに対し、C/C++ではプログラマが
明示的にそれらの処理を記述しなければいけなくて、そしてそれがしばしば失敗して
Windowsでは一般保護違反を起こしたり、UNIX系OSではSegmentation faultを起こすことも
理由の一つに挙げられます。

以下のダングリングポインタとメモリリークの二種類の問題は、ポインタを扱う上で避けるべき典型的なものです。

### dangling pointer: 宙ぶらりんのポインタ、ダングリングポインタ

*自動変数へのポインタ*
```C
char* func(){
    char str[10];
    strcpy(str, "hello!");
    return str; // ここで返すポインタは関数から抜けた瞬間に、ポインタの先のメモリが解放されてしまう
}
```
これは既に所有権を失ったスタック上のメモリへのアクセス違反を起こします。

解放したメモリへのアクセス
```C
int* c = malloc(sizeof(int));
free(c);
*c  = 3; // 解放したメモリへの書き込み！
```
こちらは既に所有権を失ったヒープ上のメモリへのアクセス違反を起こします。

### メモリーリーク

free()忘れ
```C
void func(){
    char* ch = malloc(10);
}
```
この関数は呼び出されるたびに10バイトずつ見失っていきます。積もり積もると使用メモリが増大して、計算機のメモリを圧迫します。

ダングリングポインタを避ける手っ取り早い方法は、解放直後にNULLをポインタに
入れておくことです。それによりポインタはダングリングではない状態になります。
でも、そのポインタを使って何か計算しようとすると、NULLが入っているので、
いわゆるJavaのNullPointerExceptionのような状況になり、エラーになります。
そのような問題はポインタの寿命についての設計上の失敗なので、ポインタの
寿命やスコープ、所有権について見直す必要があります。

メモリーリークが起きやすいのは、一つには長すぎるポインタの寿命がありますが、
典型的な失敗例としては、関数の途中抜けがあります。

```C
void func(int x){
   char* ch = malloc(10);
   if(x==2){
       return;
   }
   // do something
   free(ch);
}
```
最初からこのようなコードを書くのは間抜けですが、仕様変更等でif文が
挿入された場合などに起きやすいのです。

dangling pointerやメモリリークについては、最近はclangやGCCなどはコンパイラが警告を出してくれる機能もあるので、
そのようなものも役立つでしょう。

また、ダングリングポインタは危険ですが、メモリリークは危険ではない場合もあります。
例えば数分以内に終わる計算などです。最近のLinuxなどのOSはメモリ管理がしっかりしているので、
プログラムが終了時にメモリを返却していなくても全部しっかり回収してくれますから、
あえてメモリを解放しないというスタイルの人もいるようです。

メモリリークに対してシビアなのは、長時間動作するサーバやデーモンなどのプログラムや
再利用によってさまざまな目的に利用されるライブラリのプログラム、
少ないメモリで動作しなければならない組み込み系やGPUプログラミングなどでしょうか。
