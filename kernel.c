#include <stdint.h>
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define BLUE 0xFF000080
#define RED   0xFFFF0000
#define YELLOW 0xFFFFFF00

struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
  
    // Wichtig für Grafik
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    // Framebuffer Felder (Multiboot 1 / 2)
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
};

typedef struct {
    uint32_t* address;
    uint32_t width;
    uint32_t height;
    uint8_t bpp; 
} framebuffer_info_t;

//grafik - Vektorzeichnungen
void putPixel(struct multiboot_info* mbi, int x, int y, uint32_t color) {


    uint32_t* fb_ptr = (uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    uint32_t pitch_in_pixels = mbi->framebuffer_pitch / 4;
    
    if (x >= 0 && x < (int)mbi->framebuffer_width && y >= 0 && y < (int)mbi->framebuffer_height) {
        fb_ptr[y * pitch_in_pixels + x] = color;
    }
}

//Bresenham Algorhytmus
void drawLine(struct multiboot_info* mbi ,int x0,int y0,int x1,int y1,uint32_t color){
    
    int dx = ABS(x1 - x0); //distant x
    int dy = -ABS(y1 - y0); //distant y
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int error = dx + dy;
    while(1){

        putPixel(mbi,x0,y0,color);

        if(x0 == x1 && y0 == y1){
            break;
        }

        int error2 = 2 * error;

        if(error2 >= dy){
            if(x0 == x1) break;

            error += dy;
            x0 += sx;

        }

        if(error2 <= dx){
            if(y0 == y1) break;

            error += dx;
            y0 += sy;
        }
    }
}

void drawRect(struct multiboot_info* mbi ,int x0,int y0,int x1 ,int y1,int x2 , int y2,int x3, int y3,uint32_t color,int drawMode){
    while(1){
        //Punkt 1 (0,0) zu Punkt 2 (1,1)
        drawLine(mbi,x0 ,y0,x1,y1,color);

        drawLine(mbi,x0,y0,x3,y3,color);

        drawLine(mbi,x3,y3,x2,y2,color);

        drawLine(mbi,x2,y2,x1,y1,color);

        if(x0 && y0 == x1 && y1 == x2 && y2 == x3 && y3) break;

        if(drawMode == 1){
            for(int i = y0; i < y3;i++){

                drawLine(mbi,x0,i,x1,i,color);
        
            }
        }
    }
}

int kernel_main(unsigned long magic, unsigned long address) {

    if (magic != 0x2BADB002) {
        return -1; 
    }

    struct multiboot_info* mbi = (struct multiboot_info*)address;

    if (!(mbi->flags & (1 << 12))) {
        return -2;
    }

    drawRect(mbi,5,5,100,5,100,50,5,50,RED,0);


    while(1);
    return 0;
}