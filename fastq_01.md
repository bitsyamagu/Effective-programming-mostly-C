
# メモリに読み込む
FastQファイルはシーケンサーで読んだ塩基配列を保存する形式として
非常によく用いられるファイル形式です。あまりBioinformaticsに興味がないという方は
ただの4行で1セットのテキストファイルだということさえ理解していれば十分です。

>生命科学関係の方へ<br/>
>以下のデータは151塩基の塩基配列のFastQの1エントリをSRAから抽出してきたものです。
>SRAなどのデータはパブリックドメインのものとして利用できるので、
>ソフトウェアのテスト等に利用しやすいので、もし入手方法をご存じでない場合は
>把握しておくと便利でしょう。
>
次世代シーケンサー(NGS)から出力されるFastQファイルは4行で1セットで、1行目が配列名、
2行目が塩基配列、3行目は「+」、4行目は各塩基のBase qualityになっています。
FastQ形式は次世代シーケンサーの登場以前から使われていたファイル形式なので、
様々なFastQ形式のバリエーションに対応しようとすると大変ですが(後述)、
NGS関連では4行1セットでDNA配列1本分の情報として簡単に扱えるとみなして問題ないでしょう。

<pre style="white-space:nowrap;">
@SRR8206481.1 1/1
GGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTGGGGTAGGGGTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGTTAGGGTTAGGG
+
AAFFFJJJJFJJJJJFJJJJJJJJJJJJJJJJJJJJJJJJFFJJ-7FAJJ-AFJJJ7FJFFJAFJJJJ-FFJJJ-FFFJJ-AAFJJ-7AJJJAJJJJJFF7FJF7&lt;&lt;FFJJ&lt;&lt;FJF&lt;7-&lt;FA-7-&lt;FAAJ&lt;F7A-77FF7--A&lt;F---FJ7
</pre>

このチュートリアルでは、このような配列が12本格納されたFastQ形式のファイル
[test.fastq](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_01/test.fastq)
を配列名順に並び替える(ソートする)プログラムを構築していきます。
一回で全部を作るのは難しいので、簡単なものから始めて徐々に改良を加えていきます。
開発中のプログラムはいくつもの問題点や制約がありますが、
それらは改善されていくので地道に見ていきましょう。

まず、計算機でデータを扱うためには、データをメモリ上で扱えるようにHDDから読み込まなければなりません。
今日の計算機はみなノイマン型コンピュータなので、メモリ上にデータをロードしてこないとCPUでは扱うことはできません。
HDDやSSDはあくまでも補助記憶装置なので、そこにあるデータを使って足し算したり文字列操作をすることはできず、
メモリに読み込んだデータを使用して計算します。

ただ闇雲にFastQファイルを文字列として読み込むだけでは後で並び替えなどの操作が面倒になるので、
構造体を作ることにして、構造体1個に配列が一本入るようにします。

このために必要な構造体は次のようなものです。
```C
typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;
```
なお、このような構造体の定義方法は古い入門書には書いてないかもしれませんが、
C11規格で導入されたものなので心配は要りません。無駄なく読みやすい方式で
いきましょう。

この構造体では配列名に256文字、配列に256文字、BaseQualityに256文字分の記憶容量を確保することにしています。
実際には文字列の最後は'\0'つまりナル文字を入れることになるので、実際に利用できるのは255文字までです。

以下のソースコードはFastQファイルを12本メモリに読み込むプログラムです。実際に読者の方もコピペではなく、
写経するようにしてプログラムをファイルに書き写してみて、コンパイルしてみてください。
もし、誤字があればコンパイルエラーになるので、それらのコンパイルエラーを経験するのも学びの一部です。
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
    FastQ data[12];
    int index = 0;
    while(1){
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            break;
        }
        strcpy(data[index].name, buf);
        // seq
        fgets(buf, 256, fp);
        strcpy(data[index].seq, buf);
        // +
        fgets(buf, 256, fp);
        // qual
        fgets(buf, 256, fp);
        strcpy(data[index].qual, buf);

        index++;
    }

    for(int i = 0; i<12; i++){
        printf("%s", data[i].name);
        printf("%s", data[i].seq);
        printf("+\n");
        printf("%s", data[i].qual);
    }

    return 0;
}
```
それほど難しい箇所は無いと思いますが、プログラムを書く上では不可欠の関数がいくつか登場したので、使い方を確認しておくと良いでしょう。
- fopen
  - ファイルを開く関数
- fgets
  - 改行文字までの1行を読み込む関数。NULLを返した場合はファイルの末尾なのでループを脱出します
- strcpy
  - 文字列をコピーする関数

このソースコードをmain.cというファイルに保存して、以下のようなコマンドでコンパイルすると、fastqという実行ファイルが生成されます。
```
clang -std=c11 -o fastq main.c
```

以下のようなコマンドで実行できます。実行するとprintfでFastQのデータがそのまま出力されます。
```
./fastq test.fq
```

ここまでくれば、あとはFastQ data[12]の箇所を```FastQ data[4000000]```にしてやれば、400万本の配列も読み込めるでしょうか？

実はそうではありません。FastQ data[4000000]にソースコードを修正してコンパイルすればコンパイルはできますが、実行すると以下のようなエラーで異常終了してしまいます。
```
Segmentation fault (core dumped)
```
400万本というのはメモリ容量としては256 x 3 x 4000000 = 2.8GBほどなので今日ならパソコンでも大丈夫のはずですが、実際には400万本と言わずとも、1万1千本でも異常終了してしまいます。1万本は大丈夫ですが1万1千本ではエラーになります。1万1千本というと使用メモリ容量は概算で、256 byte x 3 x 11000 = 8448000 byte、つまり8.06MBほどです。

# スタックとヒープ(Stack and Heap)
実は、このプログラムの最大の問題点は、構造体の配列をFastQ data[サイズ]のように宣言してしまったことにあります。このように宣言された変数は、<strong>スタックメモリ</strong>と呼ばれるメモリ領域に確保されます。その容量が一般的なOSでは8MBしかないのです。

プログラムが関数呼び出しをしたりループなどのブロックに入ることは、新しいスコープに入ることでもあります。関数に入るとそれまでにアクセスできた自動変数たちはスタックメモリに積まれ、その上に新たにアクセス可能となった自動変数が積まれます。そして関数から出るときにはスタックから降ろされます。このスタックメモリの容量は1プロセス(1個のプログラムの実行)あたり8MBしかないので、それ以上のメモリを使おうとすると「stack overflow」と呼ばれる状況になり、エラーになります。スタックメモリの容量は管理者権限があれば設定を変更して増やすこともできますが、プログラムの動作中に頻繁にアクセスすることになるので容量が十分に小さくてCPUに近いところにあるのが望ましいので、基本的には8MBのまま使用することが多いと思います。
![stack](..%5C..%5C..%5CDesktop%5Cstack.png)

スタックメモリにはこのような制約がありますが、一方で自由に使えるメモリとして
<strong>ヒープメモリ</strong>があります。ヒープメモリから12個分のFastQ構造体のメモリを確保するには、以下のようにします。

```C
FastQ* data = (FastQ*)malloc(sizeof(FastQ)*12);
```
ここで、dataというポインタ変数はスタックに積まれますが、
メモリ容量としては64bitの計算機で8バイトです。
これは以下のように400万個にした場合でも、スタックに積まれるのは8バイトだけです。

```C
FastQ* data = (FastQ*)malloc(sizeof(FastQ)*4000000);
```
既にお気づきかもしれませんが、malloc()などで取得したメモリはポインタとして取得されるので、FastQ data[12]のような自動変数として扱うことはできません。

実際のシステム開発ではメモリの使用量が8MBで済むということはまずないので、どのようなプログラムの開発でも
ポインタを利用することは避けられないということになります。次回は、実際にmallocを
使用してヒープメモリを使用したプログラムを書いてみます。

# まとめ
1. データはメモリに読み込まなければ計算には使えない
2. FastQのように複数のデータを一まとめに扱うには構造体を使う
3. スタックメモリは8MBしかないので、むやみに使うとすぐ枯渇する
4. ヒープメモリはいくらでも使えるが、ヒープを使うなら必ずポインタでアクセスしなければならない


----
FastQ形式の解説についてはWikipediaにも記載がありますが、これは昔に私が書いたものからほぼ変わっていないようなので、
興味のある方は、ぜひ英語版などを参考にして加筆修正してください。
[Fastq](https://ja.wikipedia.org/wiki/Fastq)

ちなみに本来的なFastQファイルの例は以下のようなものです。1行目は配列名で、2列目以降は+の行の手前までが塩基配列、
＋の行から塩基配列と同じ文字数のBase qualityのデータが記録されています。ABIなどのシーケンサーから
出力された塩基配列はこのような形式のままかもしれません。

最近のPythonなどのスクリプトではこのような形式では読み込みにくいのですが、C言語なら読み込み
やすいかということそういうこともなく大変です。大変ですが、一行の文字数が一画面内に
収まる程度になっているので、C言語のようなプログラミング言語にとっては用意しておかなければ
ならないバッファサイズが固定長で済むので、無制限に長い行に対応する読み込み機能を
作るよりは遥かに簡単で、バッファオーバーランなどのメモリのアクセス違反の懸念も無いので安全です。

```
@FSRRS4401EG0ZW_4 [length=424] [gc=23.82] [flows=800] [phred_min=0] [phred_max=40] [trimmed_length=389]
tcagTTTTGATCTTTTAATAATGAATTTTAATGTGTTAAAATGATTGCATTGATGGCATAACCGCATTTAAATTAATTAC
ATGAAGTGTAAGTATGAAATTTTCCTTTCCAAATTGCAAAAACTAAAATTTAAAATTTATCGTAAAAATTAACATATATT
TTAAACGATTTTAAGAAACATTTGTAAATTATATTTTTGTGAAGCGTTCAAACAAAAATAAACAATAAAATATTTTTCTA
TTTAATAGCAAAACATTTGACGATGAAAAGGAAAATGCGGGTTTGAAAATGGGCTTTGCCATGCTATTTTCATAATAACA
TATTTTTATTATGAATAATAAATTTACATACAATATATACAGTCTTAAATTTATTCATAATATTTTTGAGAATctgagac
tgccaaggcacacaggggataggn
+
IIA94445EEII===>IIIIIIIIICCCCIIHIIICC;;;;IIIIIIIIIIIIIIIIIIIIIIIIIF;;666DDIIIIII
IIIIIIIIIIIIIEE94442244@@666CC<<BDDA=-----2<,,,,659//00===8CIII;>>==HH;;IIIIIICC
@@???III@@@@IC?666HIDDCI?B??CC<EE11111B4BDDCB;=@B777>////-=323?423,,,/=1,,,,-:4E
;??EIIIIICCCCI>;;;IIIIIII<<@@?=////7=A99988<<4455IEEEIIIIIIIIIIIII<999HIIIIIIIII
II?????IIIIIIIIIIICAC;55539EIIIIIIIIIIIIIIIIHH999HHHIA=AEEFF@=.....AD@@@DDEEEEFI
II;;;977FFCCC@24449?FDD!
```
