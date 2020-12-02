# Gameboy Summary

This is just the most important bits pulled from various sources, mostly from the GB.pdf that is also in docs.

#### Specs

- CPU: 8 bit (Similar to Z80)
- Main RAM: 8KB
- Video RAM: 8KB
- Screen Size: 2.6"
- Resolution: 160x144 (20x18 tiles)
- Max # of sprites: 40
- Max # of sprite/line: 10
- Max sprite size: 8x16
- Min sprite size: 8x8
- Clock Speed: 4.194304 MHz = 4194304 Cycles per second
- Horizontal Sync: 9198 KHz (9420 KHz for SGB)
- Vertical Sync: 59.73 Hz (61.17 Hz for SGB)
- Sound: 4 channels with stereo sound
- Power: DC6V 0.7W


#### Memory Map

| Start  | End    | Size      | Section                       | Notes |
|--------|--------|-----------|-------------------------------|-------|
| 0x0000 | 0x4000 | 16KB      | ROM Bank 0                    |       |
| 0x4000 | 0x8000 | 16KB      | Switchable ROM Bank           |       |
| 0x8000 | 0xA000 | 8KB       | Video RAM                     |       |
| 0xA000 | 0xC000 | 8KB       | Switchable RAM Bank           |       |
| 0xC000 | 0xE000 | 8KB       | Internal RAM                  |       |
| 0xE000 | 0xFE00 | 8KB       | Echo of Internal RAM          |       |
| 0xFE00 | 0xFEA0 | 160B      | Sprite Attribute Memory (OAM) |       |
| 0xFEA0 | 0xFF00 | 96B       | Empty                         |       |
| 0xFF00 | 0xFF4C | 76B       | IO Ports                      |       |
| 0xFF4C | 0xFF80 | 52B       | Empty                         |       |
| 0xFF80 | 0xFFFF | 127B      | Internal RAM                  |       |
| 0xFFFF | N/A    | N/A       | Interrupt Enable Register     |       |
