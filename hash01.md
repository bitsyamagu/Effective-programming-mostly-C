* RefGeneデータの染色体順ソート

遺伝子のリストとしてよく利用されるものの一つに、UCSCの公開しているknownGene.txtや
refGene.txt、ensGene.txtがあります。RefGeneやEnsembl GeneはUCSC由来のデータでは
ありませんが、これらのデータがリファレンスゲノム上に統合されて位置で対応付けられて
いるので対応関係を取りやすい、ファイル形式が統一されていて扱いやすいなどの理由で
便利に使われています。

コマンドでダウンロードする場合は以下のようにします。
```
wget https://hgdownload.cse.ucsc.edu/goldenPath/hg19/database/refGene.txt.gz
```

ダウンロードしたデータはMySQLのダンプデータですが、単純なタブ区切りテキスト形式の
ファイルとして自作プログラムでも簡単に読み込むことができます。各列のデータの意味は
同様にしてダウンロードできるrefGene.sqlファイルに記載されているので、それもあわせて
ダウンロードしておくと良いでしょう。

まず、RefGeneのデータを読み込むコードを以下のように書きます。array.cとarrray.hは
以前に作成したものがそのまま使えるでしょう。

main1.c
```C
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<assert.h>

#include "array.h"

#define BUFSIZE 1024*1024

typedef struct RefGene {
  int bin;
  char name[255];
  char chrom[255];
  char strand;
  int txStart;
  int txEnd;
  int cdsStart;
  int cdsEnd;
  int exonCount;
  char exonStarts[4096];
  char exonEnds[4096];
  int score;
  char name2[255];
  char cdsStartStat[8];
  char cdsEndStat[8];
  char exonFrames[1024];
} RefGene;

RefGene* create_refgene(char* buf){
    char* head = buf;
    char* token = NULL;

    RefGene* r = (RefGene*)malloc(sizeof(RefGene));
    token = strtok(head, "\t"); assert(token != NULL);  r->bin = atoi(token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->name, token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->chrom, token);
    token = strtok(NULL, "\t"); assert(token != NULL);  r->strand = token[0];
    token = strtok(NULL, "\t"); assert(token != NULL);  r->txStart = atoi(token);
    token = strtok(NULL, "\t"); assert(token != NULL);  r->txEnd = atoi(token);
    token = strtok(NULL, "\t"); assert(token != NULL);  r->cdsStart = atoi(token);
    token = strtok(NULL, "\t"); assert(token != NULL);  r->cdsEnd = atoi(token);
    token = strtok(NULL, "\t"); assert(token != NULL);  r->exonCount = atoi(token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->exonStarts, token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->exonEnds, token);
    token = strtok(NULL, "\t"); assert(token != NULL);  r->score = atoi(token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->name2, token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->cdsStartStat, token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->cdsEndStat, token);
    token = strtok(NULL, "\t"); assert(token != NULL);  strcpy(r->exonFrames, token);

    return r;
}
int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char* buf = (char*)malloc(BUFSIZE); // 1MB

    Array array;
    Array_init(&array);

    int count = 0;
    while(1){
        char* p = fgets(buf, BUFSIZE, fp);
        if(p != NULL){
            RefGene* refgene = create_refgene(buf);
            Array_add(&array, refgene);
        }else {
            break;
        }
        count++;
    }

    for(int i = 0; i<array.length; i++){
        RefGene* p = (RefGene*) Array_get(&array, i);
        printf("%s\t%d\t%d\t%s\n", p->chrom, p->txStart, p->txEnd, p->name);
    }
    Array_finish(&array);

    free(buf);
    fprintf(stderr, "%d genes are loaded\n", count);
    return 0;
}
```

このプログラムは読み込んだRefGeneの遺伝子名と位置情報をそのままコンソールに出力します。
実行してみると、でたらめな順序で出力されるのが確認できます。

```bash
$ ./refgene1 refGene.txt |head
chr3    196666747       196669405       NR_046630
chr3    192232810       192234362       NR_046598
chr13   95364969        95368199        NR_046514
chr1    17368   17436   NR_106918
chr1    17368   17436   NR_107062
chr1    17368   17436   NR_107063
chr1    17368   17436   NR_128720
chr1    30365   30503   NR_036051
chr1    30365   30503   NR_036266
chr1    30365   30503   NR_036267
```

これをリファレンスゲノムの染色体順、短腕から長腕の位置順にソートしようというのが今回の目標です。

染色体名は、chr1, chr2, chr3...chr21, chr22, chrX, chrYのようなものなので、そのまま辞書順にソートしてしまうとchr1, chr10, chr11, chr12,...chr19, chr2, chr21, chr22, chr3, chr4...のような並び順になってしまいます。辞書順ではなく、あくまでもリファレンスゲノム順のデータということで、そのようなデータのことをReference ordered dataということでROD形式などと呼んだりします。

やり方としては、各染色体にIDを振って、

| ID | chromosome|
|----|----|
|1 | chr1 |
|2 | chr2 |
|3 | chr3 |
|..| (略) |
|22| chr22 |
|23| chrX |
|24| chrY |

のようにしておいて、IDを使ってソートすればRODになります。
最も単純な実装方法は、以下のようにしてIDを返す関数を作っておいて、RefGeneの読み込み時にIDをRefGene構造体に入れ込んでおいて、比較時にはそのIDを
使ってソートするというものです。

```C
int getID(char* chrom){
   chr* chr_list[] = {"chr1", "chr2", "chr3", ... "chrY"};
   for(int i = 0; i<24; i++){
       if(strtmp(chrom, chr_list[i])==0){
           return i+1;
       }
   }
   return -1;
}
...(略)...
    // どこかで事前にIDを入れておく
     refgene->chr_id = getId(refgene->chrom);
...(略)...
int compare_rod_data(RefGene* rf1, RefGene* rf2){
   if(rf1->chr_id != rf2->chr_id){
       return rf1->chr_id - rf2->chr_id;
   }else if(rf1->txStart != rf2->txStart){
       return rf1->txStart - rf2->txStart;       
   }
   return 0;
}
```
chr_listはファイルから読み込む方式として作れば、どのような並び順にも対応できます。ただ、問題は計算の効率です。getID()内でforループで回して比較しているうえに、strcmp()もその内部では文字比較をループで行っているはずなので、言わば二重ループによる検索です。100個のX染色体のデータを処理するとgetID()はforループだけで2400回ものループを回すことになります。また、デコイ等まで含めた場合にはループは最大数千回は回ることになり、しかもリストに見つからない場合には-1を返すまでリストの末尾まで検索が走ることになります。

このような検索には、一般的にはハッシュテーブルを利用するのが効率的です。ハッシュテーブルがどのようなものかについては、Wikipediaや多数の書籍に良い記事がたくさんあるので、このチュートリアルでは詳しく説明しません。

https://ja.wikipedia.org/wiki/%E3%83%8F%E3%83%83%E3%82%B7%E3%83%A5%E3%83%86%E3%83%BC%E3%83%96%E3%83%AB

要点としては、ハッシュ関数という関数でキー(多くの場合は文字列)を整数に変換して、その整数を配列の添え字としてバリューと呼ばれる値を格納したり、検索したりします。検索に要する計算量はほぼハッシュ値の計算量のみになります。

まずはハッシュ関数が必要です。以下はWikipediaのFowler–Noll–Voハッシュ関数の記事(https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)に掲載されていたFNV-1a hash関数を実装したものです。

hash.cより
```C
uint32_t hash_code_fnv1a(char* src, int len){
    // FNV offset basis: 0x811c9dc5
    uint32_t hash = FNV_offset_basis;

    for(int i = 0; i<len; i++){
        hash = hash ^ *(src+i);
        hash = hash * FNV_prime;
    }
    return hash;
}
```

