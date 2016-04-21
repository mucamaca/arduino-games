/**
 * The only currently existing game.
 */

#define EEPROM_START 0
#define EEPROM_END 4
#define EEPROM_UNIT_LENGTH 4

#include <Adafruit_ssd1306syp.h>
#include <EEPROM.h>
#define SDA_PIN 13
#define SCL_PIN 12

Adafruit_ssd1306syp display(SDA_PIN, SCL_PIN);

struct piece{
  char x;
  char y;
};

struct snake_t{
  struct piece *arr;
  char x;
  char y;
  signed char ldx;
  signed char ldy;
  char index;
  char len;
};

static char obstacle_len = 0;
static struct snake_t *snake;
static struct piece *obst;
static struct piece food;


void update_snake(){
  snake->arr[snake->index].x = snake->x;
  snake->arr[snake->index].y = snake->y;
  snake->index = (snake->index + 1) % snake->len;
}
  

void move(){
  signed char dx = 0, dy = 0;
  if(analogRead(5) - 512 < -100){
    dx = -1;
  }
  else if(analogRead(5) - 512 > 100){
    dx = 1;
  }
  if(analogRead(4) - 512 < -100){
    dy = 1;
  }
  else if(analogRead(4) - 512 > 100){
    dy = -1;
  }
  bool mov = 1;
  if(dx && !dy)
    snake->x += 4 * dx;
  else if(dy && !dx)
    snake->y += 4 * dy;
  else if(dy && dx){
    snake->y += 2 * dy;
    snake->x += 2 * dx;
  }else{
    snake->x += 2 * snake->ldx;
    snake->y += 2 * snake->ldy;
    mov = 0;
  }
  if(mov){
    snake->ldx = dx;
    snake->ldy = dy;
  }
  
  //TODO popravi tole, da se ne bo dalo ful ostro zavijat
}

void generate_food(){
  char i;
  for(i=0; i < snake->len; i++){
    if(snake->arr[i].x == food.x && snake->arr[i].y == food.y)
      food.x = random(0, 128);
      food.y = random(0, 64);
  }
}

void eat(){
  snake->len++;
  generate_food();
}

bool collision_check(char x, char y){
  if(display.m_pFramebuffer[y / 8 * 128 + x] & 1 << (y % 8))
    return 1;
  else
    return 0;
}

void collision(){
  if(collision_check(snake->x, snake->y))
    if(snake->x == food.x && snake->y == food.y){
      //eat();
      snake->len++;
    }else
      end_game();
}

void update(){
  move();
  collision();
  update_snake();
}

void draw_snake(){
  char i = 0;
  while(i++ < snake->len){
    if((i + 1) % snake->len != snake->index)
      display.drawLine(snake->arr[i % snake->len].x, snake->arr[i % snake->len].y, 
           snake->arr[(i + 1) % snake->len].x, snake->arr[(i + 1) % snake->len].y, WHITE);
  }
}

void update_records(){
  char i, val;
  char shift=1;
  for(i = EEPROM_START + 1; i < EEPROM_END; i++){
    val = EEPROM[i];
    if(shift){
      val >>= 4;
    }
    if(snake->len > val){
      // push everything down cliff;
    }
  }
}

void end_game(){
  char i;
  delay(200);
  Serial.println(snake->len);
  //update_records();
  free(snake);
  free(obst);
  while(1);
}

void render(){
  display.clear();
  draw_snake();
  display.drawPixel(snake->x, snake->y, WHITE);
  display.drawPixel(food.x, food.y, WHITE);
  display.update();
}

void snake_init(){
  snake->len = 5;
  snake->arr = (struct piece *)malloc(sizeof(struct piece) * snake->len * 2);
  if(snake->arr == NULL)
    exit(1);
  snake->x = 50;
  snake->y = 50;
  snake->ldy = -1;
}

void setup(){
  delay(1000);
  Serial.begin(9600);
  display.initialize();
  display.clear();
  snake=(struct snake_t *)malloc(sizeof(struct snake_t));
  if(snake == NULL ){
    Serial.println("you retard!");
    exit(1); // to je temporary. drgac ne bo kr koncal programa zarad null pointerja
  }
  snake_init();

  randomSeed(micros()+(int)analogRead(4));
  food.x=42;
  food.y=42;
  char i;
  for(i = 1; i < snake->len * 4; i += 4){
    display.drawLine(snake->x, snake->y + i, snake->x, snake->y + i + 4, WHITE);
  }
}

void loop(){
  update();
  render();
}



