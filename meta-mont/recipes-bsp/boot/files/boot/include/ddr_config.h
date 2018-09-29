

// 以下这两个宏，用于选择DDR3或者DDR2，同时只可以有一个宏有效
//#define CONFIG_USE_DDR3  // 当选择DDR3时, enable这个宏
//#define CONFIG_USE_DDR2  // 当选择DDR2时, enable这个宏

// 以下这个宏，用来配置选择是FPGA还是Silicon IC
#define CONFIG_BOARD_WITH_CHIP  // Silicon IC

// 以下这些宏，用来配置当前DDR的工作频率
#ifdef CONFIG_USE_DDR2
// 以下这两个宏，用于用来配置当前DDR的工作频率，同时只可以有一个宏有效
#define CONFIG_FREQ396  // 如果定义这个宏，DDR2的时钟频率是396MHz
#undef  CONFIG_FREQ396  // 如果未定义这个宏，DDR2的时钟频率是531MHz
#endif
#ifdef CONFIG_USE_DDR3
// 以下这两个宏，用于用来配置当前DDR的工作频率，同时只可以有一个宏有效
//#define CONFIG_FREQ666  // 如果定义这个宏，DDR3的时钟频率是666MHz
#undef  CONFIG_FREQ666  // 如果未定义这个宏，DDR3的时钟频率是528MHz
#endif

#define DDR_SIZE_INFO_ADDR 0xBF005510UL     // unit: MB
