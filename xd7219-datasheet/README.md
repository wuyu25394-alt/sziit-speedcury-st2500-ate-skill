# XD7219 Datasheet Skill

把 **XD7219 / XL7219 / XD7221 / XL7221**（信路达 XINLUDA，MAX7219/MAX7221 兼容）8 位数码管驱动芯片的数据手册与实战要点，打包成一个 agent skill。

## 内容

```
xd7219-datasheet/
├── SKILL.md                      # 技能主文件：引脚、协议、寄存器、初始化、字模、亮度、级联、踩坑
├── README.md
├── LICENSE
├── references/
│   ├── XD7219_datasheet_en.pdf   # 信路达官方英文手册（17 页）
│   └── MAX7219-MAX7221_datasheet_zh.pdf  # ADI 官方中文手册（16 页，兼容型号）
└── examples/
    ├── arduino_xd7219.ino        # Arduino SPI 例程
    ├── raspberry_pi_xd7219.py    # 树莓派 spidev 例程
    └── cascade_notes.md          # 级联接线与帧顺序
```

## 安装

DSH（本仓库工作区）：

```powershell
Copy-Item -Recurse xd7219-datasheet "$env:USERPROFILE\.dsh\skills\"
```

Claude Code / 其他支持 Agent Skills 的工具：

```bash
cp -r xd7219-datasheet ~/.claude/skills/
```

重启会话后，技能目录里会出现 `xd7219`，提到 XD7219 / MAX7219 / 8 位数码管时会自动加载。

## 快速开始（5 V Arduino）

| XD7219 | Arduino |
|--------|---------|
| V+ | 5V |
| GND ×2 | GND |
| DIN | D11 |
| CLK | D13 |
| LOAD | D10 |
| ISET | 9.53 kΩ → 5V |

初始化：`0x0F=0x00` → `0x09=0xFF` → `0x0B=0x07` → `0x0A=0x07` → `0x0C=0x01`。

## 重要提醒

- **3.3 V MCU 不能直连**：V+=5 V 时 VIH 最小 3.5 V，需 74HCT245 等电平转换。
- 两个 GND 都要接。
- 上电默认关断、空白，必须写 `0x0C=0x01`。
- 段电流由 RSET 设定，最小 9.53 kΩ（约 40 mA）。

## License

技能文本与示例代码：MIT。`references/` 下的数据手册版权归 XINLUDA / Analog Devices，仅作参考分发，不适用 MIT。

本目录以 `xd7219-datasheet/` 子目录形式并入 `sziit-speedcury-st2500-ate-skill` 仓库；仓库其余内容仍按其根目录 `LICENSE`（CC BY-NC-SA 4.0）授权。
