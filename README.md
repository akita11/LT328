# LED Tile328の関連ファイル。

ver1.00 by akita11(akita@ifdl.jp)

## インストールと動作チェック

詳しい使い方は LT328man_v100.pdf を参照してください。以下は概要です。

1. ファイルをzipでダウンロード(右のほうの緑の"clone or Download"から)
2. ArduinoIDEから、スケッチ→ライブラリをインクルード→ZIP形式のライブラリをインストール、を選び、1.でダウンロードしたzipファイルを選ぶ。ライブラリに"LT328v2"が追加されていることを確認する
3. ArduinoIDEのツール→ボードから、"Arduino Pro or Pro Mini"を選び、プロセッサから"ATmega328(3.3V, 8MHz)"を選ぶ
4. LED TileとPCをmicroUSBケーブルで接続し、ArduinoIDEでシリアルポートを設定する
5. スケッチの例として"flow"を選び、コンパイルして転送すると、明るい光（レーザーポインタ光やスマホカメラのライトなど）があたったところのLEDが点灯し、本体を傾けるとその方向に模様が流れる ※使う加速度センサがMMA7660(LT328v2)の場合は、LT328v2.hでコメントアウトされている USE_MMA7660 のdefineを戻してください。ADXL345(LT328v3)の場合はそのままでOKです
6. Arduinoディレクトリ内のlibraries/LT328v2/board 以下に基板データが展開されますが、これは不要でしたら削除してください。

## ライブラリ関数の使い方

こちらを参照: http://ifdl.jp/akita/lt/

## お問い合わせ

使い方などについてのお問い合わせは、akita[at]ifdl.jpまでご連絡ください。
