LED Tile328の関連ファイル。
by akita11(akita@ifdl.jp9

インストールと動作チェック
1.ファイルをzipでダウンロード(右のほうの緑の"clone or Download"から)
2.ArduinoIDEから、スケッチ→ライブラリをインクルード→ZIP形式のライブラリをインストール、を選び、1.でダウンロードしたzipファイルを選ぶ
3.ArduinoIDEのツール→ボードから、"Arduino Pro or Pro Mini"を選び、プロセッサから"ATmega328(3.3V, 8MHz)"を選ぶ
4.LED TileとPCをmicroUSBケーブルで接続し、ArduinoIDEでシリアルポートを設定する
5.スケッチの例として"flow"を選び、コンパイルして転送すると、明るい光（レーザーポインタ光やスマホカメラのライトなど）があたったところのLEDが点灯し、本体を傾けるとその方向に模様が流れる

ライブラリ関数の使い方
こちらを参照: http://ifdl.jp/akita/lt/
