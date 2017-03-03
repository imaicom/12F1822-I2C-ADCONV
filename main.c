#include <xc.h>
#include <pic.h>
#include "skI2Cslave.h"         // I2C関数ライブラリ用
#include <string.h>

// CONFIG1
#pragma config FOSC = INTOSC    // INTOSC= 内部クロックを使用し、CLKINピン(RA5)はデジタルピンとする
#pragma config WDTE = OFF       // ウォッチドッグタイマを無効にする
#pragma config PWRTE = ON       // ON = 電源ONから64ms後にプログラムを開始する
#pragma config MCLRE = OFF      // ピンはデジタル入力ピン(RA3)として機能する
#pragma config CP = OFF         // プログラムメモリーの保護はしない
#pragma config CPD = OFF        // データメモリーの保護はしない
#pragma config BOREN = ON       // 電源電圧降下常時監視機能は有効とする
#pragma config CLKOUTEN = OFF   // CLKOUTピンは無効、RA4ピンとして使用する
#pragma config IESO = ON        // 内部から外部クロックへの切替えで起動を行う
#pragma config FCMEN = OFF      // 外部クロックの監視はしない

// CONFIG2
#pragma config WRT = OFF        // 2KWのフラッシュメモリ自己書き込み保護ビットを保護しない
#pragma config PLLEN = ON       // 4xPLLを動作させる(クロックを32MHzで動作させる場合に使用)
#pragma config STVREN = ON      // スタックがオーバフローやアンダーフローしたらリセットを行う
#pragma config BORV = LO        // リセット電圧(VBOR)のトリップポイントを低(1.9V)に設定する
#pragma config LVP = OFF         // 低電圧プログラミングしない

// PICKIT3 Setting
//
// Program Options
//   Erase All Before Program [*]
//   Program Speed [2(fastest)]
//   Enable Low Voltage Programming [ ]
//   Programming Method [Apply Vdd before Vpp]
//
// Power
//   Power target circuit from PICkit3 [*]
//   Voltage Level [4.5]

#define _XTAL_FREQ 32000000

static void Delay_ms(unsigned int DELAY_CNT) {
    for (unsigned int i = 0; i < DELAY_CNT; i++) {
        __delay_ms(1);
    }
}

/*  Pin assign PIC 12F1822
         +---__---+
  Vdd    | 1    8 | Vss
  RA5    | 2    7 | RA0/AN0
  RA4/AN3| 3    6 | RA1/SCL
  RA3    | 4    5 | RA2/SDA
         +--------+
 */

unsigned int adconv() {
    unsigned int temp;

    GO_nDONE = 1;         // アナログ読取り開始
    while(GO_nDONE);      // 読取り完了待ち
    temp = ADRESH;        // 値をADRESHとADRESLのレジスターにセット
    temp = (temp << 8) | ADRESL;  // 10ビットの分解能力
    return temp;
}

void main() {
    unsigned int temp;
    
    OSCCON  = 0b01110000; // 内部クロックを32MHzとする
//  OSCCON  = 0b01111010; // 内部クロックを16MHzとする
//  OSCCON  = 0b01110010; // 内部クロックは8MHzとする
    OPTION_REG = 0b00000000; // デジタルI/Oに内部プルアップ抵抗を使用する
//    ANSELA = 0b00000000; // アナログ使用しない(すべてデジタルI/Oに割当てる)
//    ANSELA = 0b00010000; // アナログはAN3を使用し、残りをすべてデジタルI/Oに割当
    ANSELA = 0b00010001; // アナログはAN0,AN3を使用し、残りをすべてデジタルI/Oに割当
//    TRISA   = 0b00001110; // ピンはRA1(SCL)/RA2(SDA)のみ入力(RA3は入力専用)
//    TRISA   = 0b00011110; // ピンはRA1(SCL)/RA2(SDA)のみ入力(RA3は入力専用) アナログはAN3を使用
    TRISA   = 0b00011111; // ピンはRA1(SCL)/RA2(SDA)のみ入力(RA3は入力専用) アナログはAN0,AN3を使用
//  WPUA       = 0b00001000; // RA3は内部プルアップ抵抗を指定する
//  PORTA   = 0b00000000; // 出力ピンの初期化(全てLOWにする)

    ADCON1 = 0b10010000; // 読取値は右寄せ、A/D変換クロックはFOSC/8、VDDをリファレンスとする
//  ADCON0 = 0b00000001; // アナログ変換情報設定(AN0から読込む)
    ADCON0 = 0b00001101; // アナログ変換情報設定(AN3から読込む)
    Delay_ms(1); // アナログ変換情報が設定されるまでとりあえず待つ (*1:20→5)
    InitI2C_Slave(8); // スレーブモードでの初期化、マイアドレスは8とする

    Delay_ms(1000) ;

     while(1) {
        temp = adconv(); // RA4ピンの3番ピン(AN3)から半固定抵抗の値を読み込む
        if(I2C_ReceiveCheck() >= 1) {  // 受信バイト数確認
            switch(rcv_data[0]) {
                case 0xA0:  ADCON0 = 0b00000001; // アナログ変換情報設定(AN0から読込む)
                            break;
                case 0xA4:  ADCON0 = 0b00001101; // アナログ変換情報設定(AN3から読込む)
                            break;
                case 0xAD: snd_data[0] = temp & 0xFF; snd_data[1] = (temp & 0xFF00) >> 8;  break; // 変換結果を書き出す
            };
        };
     } //while(1)
}
