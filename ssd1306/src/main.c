#include <ssd1306.h>
//... some periphery inits
int main() {
 // ...
  ssd1306_init();
  ssd1306_fill(SSD1306_COLOR_BLACK);
  ssd1306_puts("Test", &Font_11x18, SSD1306_COLOR_WHITE);
  ssd1306_update_screen();
 // ...
}
