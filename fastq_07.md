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
共通になるので、その後ろだけを見て並び替えれば良いことになります。実際に
Oprical dupilcationの除去は同一タイル内で、X,Y座標が近いリード同士で
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
} IlluminaFastQ;
```
イルミナのリードの比較の優先順位としては <strong>tile number &gt; x coordinate &gt; y coordinate</strong>
ということになります。

扱うデータの種類は増えますが、可変長配列とソートのアルゴリズムは再利用して、
SRAのソートとイルミナのソートを簡単に切り替えられるようにしようと思います。

まず、ソートもバブルソートの名前を付けて別の関数bubble_sort()に分離し、比較関数も切り替えやすいように
分離することにします。
          
リード名の比較はバブルソートのアルゴリズム内で行なっているわけですが
そこにイルミナのリード名の比較機能を直接追加するとした場合、以下のような
コードになるでしょうか。

```C
void bubble_sort(FastQArray* array){
  ...（略)...
  if(イルミナの場合){
     // illumina read名の比較
  }else if(SRAの場合){
     //  SRA read名の比較
  }
  ...（略)...
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
比較関数内で行っていたものをデータ読み込み時の1回にしてやりましたが、それは
この比較のコストを小さくしてやったことになります。

上記のような問題を解決するため、以下のようなソートや比較の仕組みにします。

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

- comp_index(): リード名比較用関数
  - p1とp2として比較したい構造体のポインタのポインタを二つ受け取り、比較した結果を返します
    - 引数の型はSRAFastQ*やIlluminaFastQ*でも良いのですが、標準ライブラリにも使えるようにqsort()に渡す比較用関数と同じ関数シグニチャにしています

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

void FastQArray_sort(FastQArray* array, int(*usr_cmp_func)(const void*, const void*)){
    bubble_sort(array, usr_cmp_func);
    //  qsort(array->buf, array->length, sizeof(FastQ*), usr_cmp_func);
}
```
ソート関数はデフォルトではこれまでに作成したバブルソートを呼ぶように
しておきます。バブルソートはソート済みデータが入ってきた場合には
比較回数が少ないアルゴリズムですが、主に未ソートのデータが入ってくる想定の
場合はクイックソートなどの方が良いでしょう。クイックソートは標準ライブラリに
組み込まれているので、そこにコメントアウトしているもののように呼びだすことができます。

> **Note**
> 関数シグニチャ: 関数は、「関数名」と「引数の数と種類」、そして「返り値」の3つセットで定義されます。
> ただし、関数を関数のポインタを介して利用する場合は、関数名は違っていても引き数の数と種類と返り値が
> 同じ関数同士は「同じ型の関数」とみなして同じように呼び出すことができます。このような関数のポインタの定義は
> int(*usr_cmp_func)(const void* p1, const void* p2)のように書かれ、
> usr_cmp_funcが関数のポインタ、int(*)(const void*, const void*)の部分は
> 関数のシグニチャと呼ばれます。要するにシグニチャは「引数の数と種類」と「返り値」で決定される「関数の型」と
> 考えていただければ良いと思います。
