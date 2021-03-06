lwhich (which 改造版)
====================

# 追加機能
通常の which に対して次の機能を追加しました。

       -l     シンボリック・リンクの場合、-> の後にリンク情報を表示します。
       -r     シンボリック・リンクの場合、-> の後にリンク先の最終フルパス情報を表示します。
       -d     最終更新日時を「yyyy-mm-dd hh:mm:ss」形式で表示します。
              日時はローカルです。
              ロケールは使っていません。必ずこの形式で出力します。
       -b     サイズをバイト単位で表示します。

改造の元にした which のソースコードは bsd 系のものですので、linux 版の which と少し異なるかも知れません。
また、戻り値は次になっています。

       0      引数に指定したコマンドがすべて見つかり、実行可能な場合
       1      いくつかのコマンドがないか、実行可能でない場合
       2      ひとつもコマンドがないか、実行可能でない場合
       3      無効なオプションを指定した場合
       4      何らかのエラーが起きた場合

最初は、which 実行時に、リンクしているのであればその情報を知りたいと思い、シェルスクリプトを作成しました。
一度に一つのプログラムしか指定できないという簡易的実装でしたが、そこそこ使えました。

タイムスタンプも知りたいな、と思った時に、いっそのこと which コマンドを書き換えてしまおうと思いたち、作成しました。
名前を which 以外にすれば、互換性を気にする必要はなくなるので、lwhich にしてみました。l はリンクを意味します。
.bashrc に「alias lwh='lwhich -l'」などと設定しておけば、タイプ量も減らせて便利です。

# インストール

## 前提

このツールはC言語のソース形式で公開しており、CMake というツールを使って Makefile を作成し、make し インストールする必要があります。  
「Makefile 位自分で書く」という方は CMake は不要です。


## Makefile 生成

build サブ・ディレクトリがなければ、「mkdir build」を実行して作成します。  
build サブ・ディレクトリに移動して、「cmake ../」を実行します。  

## make

build サブ・ディレクトリで、「make」を実行します。


## 適用

build サブ・ディレクトリで、「sudo make install」を実行します。
(または「sudo make install/strip」)


## 注意事項

- ソース内の「__FBSDID(...略」の部分は、良く分からなかったので、単純に linux では無視されるようにしただけの状態です。  
  FreeBSD でコンパイル等する場合、適切に編集する必要があるのかも知れません。
- -b オプションを指定した場合に、複数の結果を表示すると、桁揃えの処理は行っていないので、見づらくはなる可能性はあります。
  8桁までならずれないので、多くの場合は大丈夫だと思います。  
  おまけのようなオプションですし...
- 思ったよりもバイナリのサイズが大きくなりました。gcc よりも clang の方がサイズが小さくなるようです。
- 汎用性を高めるなら、locale を使った方が良いかも知れません。私には不要だったので、locale は使いませんでした。
- 万一、which と置き換える事を画策される場合、次の点に注意が必要です。
  - 戻り値
      linux/debian/jessie の which だと、無効なオプションを指定した場合が 2 になって、
      ひとつもコマンドがなかった場合の戻り値は区別されていないと思われます(ソースコード未確認で、manの情報のみ参照しました)。  
      そういった意味で、which との互換性はない状態です。まぁこれはディストリによって違うのかも知れません。  
      一つもコマンドがない場合の戻り値を1にして、無効なオプションを指定した場合の戻り値を2にするには、ソースコードを編集してください。
      RC_FIND_NONE を 2 から 1 に、RC_DISPLAY_USAGE を 3 から 2 にすれば、良いのかと思います。
  - man 用ファイルのインストール先
      既存の which のものではなく、こちらのものを表示するようにmanpathの設定などを行う必要があります。

# 実行例

私の使っている debian/jessie での実行例です。which はシンボリックリンクになっていました。

1. $ lwhich which  
/usr/bin/which

2. $ lwhich -a which  
/usr/bin/which  
/bin/which  

3. $ lwhich -al which  
/usr/bin/which -> /bin/which  
/bin/which  

4. $ lwhich -ar which  
/usr/bin/which -> /bin/which  
/bin/which  

5. $ lwhich -ald which  
2014-11-08 22:49:45 /usr/bin/which -> /bin/which  
2014-11-08 22:49:45 /bin/which  

6. $ lwhich -aldb which  
2014-11-08 22:49:45      946 /usr/bin/which -> /bin/which  
2014-11-08 22:49:45      946 /bin/which  

which だと、-r オプションの意図がわかりづらいので、別の例を出します。
joe というエディタの例です。このエディタは名前によって挙動を変えるようで、いくつものシンボリックリンクを用意します。
jstar というのは、そのうちの一つで、wordstar ライクな操作が可能になります。  
jstar は「同じディレクトリ」にある joe へのリンクとなっています。(which は別ディレクトリへのリンクでした)

7. $ lwhich jstar  
/usr/bin/jstar  

8. $ lwhich -l jstar  
/usr/bin/jstar -> joe  

9. $ lwhich -r jstar  
/usr/bin/jstar -> /usr/bin/joe  

-l だとリンク先情報がフルパス表示されませんが、-r だとフルパス表示されているのが分かると思います。  
実は -r オプションを指定した場合、シンボリックリンクが多段階に設定されている場合でも、
最終の実体のものを表示するばずですが、その良い例が浮かびませんでした。


# 最後に

バグ、要望、その他ご意見等御座いましたら下記よりお問い合わせください。

- Blog: http://unixer.xyz/
