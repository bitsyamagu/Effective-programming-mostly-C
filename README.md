# 写経とインクリメンタル開発によるプログラミング学習

## 計算が遅いなー

とつぶやきながら仕事をしていないでしょうか？

実用的なプログラムを書くにあたり、現在ではPythonやJavaを利用してプログラムを書くことは
ごく当たり前のような状況になっています。並列計算機や大型のサーバの場合でも開発の速度が
優先される状況では、やはりこれらのプログラミング言語が用いられますが、

「なぜか計算が遅いのだけど計算量に対してこの時間と負荷は妥当？」

というような状況にしばしば直面します。実際には大量のページフォルトを
起こしていたり、バグって暴走していたりするわけですが、Pythonのような言語でしか
プログラムを書いた経験のない人はそれに気づくときの
体感温度が高すぎると言いますか、気付きにくいということが
私の身の回りではしばしば観測されます。遅いだけならまだしも
最悪の場合は並列計算機を停止させたり、故障させたりすることに
なるので、なかなか放置するわけにもいかず障害対応に
追われて忙しくなる原因になってしまいます。

計算が遅くなる原因はプログラムのコードに限らず、ハードウェアの仕様によるものや、OSの設定によるものなど
様々ですが、技術的なことはさておき、まずは「計算が遅すぎるのではないか？」「想定していた
計算量に対して計算機の負荷が高すぎるのではないか？」ということに気付けることが大事です。
これに気付けるかどうかは、扱っているデータ量や計算内容からシステムテスト、過去の経験等を通して
経験的に把握している計算時間が基準になっていると思います。
おそらく、この体感的な計算の所要時間がPythonのような言語だけしか利用経験がないと
遅いことに慣れてしまったために、異常があっても気づきにくくなってしまっているのかも
しれません。

C言語のようなプログラミング言語は、コードを書くことはやや技術的に苦労が多いのですが、
ひとたび計算が回るようになってしまえば計算速度で裏切られることは少ない言語で、運用する
側にとっては比較的素直に扱うことができます。
おそらく、C言語の経験者は、計算時間を計れる体内時計を持っているので、そこから
大きなズレを感じるといてもたってもいられなくなります。この資料の目的は
読者の皆さんにそのような体内時計を身に着けてもらうことを目的の一つにしています。

## ポインタは難しい。でも、ポインタの何が難しい？
C言語といえば、必ずといっていいほど「C言語のプログラミングは挫折しやすい」
とされていますが、周囲を見回してみると以下のような理由があるようでした。

1. なぜポインタが必要か分からない
2. ポインタのポインタ？
3. ポインタの演算？？
4. ポインタ？参照？ハンドル？？？
5. またsegmentation fault？？？？

このような疑問に答えるには、計算機の仕組みや設計手法について学ばないと理解できません。
あるいはC言語の入門書だけではなく様々な他のプログラミング言語と比較してみるのも良いでしょう。
もし1番のような段階で止まったままPythonでコードを書いているとすると、動いているとしても
設計に問題を抱えていて、コードがスパゲッティのように絡み合って読みにくくなっていたり、
計算中に予期しないエラーが起きても原因の究明が困難になっているのではないでしょうか。

PythonでもPerlでも、この資料で学んだことはきっと役に立つでしょう。PythonやPerlは
便利な反面、様々な技術的な詳細が覆い隠されているので、そこに潜んでいるような
問題を解決するにはその詳細について理解している必要があるのです。

## 対象とする読者と前提として必要な知識
プログラムを書きたい人、書いている人で「計算が遅いなー」というところから前進したい人や
HPCやGPUプログラミングにこれから挑戦したい人で、C言語を使ったことがない人を想定しています。
とはいえ、入門書は世の中にたくさんありますから、入門書は市販のものを読んでいただくとして、
C言語の文法くらいは覚えた後に、この資料を読んでください。

なお、この資料ではプログラムを高速化する技法については紹介しませんが、著しい低速化を
招くような現象や、トラブルを起こしやすい問題については折に触れて紹介して
いきたいと思います。

## 進め方
写経と書いていますが進め方は自由です。例えば次のようなやり方です。
1. コードを丸写しして動かしてみる
2. 本文を読む
3. もう一度コードを丸写しする
4. 見ないで同等のコードを書く

写経することのメリットは、頭で覚えるのではなくて手が覚えてくれるという
ことです。自転車をこぐときに右足と左足を意識してこぐ人はいないように、
無意識のうちにプログラミングのタイピングをできるようにする
運筆の練習のような作業です。もし、コンパイルエラーに悩まされたときは
diffコマンドでお手本と比較してみてください。そしてコンパイルエラーの
エラーメッセージを丁寧に読み直して解釈してみてください。面倒に思われる
かもしれませんが、エラーのログを丁寧に読むのは、どんな場合でも一番の近道です。

ただ、エラーメッセージはなかなか分かりにくいものもあるので、
どのようなエラーメッセージがどのようなエラーに対して出るのかを
知っておくと、実際に後でそのようなエラーに遭遇したときに
役立つでしょう。

----

## 目次
1. [メモリに読み込む](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_01.md)
2. [ヒープメモリを使う](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_02.md)
3. [伸びる配列 - 可変長配列その1 Array(Vector)](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_03.md)
4. [伸びる配列 - 可変長配列その2 Linked list](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_04.md)
5. [よりスマートな構造](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_05.md)
6. [並び替え(ソート)](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_06.md)
7. [イルミナのリード名によるソート](https://github.com/bitsyamagu/Effective-programming-mostly-C/blob/master/fastq_07.md)
 
## より良いコーディング
1. Standards (標準)に準拠する
2. エラーに備える
3. Segmentation faultに対応する

