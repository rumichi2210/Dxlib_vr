# Dxlib_vr
DxlibでVRソフトウェアを作成するサンプルです

## 動作確認
| HMD         | コントローラー                | 
| ----------- | ----------------------------- | 
| Vive Pro2   | vive Controller               | 
| Vive Pro2   | index Controller              | 
| Meta Quest2 | Meta Quest2付属コントローラー | 

## 注意事項
#### 以下の設定を必ずしてください
DxLib_Init()より前  
DxLib::SetUseDirect3DVersion(DX_DIRECT3D_11);
SetZBufferBitDepth(24or32);  
SetCreateDrawValidGraphZBufferBitDepth(24or32);  
SetWaitVSyncFlag(false); 

DxLib_Init()より後  
SetUseRightHandClippingProcess(TRUE);
