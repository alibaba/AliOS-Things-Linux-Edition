#ifndef __MT_REG_H__
#define __MT_REG_H__

#define REG_READ32(x)  (*(volatile u32*)((x)))
#define REG_WRITE32(x,val) (*(volatile u32*)((x)) = (u32)(val))
#define REG_UPDATE32(x,val,mask) do {                     \
        u32 newval;                                           \
        newval = *(volatile u32*) ((x));                      \
        newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
        *(volatile u32*)((x)) = newval;                       \
} while(0)



#endif // __MT_REG_H__

