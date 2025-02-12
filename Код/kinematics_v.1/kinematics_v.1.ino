#include <AccelStepper.h>

#define X_STEP_PIN 54
#define X_DIR_PIN 55
#define X_ENABLE_PIN 38

#define Y_STEP_PIN 60
#define Y_DIR_PIN 61
#define Y_ENABLE_PIN 56

#define Z_STEP_PIN 46
#define Z_DIR_PIN 48
#define Z_ENABLE_PIN 62

#define E_STEP_PIN 26
#define E_DIR_PIN 28
#define E_ENABLE_PIN 24

#define ACCELERATION 10000000
#define MAX_SPEED 1000

#define ONE_STEP  0.1 // в одном шаге 0.1мм

#define R 405  //мм, длина ножки
long R_2 = long(R)*long(R);

#define L 1000 //мм, длина принтера (меньше чем на самом деле)

#define r 70   //мм, расстояние между ножками (на верхней платформе)
float r_sqrt_2 = r/sqrt(2);

#define T_WEIGHT_HALF 115 // мм половина ширины принтера

#define EXT_HEIGHT 70 // мм, высота головки (над верхней платформой)

struct Carriages {
  int car_x, car_y, car_z, car_e; // абсолютные координаты кареток в мм
};

struct Position {
  int X, Y, Z;  // абсолютные координаты сопла в мм   
};

Carriages cars;
Position head_pos;

AccelStepper stepper_x(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepper_y(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN); 
AccelStepper stepper_z(AccelStepper::DRIVER, Z_STEP_PIN, Z_DIR_PIN); 
AccelStepper stepper_e(AccelStepper::DRIVER, E_STEP_PIN, E_DIR_PIN);



void init_stepper(AccelStepper& stepper, const uint8_t& pin, const int& pos)
{
  stepper.setEnablePin(pin); 
  stepper.setPinsInverted(false, false, true); // почему так?
  stepper.enableOutputs();
  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);
  stepper.setCurrentPosition(pos);
}

void set_new_pos(Carriages& cars, const Position& new_pos)
{
  long common_part = R_2 - long(new_pos.Z - EXT_HEIGHT)*long(new_pos.Z - EXT_HEIGHT);
  float dY_0 = -T_WEIGHT_HALF - (new_pos.Y-r_sqrt_2);
  float dY_1 = T_WEIGHT_HALF - (new_pos.Y+r_sqrt_2);

  float under_root_0 = common_part - (dY_0)*(dY_0);
  float under_root_1 = common_part - (dY_1)*(dY_1);
  
  if (under_root_0 >= 0 && under_root_1 >= 0)
  {
    int delta_0 = r_sqrt_2 + sqrt(under_root_0);
    int delta_l = r_sqrt_2 + sqrt(under_root_1); // в миллиметрах
    
    cars.car_x = new_pos.X - delta_0;
    cars.car_y = new_pos.X + delta_0;
    cars.car_z = new_pos.X + delta_l;
    cars.car_e = new_pos.X - delta_l;
  }
  else
  {
    Serial.println("Wrong Destination");
  }
}

void setup() {
  Serial.begin(9600);
  init_stepper(stepper_x, X_ENABLE_PIN, 0);
  init_stepper(stepper_y, Y_ENABLE_PIN, 140/ONE_STEP);
  init_stepper(stepper_z, Z_ENABLE_PIN, -140/ONE_STEP); 
  init_stepper(stepper_e, E_ENABLE_PIN, 0);
}

const Position plan[] = {{70, 0, 430},{110,0,430},{142,25,430},{170,50,430},{170,-50,430},{143,-25,430},{110,0,430},{210,0,430},{219,12,430},{230,25,430},{230,-25, 430},{219,-12,430},{210, 0, 430}};


void loop() {

  for(int i = 0; i < 13; i++){
  
  head_pos = plan[i];
  
  set_new_pos(cars, head_pos);
  
  stepper_x.moveTo(-cars.car_x / ONE_STEP);
  stepper_y.moveTo(cars.car_y / ONE_STEP);
  stepper_z.moveTo(-cars.car_z / ONE_STEP); 
  stepper_e.moveTo(cars.car_e / ONE_STEP);
  Serial.println(cars.car_x);
  Serial.println(cars.car_y);
  Serial.println(cars.car_z);
  Serial.println(cars.car_e);
  while ( (stepper_x.distanceToGo() != 0) || (stepper_y.distanceToGo() != 0) || (stepper_z.distanceToGo() != 0) || (stepper_e.distanceToGo() != 0) )
      {
        stepper_x.run();
        stepper_y.run();
        stepper_z.run();
        stepper_e.run();
     }
  delay(1000);
  }
}
