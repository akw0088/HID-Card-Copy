object FM_Main: TFM_Main
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'ICD Programmer for Microchip'
  ClientHeight = 433
  ClientWidth = 667
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object GB_Devices: TGroupBox
    Left = 8
    Top = 8
    Width = 304
    Height = 93
    Caption = '&Devices'
    TabOrder = 0
    object CB_Devices: TComboBox
      Left = 16
      Top = 24
      Width = 193
      Height = 21
      Style = csDropDownList
      Enabled = False
      TabOrder = 0
    end
    object BT_Connect: TButton
      Left = 215
      Top = 22
      Width = 75
      Height = 25
      Caption = '&Connect'
      Enabled = False
      TabOrder = 1
      OnClick = BT_ConnectClick
    end
  end
  object Button1: TButton
    Left = 121
    Top = 59
    Width = 75
    Height = 25
    Caption = '&Read'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Memo: TMemo
    Left = 8
    Top = 107
    Width = 649
    Height = 318
    Color = clInfoBk
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 2
    WordWrap = False
  end
  object Button3: TButton
    Left = 337
    Top = 28
    Width = 75
    Height = 25
    Caption = '&Erase 0'
    TabOrder = 3
    OnClick = OnErase
  end
  object Button4: TButton
    Left = 418
    Top = 28
    Width = 75
    Height = 25
    Caption = 'Flash'
    TabOrder = 4
    OnClick = Button4Click
  end
  object Button5: TButton
    Left = 337
    Top = 59
    Width = 75
    Height = 25
    Caption = '&Erase 1'
    TabOrder = 5
    OnClick = OnErase1
  end
  object Button2: TButton
    Left = 418
    Top = 59
    Width = 75
    Height = 25
    Caption = '&Erase Boot'
    TabOrder = 6
    OnClick = OnEraseBoot
  end
  object Button6: TButton
    Left = 499
    Top = 59
    Width = 75
    Height = 25
    Caption = '&Erase Panels'
    TabOrder = 7
    OnClick = OnErasePanels
  end
  object Button7: TButton
    Left = 499
    Top = 28
    Width = 75
    Height = 25
    Caption = 'Flash EEP'
    TabOrder = 8
    OnClick = OnFlashEepromDumper
  end
  object Button8: TButton
    Left = 580
    Top = 28
    Width = 75
    Height = 56
    Caption = '&Erase Boot 1'
    TabOrder = 9
    OnClick = Button8Click
  end
  object Timer: TTimer
    Enabled = False
    Left = 40
    Top = 112
  end
end
