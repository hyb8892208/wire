Version 1.4 （App9）
1. 修改文件 EmuVcard_g9.h，card_g9.h EmuVcard_g9.cpp,VcardGenerator_Translator_g9.cpp Emu_Rdr_Common_g9.h 
2. 应用例子：TestMultiEmuBankThread_g9.cpp
3. 修改内容
    a. 增加 Uim card 的 Usim Apdu 指令模式 （EC20 模块中使用此指令）
    b. 支持超长指令参数长度； 在Usim Apdu指令中扩展原适用于Uimcard 的指令集， 
    c. 增加PIN Verfiy CHV 支持机制， 提供Bank端 索要及判决Pin码的机制， 提供Emu 端获取实际Pin的外部接口
    d. 修改1.3版本Bug （如修改读取>256长度文件Bug）

    需要固件配合升级 (    EC20 部分APDU指令字节间隔超过ISO7816标准规定) 
    (名称: SimEmu180625_.bin, SimCardEmuAdaptor180625_.bin     )

version[SimEmuII V1.0.1.0_46Jun 25 2018 16:32:10]
Mini52[slot:0] version[SimCardEmuAdaptor V1.0.Jun 25 2018 16:45:15]

