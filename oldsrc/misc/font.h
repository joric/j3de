//super-cool-antialias-truetype font writer

//data is just a long horizontal 2bpp stripe "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//char width is determined by the pixels on the top line

#define tlength 192
#define tbytes 1516
#define ch_hgt 49
#define ch_wid_def 40

#define PM_DESIGNED 0
#define PM_CENTERED 1
#define PM_ALIGN_LEFT 2
#define PM_ALIGN_RIGHT 3

unsigned int ttf[] = {
	0x04D524F46, 0x0B6250000, 0x04D424C49, 0x044484D42, 0x014000000, 0x032000006, 0x000000000, 0x000000001,
	0x00B0A0000, 0x032000006, 0x050414D43, 0x006000000, 0x0FC040404, 0x04F42FCFC, 0x000005944, 0x0FFDF8025,
	0x0FFFDFFFF, 0x0DFFFFFFF, 0x0FDFFFFFF, 0x0FFFFFFFF, 0x0FFFFFFDF, 0x0FFFFFFFD, 0x0FFFFDFFF, 0x0FFFFFDFF,
	0x0FFDFFFFF, 0x0FFFDFFFF, 0x0DFFFFFFF, 0x0FFFFFFFF, 0x0FFFFBFFF, 0x0FFF7FFFF, 0x0FFFFFFFF, 0x0FFFFFFEF,
	0x0FFEFFFFF, 0x0FBFFFFFF, 0x0FFFFFFFF, 0x0FFFFFFFB, 0x07FFFFFFF, 0x0FFFFFFFF, 0x0FFFEFFFD, 0x07FFFFFFF,
	0x0FFFFFFFF, 0x0FFFFFFFD, 0x0FFFFDFFF, 0x0FEFFFFFF, 0x0FFFFFFFF, 0x0FFFFFBFF, 0x0FFFFFFFF, 0x0FFFFFF7F,
	0x0FFFFEFFF, 0x0FEFFFFFF, 0x0FFFFFFFF, 0x0FFFFFDFF, 0x0FFFEFFFF, 0x0FFFFFFFF, 0x0FFFFFFDF, 0x0FFFFF7FF,
	0x0DFFFFFFF, 0x0FFFFFFFF, 0x0FFFEFFFF, 0x0FFFFFFFF, 0x0FFFFFFDF, 0x0FFFFFBFF, 0x0F0F7FFFF, 0x000000000,
	0x00000807F, 0x000001F00, 0x000C07F00, 0x000F80700, 0x03E000000, 0x000000000, 0x00F000000, 0x0000000F0,
	0x000000000, 0x00000E01F, 0x00000FC01, 0x000FC0100, 0x0FFFF0300, 0x0000000C0, 0x000F0FF03, 0x0F8FFFF00,
	0x0FF010000, 0x080FFFFFF, 0x0FFFFFF1F, 0x0FF0000C0, 0x0070000FE, 0x01F0000E0, 0x000C00F80, 0x003F00300,
	0x01F0000F0, 0x000007EE0, 0x000FE0700, 0x0C03F0000, 0x00000E00F, 0x00000003F, 0x00000FCFF, 0x0E0FFFF03,
	0x003000000, 0x00000E0FF, 0x0F0FFFF1F, 0x01F000000, 0x03F0080FF, 0x0FCFFFFFF, 0x00000E007, 0x000FCC00F,
	0x0FF0F0000, 0x0F8070080, 0x03F7E0000, 0x03F0000C0, 0x00000C09F, 0x0FF3FF803, 0x000E0FFFF, 0x001000000,
	0x00000F0FF, 0x000001F00, 0x000F8FF03, 0x000FF1F00, 0x03E000000, 0x0FFFF0000, 0x07F0000FF, 0x0FF0F00FC,
	0x000C0FFFF, 0x00000FCFF, 0x00080FF0F, 0x000FE0100, 0x0FFFF0700, 0x0000000FE, 0x000FCFF1F, 0x0FFFFFF00,
	0x0FF0100C0, 0x080FFFFFF, 0x0FFFFFF3F, 0x0FF0700C0, 0x00700C0FF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x03F0000F0, 0x000007EC0, 0x000FE0700, 0x0C07F0000, 0x00000F00F, 0x00700803F, 0x00080FFFF, 0x0FFFFFF07,
	0x01F000080, 0x00000FCFF, 0x0FFFFFF1F, 0x07F000080, 0x03F00F0FF, 0x0FEFFFFFF, 0x00000F007, 0x000FEC01F,
	0x0FF0F0000, 0x0FC0700C0, 0x03FFE0000, 0x07F0000C0, 0x00000E09F, 0x0FF3FF807, 0x000E0FFFF, 0x007000000,
	0x00000F8FF, 0x000003F00, 0x000FEFF0F, 0x080FF7F00, 0x07E000000, 0x0FFFF0000, 0x0FF0100FF, 0x0FF0F00FF,
	0x003C0FFFF, 0x00000FFFF, 0x000E0FF1F, 0x000FE0300, 0x0FFFF0700, 0x0000080FF, 0x000FFFF7F, 0x0FFFFFF00,
	0x0FF0100F8, 0x080FFFFFF, 0x0FFFFFF3F, 0x0FF1F00C0, 0x00700F0FF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x07F0000F0, 0x000007E80, 0x000FE0700, 0x0C07F0000, 0x00000F00F, 0x01F00803F, 0x000C0FFFF, 0x0FFFFFF07,
	0x07F0000E0, 0x00000FFFF, 0x0FFFFFF1F, 0x0FF0100E0, 0x03F00FCFF, 0x0FEFFFFFF, 0x00000F007, 0x000FEC01F,
	0x0DF1F0000, 0x0FC0700C0, 0x01FFE0000, 0x0FF0000E0, 0x00000F00F, 0x0FF3FF007, 0x000E0FFFF, 0x00F000000,
	0x00000FCFF, 0x000007F00, 0x000FFFF1F, 0x0E0FFFF00, 0x0FE000000, 0x0FFFF0000, 0x0FF0300FF, 0x0FF0F80FF,
	0x007C0FFFF, 0x00080FFFF, 0x000F0FF7F, 0x000FF0300, 0x0FFFF0700, 0x00000C0FF, 0x080FFFFFF, 0x0FFFFFF00,
	0x0FF0100FC, 0x080FFFFFF, 0x0FFFFFF3F, 0x0FF3F00C0, 0x00700F8FF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0FF0000F0, 0x000007E00, 0x000FF0700, 0x0C07F0000, 0x00000F80F, 0x03F00803F, 0x000F0FFFF, 0x0FFFFFF07,
	0x0FF0000F0, 0x00080FFFF, 0x0FFFFFF1F, 0x0FF0300F0, 0x03F00FEFF, 0x0FEFFFFFF, 0x00000F007, 0x0007FC01F,
	0x0CF1F0000, 0x0FC0F00C0, 0x00FFC0000, 0x0FE0100F0, 0x00000F007, 0x0FF3FF00F, 0x000E0FFFF, 0x01F000000,
	0x00000FEFF, 0x00000FF00, 0x080FFFF3F, 0x0F0FFFF01, 0x0FE010000, 0x0FFFF0000, 0x0FF0700FF, 0x0FF0FC0FF,
	0x00FC0FFFF, 0x000C0FFFF, 0x000F8FFFF, 0x000FF0300, 0x0FFFF0700, 0x00100E0FF, 0x0E0FFFFFF, 0x0FFFFFF00,
	0x0FF0100FE, 0x080FFFFFF, 0x0FFFFFF3F, 0x0FFFF00C0, 0x00700FCFF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0FE0100F0, 0x000007E00, 0x000FF0700, 0x0C0FF0000, 0x00000FC0F, 0x07F00803F, 0x000F8FFFF, 0x0FFFFFF07,
	0x0FF0300F8, 0x000E0FFFF, 0x0FFFFFF1F, 0x0FF0700F8, 0x03F00FFFF, 0x0FEFFFFFF, 0x00000F007, 0x0007FC01F,
	0x0CF3F0000, 0x0FC0F00E0, 0x00FFC0100, 0x0FC0100F0, 0x00000F807, 0x0FF3FE01F, 0x000E0FFFF, 0x03F000000,
	0x00000FFE1, 0x00000FF01, 0x0C0FFE07F, 0x0F01FFE03, 0x0FE010000, 0x0FFFF0000, 0x0F80F00FF, 0x0FF0FE03F,
	0x01FC0FFFF, 0x001E07FF8, 0x000FC83FF, 0x000FF0700, 0x0FFFF0700, 0x00700F0FF, 0x0E0FFE1FF, 0x0FFFFFF00,
	0x0FF0180FF, 0x080FFFFFF, 0x0FFFFFF3F, 0x0FFFF01C0, 0x00700FEFF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0FC0300F0, 0x000007E00, 0x000FF0700, 0x0C0FF0000, 0x00000FC0F, 0x0FF01803F, 0x000FC7FF8, 0x0FFFFFF07,
	0x0FF0700FC, 0x000F0FFE3, 0x0FFFFFF1F, 0x0FF0F00FC, 0x03F80FF0F, 0x0FEFFFFFF, 0x00000F007, 0x0007FC01F,
	0x08F3F0000, 0x0FE0F00E0, 0x007FC0100, 0x0FC0300F8, 0x00000FC03, 0x0FF3FC01F, 0x000E0FFFF, 0x07F000000,
	0x000807F80, 0x00000FF03, 0x0C03F00FF, 0x0F803F807, 0x0FE030000, 0x0FFFF0100, 0x0E01F00FF, 0x0FF0FF00F,
	0x01F80FFFF, 0x001E00FC0, 0x000FE00FE, 0x080FF0700, 0x000F00700, 0x00F00F83F, 0x0F03F00FE, 0x00700FE00,
	0x0FC0180FF, 0x000000000, 0x00000803F, 0x080FF0300, 0x00700FF03, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0F80700F0, 0x000007E00, 0x080FF0700, 0x0C0FF0100, 0x00000FE0F, 0x0FF01803F, 0x000FE0780, 0x00F00F007,
	0x0FE0F00FE, 0x000F83F00, 0x00700C01F, 0x0F01F00FE, 0x000C0FF00, 0x000007E00, 0x00000F007, 0x0803FC01F,
	0x08F3F0000, 0x0FE1F00E0, 0x003FC0100, 0x0F80700FC, 0x00000FC01, 0x00000C03F, 0x000E01F00, 0x07F000000,
	0x000801F00, 0x00000FF07, 0x0E00F00FE, 0x0FC01F007, 0x0FE070000, 0x000F80100, 0x0C03F0000, 0x00000F007,
	0x03F803F00, 0x003F00780, 0x0007F00FC, 0x080DF0F00, 0x000F00700, 0x00F00F807, 0x0F80F00F8, 0x00000FE00,
	0x0FC01C07F, 0x000000000, 0x00000803F, 0x000FE0700, 0x00780FF00, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0F00F00F0, 0x000007E00, 0x080FF0700, 0x0C0FF0100, 0x00000FE0F, 0x0FE03803F, 0x000FF0100, 0x00100F007,
	0x0F81F00FE, 0x000FC0F00, 0x00100C01F, 0x0E01F00FE, 0x000C03F00, 0x000007E00, 0x00000F007, 0x0803FC01F,
	0x0077F0000, 0x0FE1F00E0, 0x003F80100, 0x0F00700FE, 0x00000FE01, 0x00000807F, 0x000C03F00, 0x0FE000000,
	0x000C01F00, 0x00100FF0F, 0x0E00700FC, 0x0FC00E00F, 0x0FE0F0000, 0x000F80100, 0x0803F0000, 0x00000F003,
	0x03F007F00, 0x003F00300, 0x0003F00F8, 0x0C0DF0F00, 0x000F00700, 0x01F00FC03, 0x0FC0700E0, 0x00000FE00,
	0x0FC01E03F, 0x000000000, 0x00000803F, 0x000F80700, 0x007807F00, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0E01F00F0, 0x000007E00, 0x080FF0700, 0x0C0FF0100, 0x00000FF0F, 0x0F807803F, 0x080FF0000, 0x00000F007,
	0x0E01F00FF, 0x000FC0300, 0x00000C01F, 0x0C03F00FF, 0x000E01F00, 0x000007E00, 0x00000F007, 0x0801FC01F,
	0x0077F0000, 0x07E1F00E0, 0x001F80300, 0x0E00F00FE, 0x00000FF00, 0x00000007F, 0x000807F00, 0x0FC000000,
	0x000C00F00, 0x00100FF3F, 0x0F00700F8, 0x0FC00C00F, 0x0FE0F0000, 0x000F80100, 0x0007F0000, 0x00000F803,
	0x03F00FE00, 0x007F80300, 0x0801F00F0, 0x0C0CF0F00, 0x000F00700, 0x03F00FC01, 0x0FC0300C0, 0x00000FE00,
	0x0FC01F00F, 0x000000000, 0x00000803F, 0x000F00F00, 0x007C03F00, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0C03F00F0, 0x000007E00, 0x0C0FF0700, 0x0C0FF0300, 0x00080FF0F, 0x0F00F803F, 0x0C03F0000, 0x00000F007,
	0x0C03F007F, 0x000FE0100, 0x00000C01F, 0x0803F007F, 0x000E00F00, 0x000007E00, 0x00000F007, 0x0C01FC01F,
	0x0077F0000, 0x03F1F00F0, 0x000F80300, 0x0E01F00FF, 0x000007F00, 0x0000000FF, 0x00000FF00, 0x0FC000000,
	0x000E00700, 0x00100FF7F, 0x0F00300F8, 0x0FE00C01F, 0x0FE1F0000, 0x000F80100, 0x0007E0000, 0x00000F801,
	0x07F00FC01, 0x007F80100, 0x0801F00E0, 0x0C0CF1F00, 0x000F00700, 0x03F00FC01, 0x0FE010080, 0x00000FE00,
	0x0FC01F00F, 0x000000000, 0x00000803F, 0x000E00F00, 0x007C01F00, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0807F00F0, 0x000007E00, 0x0C0EF0700, 0x0C0EF0300, 0x00080FF0F, 0x0E00F803F, 0x0C01F0000, 0x00000F007,
	0x0807F007F, 0x000FF0000, 0x00000C01F, 0x0003F007F, 0x000E00F00, 0x000007E00, 0x00000F007, 0x0C01FC01F,
	0x007FE0000, 0x03F3F00F0, 0x000F80300, 0x0C03F807F, 0x001807F00, 0x0000000FE, 0x00000FF00, 0x0F8010000,
	0x001E00700, 0x00300FFFF, 0x0F00300F8, 0x07E00C01F, 0x07E3F0000, 0x000F00300, 0x0007E0000, 0x00000F801,
	0x07E00F801, 0x007F80100, 0x0801F00E0, 0x0E08F1F00, 0x000F00700, 0x07F00FC00, 0x0FE000000, 0x00000FE00,
	0x0FC01F807, 0x000000000, 0x00000803F, 0x000C01F00, 0x007C01F00, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000FF00F0, 0x000007E00, 0x0C0EF0700, 0x0C0EF0300, 0x000C0FF0F, 0x0C01F803F, 0x0E01F0000, 0x00000F007,
	0x0007F003F, 0x0007F0000, 0x00000C01F, 0x0003F003F, 0x000F00700, 0x000007E00, 0x00000F007, 0x0C00FC01F,
	0x003FE0000, 0x03F3F00F0, 0x000F00300, 0x0803F807F, 0x001C03F00, 0x0000000FC, 0x00000FE01, 0x0F8010000,
	0x003E00700, 0x003003FFF, 0x0F00300F0, 0x07E00800F, 0x07E3F0000, 0x000F00300, 0x000FC0000, 0x000000000,
	0x07E00F803, 0x00FF80100, 0x0C00F00E0, 0x0E0873F00, 0x000F00700, 0x07F00FC00, 0x0FE000000, 0x00000FE00,
	0x0FC01F803, 0x000000000, 0x00000803F, 0x000C01F00, 0x007E00F00, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000FE01F0, 0x000007E00, 0x0E0E70700, 0x0C0CF0700, 0x000E0FF0F, 0x0C01F803F, 0x0E00F0000, 0x00000F007,
	0x000FE803F, 0x0807F0000, 0x00000C01F, 0x0003F003F, 0x000F00700, 0x000007E00, 0x00000F007, 0x0E00FC01F,
	0x003FC0100, 0x03F3E00F0, 0x000F00780, 0x0807FC03F, 0x003C01F00, 0x0000000FC, 0x00000FC03, 0x0F8010000,
	0x003F00700, 0x003003FFC, 0x0F00300F0, 0x0FE000000, 0x07E7E0000, 0x000F00300, 0x000FC0000, 0x000000000,
	0x07F00F007, 0x00FF80100, 0x0C00F00C0, 0x0F0873F00, 0x000F00700, 0x0FE00FC00, 0x07E000000, 0x00000FE00,
	0x0FC01F803, 0x000000000, 0x00000803F, 0x000803F00, 0x007E00F00, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000FC03F0, 0x000007E00, 0x0E0E70700, 0x0C0CF0700, 0x000E0DF0F, 0x0803F803F, 0x0F0070000, 0x00000F007,
	0x000FE803F, 0x0803F0000, 0x00000C01F, 0x0003F003F, 0x000F00700, 0x000007E00, 0x00000F007, 0x0E00FC01F,
	0x003FC0100, 0x01F7E00F8, 0x000F00780, 0x000FFE01F, 0x007E01F00, 0x0000000F8, 0x00000F807, 0x0F8010000,
	0x003F00300, 0x000003FF8, 0x0F0030010, 0x0FC000000, 0x07EFC0000, 0x000F00300, 0x000FC0000, 0x000000000,
	0x03F00E007, 0x00FF80300, 0x0C00F00C0, 0x0F0073F00, 0x000F00700, 0x0FE00FC00, 0x070000000, 0x00000FE00,
	0x0FC01FC01, 0x000000000, 0x00000803F, 0x000803F00, 0x007000700, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000F807F0, 0x000007E00, 0x0E0E70700, 0x0C0CF0700, 0x000F0CF0F, 0x0803F803F, 0x0F0070000, 0x00000F007,
	0x000FE803F, 0x0803F0000, 0x00000C01F, 0x0803F003F, 0x000000000, 0x000007E00, 0x00000F007, 0x0F007C01F,
	0x003FC0100, 0x01F7E00F8, 0x000E00780, 0x000FEE11F, 0x007F00F00, 0x0000000F0, 0x00000F00F, 0x0F0030000,
	0x003F00300, 0x000003FE0, 0x0F0030000, 0x0FC000000, 0x07EFC0100, 0x000F00300, 0x000FC0000, 0x000000000,
	0x03F00E00F, 0x00FF80300, 0x0C00F00C0, 0x0F0037F00, 0x000F00700, 0x0FE00FC01, 0x000000000, 0x00000FE00,
	0x0FC01FC01, 0x000000000, 0x00000803F, 0x000007F00, 0x007000000, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000F00FF0, 0x000007E00, 0x0F0E30700, 0x0C08F0F00, 0x000F8C70F, 0x0003F803F, 0x0F0070000, 0x00000F007,
	0x000FC813F, 0x0C01F0000, 0x00000C01F, 0x0803F007F, 0x000000000, 0x000007E00, 0x00000F007, 0x0F007C01F,
	0x001F80300, 0x01F7E00F8, 0x000E00780, 0x000FCF10F, 0x00FF00700, 0x0000000E0, 0x00000F00F, 0x0F0030000,
	0x003F00300, 0x000003F80, 0x0F0070000, 0x0FC010000, 0x07EF80100, 0x0FFE10700, 0x000F80100, 0x000000000,
	0x03F00C00F, 0x00FF00780, 0x0C00F00C0, 0x0F8037E00, 0x000F00700, 0x0FC00FC01, 0x000000000, 0x00000FE00,
	0x0FC01FC01, 0x000000000, 0x00000803F, 0x000007F00, 0x007000000, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000E01FF0, 0x000007E00, 0x0F0E30700, 0x0C08F0F00, 0x000F8C70F, 0x0007F803F, 0x0F8030000, 0x00000F007,
	0x000FC013F, 0x0C01F0000, 0x00000C01F, 0x0C03F007F, 0x000000000, 0x000007E00, 0x00000F007, 0x0F003C01F,
	0x001F80300, 0x01F7C00F8, 0x000E007C0, 0x000FCFB07, 0x00FF80700, 0x0000000E0, 0x00000E01F, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x0F0070000, 0x0F8030000, 0x07EF00300, 0x0FFE70700, 0x01FF801C0, 0x0000000F8,
	0x01F00C01F, 0x00FF007C0, 0x0C00F00C0, 0x0F8017E00, 0x000F00700, 0x0FC01F803, 0x000000000, 0x00000FE00,
	0x0FC01FC00, 0x000000000, 0x00000803F, 0x000007F00, 0x007000000, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000C03FF0, 0x000007E00, 0x0F0E30700, 0x0C08F0F00, 0x000FCC30F, 0x0007F803F, 0x0F8030000, 0x00000F007,
	0x000FC017F, 0x0C01F0000, 0x00000C01F, 0x0F01F00FF, 0x000000000, 0x000007E00, 0x00000F007, 0x0F803C01F,
	0x001F00300, 0x00FFC00F8, 0x000E00FC0, 0x000F8FF03, 0x01FFC0300, 0x0000000C0, 0x00000C03F, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x0E00F0000, 0x0F8070000, 0x07EE00700, 0x0FFFF0700, 0x07FF801F0, 0x0000000FE,
	0x01F00803F, 0x00FE01FE0, 0x0E00F00C0, 0x0FC01FE00, 0x000F00700, 0x0FC01F007, 0x000000000, 0x00000FE00,
	0x0FC01FC00, 0x000000000, 0x00000803F, 0x000007E00, 0x007000000, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000807FF0, 0x000007E00, 0x0F8E10700, 0x0C00F1F00, 0x000FEC10F, 0x0007F803F, 0x0F8030000, 0x00000F007,
	0x000F8017F, 0x0C00F0000, 0x00100C01F, 0x0FE1F00FE, 0x000000000, 0x000007E00, 0x00000F007, 0x0F803C01F,
	0x001F00700, 0x00FFC00FC, 0x000C00FC0, 0x000F0FF03, 0x03FFC0100, 0x000000080, 0x00000807F, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x0E01F0000, 0x0F03F0000, 0x07EE00700, 0x0FFFF0700, 0x0FFF801F8, 0x0000000FF,
	0x00F00803F, 0x00FC0FFFF, 0x0E00F00E0, 0x0FC01FC00, 0x000F00700, 0x0FC01F03F, 0x000000000, 0x00000FE00,
	0x0FC01FC00, 0x000000000, 0x00000803F, 0x000007E00, 0x007000000, 0x0FFFFFFFF, 0x000E00F80, 0x003F00700,
	0x00000FFF0, 0x000007E00, 0x0F8E10700, 0x0C00F1F00, 0x000FEC10F, 0x0007E803F, 0x0F8030000, 0x00000F007,
	0x000F801FF, 0x0C00F0000, 0x00300C01F, 0x0FF0F00FE, 0x0000000E0, 0x000007E00, 0x00000F007, 0x0F801C01F,
	0x000F00700, 0x00FF800FC, 0x000C00FC0, 0x000F0FF01, 0x03FFE0100, 0x000000080, 0x0000000FF, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x0C03F0000, 0x0E0FF0300, 0x07EC00F00, 0x0FFFF0700, 0x0FFFB01FC, 0x00000C0FF,
	0x00300007F, 0x00780FFFF, 0x0E01F00E0, 0x0FE00FC01, 0x0FFFF0700, 0x0FC01E0FF, 0x000000000, 0x00000FE00,
	0x0FF01FE00, 0x000FEFFFF, 0x0FEFFFF3F, 0x000007E00, 0x007000000, 0x0FFFFFFFF, 0x000E00F80, 0x003F00700,
	0x00080FFF1, 0x000007E00, 0x0FCE10700, 0x0C00F1F00, 0x000FFC00F, 0x0007E803F, 0x0F8010000, 0x00100F007,
	0x000F801FE, 0x0E00F0000, 0x07F00C01F, 0x0FF0700FC, 0x0000000FE, 0x000007E00, 0x00000F007, 0x0FC01C01F,
	0x000E00F00, 0x00FF801FC, 0x000C00FE0, 0x000E0FF00, 0x07FFF0000, 0x000000000, 0x0000000FF, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x0807F0000, 0x080FF0300, 0x07E801F00, 0x007FE0F00, 0x0FFFB01FE, 0x00000C0FF,
	0x00100007E, 0x00700FEFF, 0x0E01F00E0, 0x0FE00F801, 0x0FFFF0700, 0x0F80180FF, 0x000000000, 0x00000FE00,
	0x0FF01FE00, 0x000FEFFFF, 0x0FEFFFF3F, 0x00000FE00, 0x007000000, 0x0FFFFFFFF, 0x000E00F80, 0x003F00700,
	0x000C0FFF3, 0x000007E00, 0x0FCE00700, 0x0C00F3E00, 0x0807FC00F, 0x0007E803F, 0x0F8010000, 0x00700F007,
	0x000F803FE, 0x0E00F0000, 0x0FFFFFF1F, 0x0FF0300F8, 0x00000E0FF, 0x000007E00, 0x00000F007, 0x0FC00C01F,
	0x000E00F00, 0x007F801FC, 0x000C01FE0, 0x000C0FF00, 0x0FE7F0000, 0x001000000, 0x0000000FE, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x080FF0000, 0x0E0FF0300, 0x07E003F00, 0x000F80F00, 0x0F0FF01FF, 0x00000E03F,
	0x0030000FE, 0x00700FFFF, 0x0E03F00F0, 0x07E00F801, 0x0FFFF0700, 0x0F80180FF, 0x000000000, 0x00000FE00,
	0x0FF01FE00, 0x000FEFFFF, 0x0FEFFFF3F, 0x00000FE00, 0x007000000, 0x0FFFFFFFF, 0x000E00F80, 0x003F00700,
	0x000C0FFF7, 0x000007E00, 0x0FCE00700, 0x0C00F3E00, 0x0807FC00F, 0x0007E803F, 0x0F8010000, 0x0FFFFFF07,
	0x000F803FC, 0x0E00F0000, 0x0FFFFFF1F, 0x0FF0000F0, 0x00000F8FF, 0x000007E00, 0x00000F007, 0x0FC00C01F,
	0x000C00F00, 0x007F8017E, 0x000801FE0, 0x000807F00, 0x0FE7F0000, 0x003000000, 0x0000000FC, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x000FF0100, 0x0F0FF0300, 0x07E003F00, 0x000E00F00, 0x0C0FF017F, 0x00000F00F,
	0x00F0000FE, 0x003C0FFFF, 0x0E07F00F8, 0x07F00F803, 0x0FFFF0700, 0x0F801E0FF, 0x000000000, 0x00000FE00,
	0x0FF01FE00, 0x000FEFFFF, 0x0FEFFFF3F, 0x00000FE00, 0x007F0FFFF, 0x0FFFFFFFF, 0x000E00F80, 0x003F00700,
	0x000E0FFFF, 0x000007E00, 0x0FEE00700, 0x0C00F7E00, 0x0C03FC00F, 0x0007E803F, 0x0F8010000, 0x0FFFFFF07,
	0x000F803F8, 0x0E00F0000, 0x0FFFFFF1F, 0x03F0000E0, 0x00000FEFF, 0x000007E00, 0x00000F007, 0x0FE00C01F,
	0x000C01F00, 0x007F0017E, 0x000801FF0, 0x000C07F00, 0x0FC3F0000, 0x007000000, 0x0000000F8, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x000FE0300, 0x0FCFF0300, 0x07E007E00, 0x000C00F00, 0x080FF813F, 0x00100F007,
	0x01F0000FC, 0x003E0FFFC, 0x0E0FF00FC, 0x07F00F003, 0x0FFFF0700, 0x0F801F0FF, 0x000000000, 0x00000FE00,
	0x0FF01FE00, 0x000FEFFFF, 0x0FEFFFF3F, 0x00000FE00, 0x007F0FFFF, 0x0FFFFFFFF, 0x000E00F80, 0x003F00700,
	0x000F0EFFF, 0x000007E00, 0x07EE00700, 0x0C00F7C00, 0x0E01FC00F, 0x0007E803F, 0x0F8010000, 0x0FFFFFF07,
	0x000F803F0, 0x0E00F0000, 0x0FFFFFF1F, 0x007000080, 0x00000FFFF, 0x000007E00, 0x00000F007, 0x07E00C01F,
	0x000C01F00, 0x003F0037E, 0x000801FF0, 0x000C0FF00, 0x0F81F0000, 0x00F000000, 0x0000000F0, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x000FC0700, 0x0FE030000, 0x07E00FC00, 0x000C00300, 0x000FF811F, 0x00100F803,
	0x03F0000FC, 0x001F00FC0, 0x0E0FF03FF, 0x03F00F007, 0x0FFFF0780, 0x0F801FCFF, 0x000000000, 0x00000FE00,
	0x0FF01FE00, 0x000FEFFFF, 0x0FEFFFF3F, 0x00000FE00, 0x007F0FFFF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000F8C7FF, 0x000007E00, 0x07EE00700, 0x0C00F7C00, 0x0E01FC00F, 0x0007E803F, 0x0F8010000, 0x0FFFFFF07,
	0x000F803E0, 0x0E00F0000, 0x0FCFFFF1F, 0x000000000, 0x000C0FFFF, 0x000007E00, 0x00000F007, 0x07E00C01F,
	0x000801F00, 0x003F0037E, 0x000803FF0, 0x000E0FF01, 0x0F81F0000, 0x00F000000, 0x0000000E0, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x000F80F00, 0x0FE000000, 0x07E00FC01, 0x000000000, 0x000FEC11F, 0x00100F801,
	0x03F0000F8, 0x001F80780, 0x0E0F7FFFF, 0x03F00F007, 0x000F00780, 0x0FC01FC07, 0x000000000, 0x00000FE00,
	0x0FC01FE00, 0x000000000, 0x00000803F, 0x00000FE00, 0x007F0FFFF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000F887FF, 0x000007E00, 0x03FE00700, 0x0C00FFC00, 0x0F00FC00F, 0x0007E803F, 0x0F8010000, 0x0FFFFFF07,
	0x000F80380, 0x0E00F0000, 0x0F81FC01F, 0x000000000, 0x000C0FF0F, 0x000007E00, 0x00000F007, 0x07F00C01F,
	0x000803F00, 0x003F0033E, 0x000003FF0, 0x000F0FF01, 0x0F00F0000, 0x01F000000, 0x0000000E0, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x000F01F00, 0x07F000000, 0x07E00F801, 0x000000000, 0x000FEC11F, 0x00300F801,
	0x07F0000F8, 0x000F80300, 0x0C0E7FFFF, 0x01F00E007, 0x000F00780, 0x0FC01FE01, 0x000000000, 0x00000FE00,
	0x0FC01FC00, 0x000000000, 0x00000803F, 0x000007E00, 0x007F0FFFF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000FC03FF, 0x000007E00, 0x03FE00700, 0x0C00FF800, 0x0F807C00F, 0x0007E803F, 0x0F8010000, 0x0FCFFFF07,
	0x000F80100, 0x0C00F0000, 0x0FC03C01F, 0x000000000, 0x000E0FF00, 0x000007E00, 0x00000F007, 0x03F00C01F,
	0x000003F00, 0x003E0073F, 0x000003FF8, 0x000F8FF03, 0x0E0070000, 0x03F000000, 0x0000000C0, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x000E03F00, 0x03F000000, 0x07E00F003, 0x000000000, 0x000FCC10F, 0x00300FC01,
	0x07E0000F0, 0x000FC0100, 0x0C0C7FF7F, 0x0FFFFFF0F, 0x000F007C0, 0x0FC01FF00, 0x000000000, 0x00000FE00,
	0x0FC01FC00, 0x000000000, 0x00000803F, 0x000007E00, 0x007F0FFFF, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000FE01FE, 0x000007E00, 0x03FE00700, 0x0C00FF800, 0x0F807C00F, 0x0007E803F, 0x0F8030000, 0x00000F007,
	0x000F80100, 0x0C00F0000, 0x0FE01C01F, 0x000000000, 0x000F01F00, 0x000007E00, 0x00000F007, 0x03F00C01F,
	0x000007F80, 0x001E0073F, 0x000003FF8, 0x000F8FF07, 0x0E0070000, 0x07F000000, 0x000000080, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x000C07F00, 0x03F000000, 0x07E00E087, 0x000000000, 0x000FCC10F, 0x00300FC00,
	0x0FE0000F0, 0x000FC0100, 0x0C08FFF1F, 0x0FFFFFF0F, 0x000F007C0, 0x0FC017F00, 0x000000000, 0x00000FE00,
	0x0FC01FC00, 0x000000000, 0x00000803F, 0x000007F00, 0x007F00700, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000FF01FC, 0x000007E00, 0x01FE00700, 0x0C00FF081, 0x0FC03C00F, 0x0007F803F, 0x0F8030000, 0x00000F007,
	0x000FC0100, 0x0C01F0000, 0x0FF00C01F, 0x000000000, 0x000F00F00, 0x000007E00, 0x00000F007, 0x01F00C01F,
	0x000007F80, 0x001E0073F, 0x000007EF8, 0x000FCF30F, 0x0E0070000, 0x0FF000000, 0x000000000, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x00080FF00, 0x01F000000, 0x0FFFFFF87, 0x0000000F8, 0x000FCC10F, 0x00700FC00,
	0x0FC0000F0, 0x000FC0000, 0x0C00FFE07, 0x0FFFFFF0F, 0x000F007E0, 0x0FC007F00, 0x000000000, 0x00000FE00,
	0x0FC01FC01, 0x000000000, 0x00000803F, 0x000007F00, 0x007F00700, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x000FF00F8, 0x000007E00, 0x01FE00700, 0x0C00FF081, 0x0FE01C00F, 0x0007F803F, 0x0F8030000, 0x00000F007,
	0x000FC0100, 0x0C01F0000, 0x07F00C01F, 0x000000080, 0x000F80700, 0x000007E00, 0x00000F007, 0x01F00C01F,
	0x000007E80, 0x001C0071F, 0x000007EF8, 0x000FEF10F, 0x0E0070000, 0x0FE000000, 0x000000000, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x00000FF03, 0x01F000000, 0x0FFFFFF87, 0x0000000F8, 0x000FCC10F, 0x00700FC00,
	0x0FC0000E0, 0x000FC0000, 0x0C00F0000, 0x0FFFFFF1F, 0x000F007E0, 0x0FE803F00, 0x03E000000, 0x00000FE00,
	0x0FC01FC01, 0x000000000, 0x00000803F, 0x000007F00, 0x007F00700, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0807F00F0, 0x000007E00, 0x01FE00700, 0x0C00FF081, 0x0FE01C00F, 0x0007F803F, 0x0F8030000, 0x00000F007,
	0x000FC0100, 0x0C01F0000, 0x03F00C01F, 0x0003C00C0, 0x000F80300, 0x000007E00, 0x00000F007, 0x01F00C01F,
	0x00000FEC0, 0x001C08F1F, 0x000007EFC, 0x000FEE11F, 0x0E0070000, 0x0FE010000, 0x000000000, 0x0F0030000,
	0x000F00300, 0x000003F00, 0x00000FC07, 0x01F000000, 0x0FFFFFF87, 0x0000000F8, 0x000FCC10F, 0x00700FC00,
	0x0FC0000E0, 0x000FC0000, 0x0C00F0000, 0x0FFFFFF1F, 0x000F007E0, 0x0FE803F00, 0x07F000000, 0x00000FE80,
	0x0FC01FC01, 0x000000000, 0x00000803F, 0x000003F00, 0x007F00700, 0x01F0000E0, 0x000E00F80, 0x003F00700,
	0x0C03F00F0, 0x000007E00, 0x00FE00700, 0x0C00FE0C3, 0x0FF00C00F, 0x0803F803F, 0x0F0070000, 0x00000F007,
	0x000FC0000, 0x0801F0000, 0x01F00C01F, 0x000FC00E0, 0x000F80300, 0x000007E00, 0x00000F007, 0x00F00C01F,
	0x00000FCC0, 0x000C08F1F, 0x000007EFC, 0x000FFC03F, 0x0E0070000, 0x0FC030000, 0x000000000, 0x0F8010000,
	0x000F00300, 0x000003F00, 0x00000F80F, 0x01F000000, 0x0FFFFFF87, 0x0000000F8, 0x000FCC10F, 0x00F00FC00,
	0x0FC0000E0, 0x000FC0000, 0x0C00F0000, 0x00700803F, 0x000F007F0, 0x0FE803F00, 0x07F000000, 0x00000FE00,
	0x0FC01F803, 0x000000000, 0x00000803F, 0x000803F00, 0x007F00700, 0x01F0000E0, 0x01FE00F80, 0x003F00700,
	0x0E03F00F0, 0x000007E00, 0x00FE00700, 0x0C00FE0C3, 0x07F00C00F, 0x0803F80BF, 0x0F0070000, 0x00000F007,
	0x000FE0000, 0x0803F0000, 0x00F00C01F, 0x000FC00E0, 0x000F80300, 0x000007E00, 0x00000F007, 0x00F00C01F,
	0x00000FCC1, 0x000C08F1F, 0x000007CFC, 0x0807F807F, 0x0E0070000, 0x0F8070000, 0x000000000, 0x0F8010000,
	0x000F00700, 0x000003F00, 0x00000F01F, 0x01F00801F, 0x0FFFFFF87, 0x000800FF8, 0x000FCC00F, 0x00F00FC00,
	0x0FC0000C0, 0x000FC0000, 0x0C01F0000, 0x00700803F, 0x000F007F0, 0x07F803F00, 0x07F000000, 0x00000FE00,
	0x0FC01F803, 0x000000000, 0x00000803F, 0x000803F00, 0x007F00700, 0x01F0000E0, 0x03FE00F80, 0x003F00700,
	0x0E01F00F0, 0x000007E00, 0x00FE00700, 0x0C00FE0C3, 0x07F00C00F, 0x0C01F80BF, 0x0F00F0000, 0x00000F007,
	0x000FE0000, 0x0803F8003, 0x00F00C01F, 0x000FE00F0, 0x000F80300, 0x000007E00, 0x00000F003, 0x00700801F,
	0x00000FCE1, 0x000808F0F, 0x00000FCFC, 0x0807F807F, 0x0E0070000, 0x0F00F0000, 0x000000000, 0x0F8010000,
	0x000E00700, 0x000003F00, 0x00000E03F, 0x01F00801F, 0x0FFFFFF87, 0x000801FF8, 0x000FCC00F, 0x00F00FC01,
	0x0FC0000C0, 0x000FC0000, 0x0801F0000, 0x00300003F, 0x000F007F8, 0x07F803F00, 0x0FF000000, 0x00000FE00,
	0x0FC01F807, 0x000000000, 0x00000803F, 0x000C01F00, 0x007F00700, 0x01F0000E0, 0x03FE00F80, 0x003F00700,
	0x0F00F00F0, 0x000007E00, 0x007E00700, 0x0C00FC0E7, 0x03F00C00F, 0x0C01F80FF, 0x0E00F0000, 0x00000F007,
	0x0007F0000, 0x0007FE007, 0x00700C01F, 0x000FE00F8, 0x000F80300, 0x000007E00, 0x00000F803, 0x00700803F,
	0x00000F8E1, 0x00080DF0F, 0x00000FCFC, 0x0C03F00FF, 0x0E0070000, 0x0E00F0000, 0x000000000, 0x0F8010000,
	0x000E00700, 0x000003F00, 0x00000C03F, 0x01F00801F, 0x07E000080, 0x000C01F00, 0x000FE801F, 0x00F00F801,
	0x0FC0000C0, 0x007FC0000, 0x0801F00E0, 0x00300007F, 0x000F007F8, 0x03F003F00, 0x0FE000080, 0x00000FE00,
	0x0FC01F007, 0x000000000, 0x00000803F, 0x000E01F00, 0x007F00700, 0x01F0000E0, 0x03FE00F80, 0x003F00780,
	0x0F80F00F0, 0x000007E00, 0x007E00700, 0x0C00FC0E7, 0x01F00C00F, 0x0E00F80FF, 0x0E01F0000, 0x00000F007,
	0x0807F0000, 0x000FFF807, 0x00300C01F, 0x0007E00F8, 0x000F80300, 0x000007E00, 0x00000F803, 0x00700803F,
	0x00000F8E3, 0x00080DF0F, 0x00100FC7E, 0x0E01F00FE, 0x0E0070000, 0x0E01F0000, 0x000000000, 0x0FC000000,
	0x000E00700, 0x000003F00, 0x00000807F, 0x03F00C01F, 0x07E000080, 0x000C00F00, 0x000FE801F, 0x01F00F801,
	0x0FE0000C0, 0x007FC0000, 0x0803F00E0, 0x00300007F, 0x000F007F8, 0x03F007F00, 0x0FE0100C0, 0x00000FE00,
	0x0FC01F00F, 0x000000000, 0x00000803F, 0x000F00F00, 0x007F00700, 0x01F0000E0, 0x01FE00F80, 0x003F00780,
	0x0F80700F0, 0x000007E00, 0x007E00700, 0x0C00FC0E7, 0x01F00C00F, 0x0F00F80FF, 0x0C03F0000, 0x00000F007,
	0x0803F0000, 0x000FEFC07, 0x00300C01F, 0x0007F00FC, 0x000F00300, 0x000007E00, 0x00000FC03, 0x00300807F,
	0x00000F8F3, 0x00080DF0F, 0x00100F87E, 0x0F00F00FE, 0x0E0070000, 0x0C03F0000, 0x000000000, 0x0FC000000,
	0x000C00F00, 0x000003F00, 0x0000000FF, 0x03F00C01F, 0x07E000000, 0x000E00F00, 0x0007F803F, 0x01F00F803,
	0x07E000080, 0x007FC0100, 0x0003F00F0, 0x0010000FE, 0x000F007FC, 0x01F00FF00, 0x0FC0300E0, 0x00000FE00,
	0x0FC01E01F, 0x000000000, 0x00000803F, 0x000F80F00, 0x007F00F00, 0x01F0000E0, 0x01FE00F80, 0x003E00780,
	0x0FC0300F0, 0x000007E00, 0x003E00700, 0x0C00F80FF, 0x00F00C00F, 0x0F80780FF, 0x0807F0000, 0x00000F007,
	0x0E03F0000, 0x000FEFF07, 0x00100C01F, 0x0807F00FE, 0x000F00700, 0x000007E00, 0x00000FC01, 0x00300007F,
	0x00000F0F3, 0x00000DF07, 0x00300F87F, 0x0F00F00FC, 0x0E0070000, 0x0807F0000, 0x000000000, 0x0FE000000,
	0x000C01F00, 0x000003F00, 0x0000000FE, 0x07F00E00F, 0x07E000000, 0x000E00F00, 0x0007F003F, 0x01F00F003,
	0x07F000080, 0x007F80300, 0x0007F00F0, 0x0010000FE, 0x000F007FC, 0x01F00FE01, 0x0FC0700F0, 0x00000FE00,
	0x0FC01C03F, 0x000000000, 0x00000803F, 0x000FC0700, 0x007F03F00, 0x01F0000E0, 0x01FE00F80, 0x003E00FC0,
	0x0FE0100F0, 0x000007E00, 0x003E00700, 0x0C00F80FF, 0x00700C00F, 0x0FC0780FF, 0x080FF0000, 0x00000F007,
	0x0F01F0000, 0x000FCFF01, 0x00100C01F, 0x0C03F00FE, 0x000F00F00, 0x000007E00, 0x00000FE01, 0x0010000FF,
	0x00000F0F7, 0x00000FF07, 0x00700F83F, 0x0F80700F8, 0x0E0070000, 0x000FF0000, 0x000000000, 0x07F000000,
	0x000801F00, 0x001003F00, 0x0000000FC, 0x0FE00F00F, 0x07E000000, 0x000F00700, 0x0803F007F, 0x01F00F007,
	0x03F000080, 0x003F80380, 0x000FE00F8, 0x0010000FE, 0x000F007FE, 0x00F00FE07, 0x0F81F00FC, 0x00100FE00,
	0x0FC01C0FF, 0x000000000, 0x00000803F, 0x000FF0300, 0x007F0FF01, 0x01F0000E0, 0x01FE00F80, 0x003E01FC0,
	0x0FF0100F0, 0x000007E00, 0x003E00700, 0x0C00F80FF, 0x00700C00F, 0x0FF0380FF, 0x000FF0300, 0x00000F007,
	0x0F80F0000, 0x000F83F00, 0x00000C01F, 0x0F03F00FF, 0x000E03F00, 0x000007E00, 0x00380FF00, 0x0010000FE,
	0x00000E0FF, 0x00000FF07, 0x00F00F03F, 0x0FC0300F0, 0x0E0070000, 0x000FE0000, 0x000000000, 0x07F000000,
	0x000807F80, 0x003003F00, 0x0F0FFFFFF, 0x0FE01F807, 0x07E000000, 0x001F80700, 0x0C01F00FE, 0x01F00E00F,
	0x03F000080, 0x003F00FC0, 0x001FC01FC, 0x0000000FC, 0x0FFFF07FE, 0x00700FCFF, 0x0F07F00FF, 0x0FFFFFF00,
	0x0FF0180FF, 0x0C0FFFFFF, 0x00000803F, 0x0E0FF0100, 0x007C0FF0F, 0x01F0000E0, 0x00FE00F80, 0x003C03FF0,
	0x0FF0000F0, 0x0FFFF7F00, 0x001E007FC, 0x0C00F00FF, 0x00300C00F, 0x0FF0180FF, 0x000FE1FE0, 0x00000F007,
	0x0FF070000, 0x000F07F00, 0x00000C01F, 0x0FE1F007F, 0x000C0FF00, 0x000007E00, 0x01FF0FF00, 0x0010000FE,
	0x00000E0FF, 0x00000FE03, 0x00F00F03F, 0x0FC0300F0, 0x0E0070000, 0x0FFFF0100, 0x000F0FFFF, 0x03F000000,
	0x00000FFE1, 0x003003F00, 0x0F0FFFFFF, 0x0FC0FFE03, 0x07E000000, 0x007FF0300, 0x0F81F00FC, 0x03F00E07F,
	0x01F000080, 0x001E03FF8, 0x001FC07FF, 0x0000000FC, 0x0FFFF07FE, 0x00300F8FF, 0x0E0FFFFFF, 0x0FFFFFF00,
	0x0FF0100FF, 0x0C0FFFFFF, 0x00000803F, 0x0FFFF0000, 0x00780FFFF, 0x01F0000E0, 0x00FE00F80, 0x003C0FFFF,
	0x07F0000F0, 0x0FFFF7F80, 0x001E007FE, 0x0C00F00FF, 0x00100C00F, 0x0FF0080FF, 0x000FCFFFF, 0x00000F007,
	0x0FF030000, 0x000F8FFFF, 0x00000C01F, 0x0FF0F807F, 0x000C0FFFF, 0x000007E00, 0x0FFFF7F00, 0x0000000FC,
	0x00000E0FF, 0x00000FE03, 0x01F00F03F, 0x0FE0100E0, 0x0E0070000, 0x0FFFF0100, 0x000F0FFFF, 0x01F000000,
	0x00000FEFF, 0x003003F00, 0x0F0FFFFFF, 0x0F8FFFF03, 0x07E000000, 0x0FFFF0100, 0x0FF0F00F8, 0x03F00C0FF,
	0x00F000080, 0x000E0FFFF, 0x001F8FFFF, 0x0000000F8, 0x0FFFF07FF, 0x00100F8FF, 0x0C0FFFFFF, 0x0FFFFFF00,
	0x0FF0100FE, 0x0C0FFFFFF, 0x00000803F, 0x0FF7F0000, 0x00700FEFF, 0x01F0000E0, 0x007E00F80, 0x00380FFFF,
	0x07F0000F0, 0x0FFFF7FC0, 0x001E007FE, 0x0C00F00FF, 0x00100C00F, 0x07F0080FF, 0x000F8FFFF, 0x00000F007,
	0x0FF010000, 0x000FCFFFF, 0x00000C01F, 0x0FF07C03F, 0x00080FFFF, 0x000007E00, 0x0FFFF3F00, 0x0000000F8,
	0x00000C0FF, 0x00000FE03, 0x03F00F01F, 0x0FF0000C0, 0x0E0070000, 0x0FFFF0100, 0x000F0FFFF, 0x00F000000,
	0x00000FCFF, 0x003003F00, 0x0F0FFFFFF, 0x0F0FFFF00, 0x07E000000, 0x0FFFF0000, 0x0FF0700F0, 0x03F0080FF,
	0x007000000, 0x00080FFFF, 0x003F0FF7F, 0x0000000F8, 0x0FFFF077F, 0x00000E0FF, 0x080FFFFFF, 0x0FFFFFF00,
	0x0FF0100F8, 0x0C0FFFFFF, 0x00000803F, 0x0FF1F0000, 0x00700FCFF, 0x01F0000E0, 0x003E00F80, 0x00300FFFF,
	0x03F0000F0, 0x0FFFF7FE0, 0x000E007FE, 0x0C00F00FE, 0x00000C00F, 0x01F0080FF, 0x000E0FFFF, 0x00000F007,
	0x0FF000000, 0x000FFFFFF, 0x00000C01F, 0x0FF03C01F, 0x00000FEFF, 0x000007E00, 0x0FFFF1F00, 0x0000000F0,
	0x00000C0FF, 0x00000FE03, 0x07F00E01F, 0x0FF0000C0, 0x0E0070000, 0x0FFFF0100, 0x000F0FFFF, 0x007000000,
	0x00000F8FF, 0x007003F00, 0x0F0FFFFFF, 0x0C0FF7F00, 0x07E000000, 0x0FF7F0000, 0x0FF0100E0, 0x03F0000FE,
	0x003000000, 0x00000FFFF, 0x003C0FF3F, 0x0000000F8, 0x0FFFF877F, 0x0000080FF, 0x000FEFF3F, 0x0FFFFFF00,
	0x0FF0100E0, 0x0C0FFFFFF, 0x00000803F, 0x0FF070000, 0x00700F0FF, 0x01F0000E0, 0x001E00F80, 0x00300FEFF,
	0x01F0000F0, 0x0FFFF7FE0, 0x000E007FE, 0x0C00F00FE, 0x00000C00F, 0x00700807F, 0x00080FFFF, 0x00000F007,
	0x03F000000, 0x0C0FFFEFF, 0x00000C01F, 0x0FF00E01F, 0x00000FCFF, 0x000007E00, 0x0FFFF0700, 0x0000000C0,
	0x00000807F, 0x00000FC01, 0x07F00E01F, 0x07F000080, 0x0E0070080, 0x0FFFF0100, 0x000F0FFFF, 0x001000000,
	0x00000F0FF, 0x007003F00, 0x0F0FFFFFF, 0x080FF1F00, 0x07E000000, 0x0FF1F0000, 0x0FF000080, 0x03F0000FC,
	0x000000000, 0x00000FCFF, 0x00780FF1F, 0x0000000F0, 0x0FFFF873F, 0x0000000FC, 0x000F8FF0F, 0x0FFFFFF00,
	0x0FF010000, 0x0C0FFFFFF, 0x00000803F, 0x0FF010000, 0x0070080FF, 0x01F0000E0, 0x000E00F80, 0x00300F8FF,
	0x00F0000F0, 0x0FFFF7FF0, 0x000E007FE, 0x0C00F00FE, 0x00000C00F, 0x00100807F, 0x00000FEFF, 0x00000F007,
	0x00F000000, 0x0E03FF8FF, 0x00000C01F, 0x03F00F00F, 0x00000F0FF, 0x000007E00, 0x0FFFF0100, 0x000000000,
	0x00000807F, 0x00000FC01, 0x0FF00E01F, 0x03F000000, 0x0E00700C0, 0x0FFFF0100, 0x000F0FFFF, 0x000000000,
	0x00000807F, 0x000000000, 0x000000000, 0x000FC0700, 0x000000000, 0x0FC030000, 0x01F000000, 0x0000000E0,
	0x000000000, 0x00000E01F, 0x00000FC03, 0x000000000, 0x000000000, 0x000000000, 0x000C0FF00, 0x000000000,
	0x000000000, 0x000000000, 0x000000000, 0x01F000000, 0x0000000F8, 0x000000000, 0x000000000, 0x00000E01F,
	0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x00000F03F, 0x000000000,
	0x000000000, 0x0E01F80FF, 0x000000000, 0x003000000, 0x0000000FF, 0x000000000, 0x0F83F0000, 0x000000000,
	0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000,
	0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000,
	0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000,
	0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000,
	0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000,
	0x000000000, 0x0E0070000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000,
	0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000, 0x000000000
};

char pbuf[1024 * ch_hgt];

char *ttfofs;
int ch_start, ch_wid;

//locating bit
unsigned char p_ttf(int i, int j)
{
	unsigned char b8 = ttfofs[j * tlength + i / 8];
	int b = (b8 >> (i & 7)) & 1;
	return b * 255;
	
	if ((ttfofs[j * tlength + (i >> 3)] & 128) >> (i & 7))
		return 255;
	else
		return 0;

}

//getting start & width
char get_char_info(char ch)
{
	char cf = 0;
	int i, j, n;
	char *p;
	char font_table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	//finding this char in font table
	p = font_table;
	n = 1;
	while (*p != 0 && *p != ch)
	{
		p++;
		n++;
	}

	// if found
	if (*p != 0)
	{
		//define character offset
		i = 0;
		for (j = 0; j < n; j++)
		{
			while (p_ttf(i, 0) == 255)
				i++;

			i++;
		}
		ch_start = i;

//	printf ("ch: %c, n: %d, ch_start: %d\n", ch, n, ch_start);


		//define character width
		i++;

		while (p_ttf(i, 0) == 255)
			i++;

		ch_wid = i - ch_start;
		cf = 1;

		//success, else

//		printf("char %c width %d start %d\n", *p, ch_wid, ch_start);
	}
	if (cf == 0)
		ch_wid = ch_wid_def;
	return (cf);
}

//pasting
void paste_pbuf(int x0, int y0, int len, int hgt, int u0, int v0, int _len, int _hgt, int color)
{
	int i, j, u, v, du, dv;
	char c;

	if (hgt > 0 && len > 0)
	{			//antialias
		du = (1 << 16) * _hgt / hgt;
		dv = (1 << 16) * _len / len;
		u = u0;
		for (j = y0; j < y0 + hgt; j++)
		{
			v = v0;
			for (i = x0; i < x0 + len; i++)
			{
				c = pbuf[(u >> 16 << 10) + (v >> 16)];
				if (c != 0)
					putpixel(i, j, color);
				v += dv;
			}
			u += du;
		}

	}
}

//filling
int text2buf(char *s, unsigned char color)
{
	int xp, yp, i, j, k, t, x, y, cf;
	int b_ofs;
	char b;
	ttfofs = (char *)(ttf) + 62;
	xp = 0;
	yp = 0;
	while (*s != 0)
	{
		x = xp;
		y = yp;
		cf = get_char_info(*s);
		s++;
		xp += ch_wid;
		{
			for (j = 0; j < ch_hgt; j++)
			{
				b_ofs = (j + 1) * tlength;
				for (i = 0; i < ch_wid; i++)
				{
					k = i + x;
					if (k > 0 && k < 1024)
					{
						t = i + ch_start;
						if (!cf)
							b = 0;
						else
							b = (ttfofs[b_ofs + (t >> 3)] & 128) >> (t & 7);
						if (b != 0)
							b = color;
						pbuf[k + (j << 10)] = b;
					}
				}
			}
		}
	}
	return (xp);
}

//general
void outtext(int x0, int y0, char *s, float size, int color, char position)
{
	int len, hgt;
	len = text2buf(s, color);
	hgt = ch_hgt;
	if (position == PM_CENTERED)
	{
		x0 -= len * int(size / 2);
		y0 -= hgt * int(size / 2);
	}

	paste_pbuf(x0, y0, int(len * size), int(hgt * size), 0, 0, len, hgt, color);
}

