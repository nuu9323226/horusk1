static inline void udelay(volatile int us) 

{ 

    volatile int i; 

    while(us--) 

        for(i = 30; i > 0; i--); 

} 

 

static inline void mdelay(volatile int ms) 

{ 

    while(ms--) { 

        udelay(700); 

    } 

}  
