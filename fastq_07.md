# イルミナのリード名によるソート

これまでに見てきたリード名「@SRR8206481.1 1/1」はSRAから入手したデータのリード名の
形式でした。

このようなリード名であれば1/1の分子の数字を抽出して、その順にソートしてやればよかったのですが、
イルミナのリード名は以下のような形式です。
```
@SIM:1:FCX:1:15:6329:1045:GATTACT+GTCTTAAC 1:N:0:ATCCGA
```
> https://support.illumina.com/help/BaseSpace_Sequence_Hub/Source/Informatics/BS/FileFormat_FASTQ-files_swBS.htm より

<table>
<th>値<th> 意味 </th> 
  <tr><td> sim<td> instrument ID</td>
<tr><td>   1<td> run number</td>
<tr><td> fcx<td> flow cell ID</td>
<tr><td>   1<td> lane number</td>
<tr><td>  15<td> tile number</td>
<tr><td>6329<td> x coordinate of cluster</td>
<tr><td>1045<td> y coordinate of cluster</td>
</table>

一般的なWhole exomeのデータの場合は1回のシーケンシングで、単一レーンで
済むようなデータが多いのでInstrument ID, Run number, Flow cell ID, Lane numberまでは
共通になるので、その後ろだけを見て並び替えれば良いことになります。実際の解析では
Optical dupilcationの除去は同一タイル内で、X,Y座標が近いリード同士で
チェックすることになるはずなので、タイル番号と座標で並び替えておけば十分です。

イルミナ用のFastQ構造体は以下のようになります。
```C
typedef struct {
    char* name;
    char* seq;
    char* qual;
    uint16_t tile;
    uint16_t xpos;
    uint16_t ypos;
} IlluminaFastQ;
```
FastQ構造体も名前を変更してSRAFastQにしましょう。
```C
typedef struct {
    char* name;
    char* seq;
    char* qual;
    uint32_t index;
} SRAFastQ;
```
イルミナのリードの比較の優先順位としては <strong>tile number &gt; x coordinate &gt; y coordinate</strong>
ということになります。
> **Note**
> レーンやタイルについて、より具体的に知りたい方はこのような資料を探してお読みください。
> https://www.biostars.org/p/9461090/

SRAに加えてIlluminaと、扱うデータの種類は増えますが、可変長配列とソートのアルゴリズムは再利用して、
SRAのソートとイルミナのソートを簡単に切り替えられるようにしようと思います。

まず、ソートもバブルソートの名前を付けて別の関数bubble_sort()に分離し、比較関数も切り替えやすいように
分離することにします。リード名の比較はバブルソートのアルゴリズム内で行なっているわけですが
そこにイルミナのリード名の比較機能を直接追加するとした場合、疑似コードで書くと以下のような
コードになるでしょうか。

```C
enum read_type {
    SRA, ILLUMINA
};
void bubble_sort(FastQArray* array, enum read_type rtype /*illumina? or SRA?*/){
    int swapped = 1;
    while(swapped){
        swapped = 0;
        for(int i = 0; i<array->length-1; i++){
            int c = 0;
            if(rtype == SRA){
               c = SRAのリード名比較関数();
            }else {
               c = イルミナのリード名比較関数();
            }
            if(c > 0){
                void* tmp = array->buf[i];
                array->buf[i] = array->buf[i+1];
                array->buf[i+1] = tmp;
                swapped = 1;
            }
        }
    }
}
```

これは問題含みな改良です。なぜなら、今回はイルミナ対応ですが、何かちょっと違うリードの
命名ルールを追加するたびに、バブルソート内にif文で分岐が増えていってしまいます。
このような状況には以下のような問題があります。

- 分岐による速度の低下
  - 何が問題?:比較のたびにリード名の種類による切り替えが発生します
  - どうすべき？: 実行回数の多い比較時の計算コストは最小限に抑えるべきなので、ifによる追加は避けるべきです
- 終わることのないソートのロジックの改変
  - 何が問題?:FastQArrayやバブルソートは開発を完了させて、別モジュールとして切り出したいところですが、このままでは永遠にif文の分岐の追加が続きそうです
  - どうすべき？: どう比較するか、という機能を外部から注入できるようにすべきです
  
計算機科学ではソートアルゴリズムの計算量は昔から大きな関心事ですが、
ソートの性能の善し悪しは、単純に比較回数の大小で、比較回数が
少なければ少ないほど良いということです。
  
ただ実際のソート全体の計算のコストは
  
  計算コスト合計 = (そのソートアルゴリズムでの比較回数) * (比較一回あたりの計算コスト)

なので、良いソートアルゴリムを選ぶことも重要ですが、比較一回あたりの計算コストを
小さくしてやることも同じように重要です。前回の例でもリード名からの番号抽出を
比較関数内で行っていたものをデータ読み込み時の1回のみにしてやりましたが、それは
この比較のコストを小さくしてやったことになります。

上記のような問題を解決するため、以下のようなソート関数にします。

ソート関数部分(fastq_array.c)
```C
// private
void bubble_sort(FastQArray* array, int(*usr_cmp_func)(const void*, const void*)){
    int swapped = 1;
    while(swapped){
        swapped = 0;
        for(int i = 0; i<array->length-1; i++){
            if(usr_cmp_func(array->buf[i], array->buf[i+1]) > 0){
                void* tmp = array->buf[i];
                array->buf[i] = array->buf[i+1];
                array->buf[i+1] = tmp;
                swapped = 1;
            }
        }
    }
}
```

```usr_cmp_func(array->buf[i], array->buf[i+1])```の「usr_cmp_func」は普通の
関数のように見えますが、宣言を見ると ```int(*usr_cmp_func)(const void*, const void*)```
となっています。この場合のusr_cmp_funcは関数のポインタ(または関数へのポインタ)と
呼ばれるもので、関数(のポインタ)を変数に入れておいて、他の関数の引数として渡したり、
その中で呼び出させたりできるというものです。

実際にどのような関数を渡すのかというと、今回の場合は以下のような
二つの関数です。

イルミナ用比較関数(illumina_fastq.c)
```C
int illumina_read_comparison(const void* fq1, const void* fq2){
    IlluminaFastQ* p1 = (IlluminaFastQ*)fq1;
    IlluminaFastQ* p2 = (IlluminaFastQ*)fq2;
    if(p1->tile != p2->tile){
        return p1->tile - p2->tile;
    }else if(p1->xpos != p2->xpos){
        return p1->xpos - p2->xpos;
    }else if(p1->ypos != p2->ypos) {
        return  p1->ypos - p2->ypos;
    }
    return 0;
}
```
SRA用比較関数(sra_fastq.c)
```C
int sra_read_comparison(const void* fq1, const void* fq2){
    SRAFastQ* sfq1 = (SRAFastQ*)fq1;
    SRAFastQ* sfq2 = (SRAFastQ*)fq2;
    return sfq1->index - sfq2->index;
}
```

例えばSRA用の関数をソートに使わせる場合には、次のようにしてソートを呼ぶことになります。
```
bubble_sort(array, sra_read_comparison);
```
usr_cmp_func()という関数はリード名の比較を行いますが、これはポインタなのでusr_cmp_func()関数の実体はなく、
その実体はusr_cmp_funcに代入されているillumina_read_comparisonかsra_read_comparisonです。

「int(*usr_cmp_func)(const void*, const void*)」という引数は複雑に見えるかもしれませんが、
単純に一つの関数の型を定義したものです。これ全体で1個の関数の型です。実に長いですが。
 「p1とp2という比較したい構造体のポインタのポインタを二つ受け取り、比較した結果を返す関数」
 という定義をしています。
 
> **Note**
> 引数の型はSRAFastQ*やIlluminaFastQ*でも良いのですが、標準ライブラリにも使えるようにqsort()に渡す比較用関数と同じvoid*を使用した関数シグニチャにしています

```C
void FastQArray_sort(FastQArray* array, int(*usr_cmp_func)(const void*, const void*)){
#ifdef ASSUME_UNSORTED
    qsort(array->buf, array->length, sizeof(FastQ*), usr_cmp_func);
#else
    bubble_sort(array, usr_cmp_func);
#endif
}
```
ソート関数はデフォルトではこれまでに作成したバブルソートを呼ぶようにしておくとして(ASSUME_UNSORTEDは定義しない)、
マクロで簡単に切り替えらられるようにしておきました。バブルソートはソート済みデータが入ってきた場合には
比較回数が少ないアルゴリズムですが、主に未ソートのデータが入ってくる想定の
場合はクイックソートなどの方が良いでしょう。クイックソートは標準ライブラリに
組み込まれているので、ASSUME_UNSORTEDを定義すればそちらが呼び出されることになります。

この関数の呼び出しのレベルであればif文で切り替えても問題にはならないので、
ビルド時に決定できるならどちらのソートアルゴリズムを使用するかはマクロで切り替え、
実行時でないと判断がつかないならif文や、関数の引数にソート関数のポインタを渡すようにするのでも
良いでしょう。

> **Note**
> 関数シグニチャ: 通常は関数は「関数名」と「引数の数と種類」、そして「返り値」の3つセットで定義されます。
> ただし、関数を関数のポインタを介して利用する場合は、関数名は違っていても引き数の数と種類と返り値が
> 同じ関数同士は「同じ型の関数」とみなして同じように呼び出すことができます。このような関数のポインタの定義は
> int(*usr_cmp_func)(const void* p1, const void* p2)のように書かれ、
> usr_cmp_funcが関数のポインタ、int(*)(const void*, const void*)の部分は
> 関数のシグニチャ(英語: function prototype)と呼ばれます。要するにシグニチャは関数から名前を抜いた
> 「引数の数と種類(入力)」と「返り値(出力)」のみで決定される「関数の型」と考えていただければ良いと思います。

ただし、ちょっと困ったことに標準ライブラリのqsort()関数は比較関数に比較対象データの
ポインタを渡してくるので、実際のポインタをソートするコードは以下のようになります。

qsort互換イルミナ用比較関数(illumina_fastq.c)
```C
int illumina_read_comparison(const void* fq1, const void* fq2){
    IlluminaFastQ* p1 = *(IlluminaFastQ**)fq1; # ポインタのポインタをデリファレンス
    IlluminaFastQ* p2 = *(IlluminaFastQ**)fq2;
    if(p1->tile != p2->tile){
        return p1->tile - p2->tile;
    }else if(p1->xpos != p2->xpos){
        return p1->xpos - p2->xpos;
    }else if(p1->ypos != p2->ypos) {
        return  p1->ypos - p2->ypos;
    }
    return 0;
}
```
qsort互換SRA用比較関数(sra_fastq.c)
```C
int sra_read_comparison(const void* fq1, const void* fq2){
    SRAFastQ* sfq1 = *(SRAFastQ**)fq1; # ポインタのポインタをデリファレンス
    SRAFastQ* sfq2 = *(SRAFastQ**)fq2;
    return sfq1->index - sfq2->index;
}
```

qsort互換ソート関数部分(fastq_array.c)
```C
// private
void bubble_sort(FastQArray* array, int(*usr_cmp_func)(const void*, const void*)){
    int swapped = 1;
    while(swapped){
        swapped = 0;
        for(int i = 0; i<array->length-1; i++){
            if(usr_cmp_func(&array->buf[i], &array->buf[i+1]) > 0){ # アドレス渡しになっている
                void* tmp = array->buf[i];
                array->buf[i] = array->buf[i+1];
                array->buf[i+1] = tmp;
                swapped = 1;
            }
        }
    }
}
```
> **Note**
> ```*(IlluminaFastQ**)fq1```はやや難解に見えるかもしれません。わかりやすいように分解すると、まず
> ```(IlluminaFastQ**)fq1```はvoid*型として渡されてきたfq1はそのままでは型が分からなくて使えないので、
> キャストして```IlluminaFastQ**```型に変換しています。次に*演算子を外からかけて```*((IlluminaFastQ**)fq1)```とすることで
> ポインタを取り出しています。２行に分けて書くと<BR/>
> ```IlluminaFastQ** pointer_to_pointer_to_fastq1 = (IlluminaFastQ**)fq1;```<br/>
> ```IlluminaFastQ* p1 = *pointer_to_pointer_to_fastq1;```<br>
> となります。

ここで大事なことは、アルゴリズムなどの外部から瞬時に処理を切り替えられるようにしておくことです。
このような切り替えはデータの種類が増えた場合だけでなく、CPU vs GPUや、シングルスレッド vs マルチスレッド、
あるいはチューニング前vs チューニング後など、改良前と改良後で計算結果が同じでなければならない
場合のシステムのテストなどにも有効です。もっとも、何の要求が来るかは、予知能力でもないと予測は難しいので、
実際には何かこのような機能の追加やGPU対応化が必要になった時点で関数を切り出し、
必要があればそれをポインタとして扱えるように共通化するといった作業をすることになります。

機能を追加するためには、その前にまず新機能と旧機能の呼び出しの条件をそろえたり
関数のシグニチャをそろえたりといった作業をすることになりますが、既存の機能を壊すことのないように
ないようにしなければなりません。これがいわゆる「リファクタリング」と呼ばれる作業の一種です。
今回の場合はイルミナ用比較関数(illumina_fastq.c)を作って引数として渡せるようにする作業は、
いわゆるエンハンス、改良作業にあたりますが、それ以外の関数を切り分けたり、関数をポインタ経由で
呼び出せるように変更する作業、FastQをSRAFastQに変更するなどの作業は古い機能を温存しつつ
新機能を追加するための準備作業でした。リファクタリングは、古い機能を要素に分解して
整理しなおし、新しい要素と組み合わせられるようにする作業と言えます。

