#include "esp_log.h"
#include <logging.hpp>
#include "config.h"

// General hardware libs
#include "SPIFFS.h"
#include "Wire.h"
#include <Preferences.h>

// Specific hardware libs
#include "ESPAsyncWebServer.h"
#include <ESPDash.h>
#include <FastLED.h>

#include "ESPmDNS.h"

#include "StrokeEngine.h"

//#include "controller/canfuck.hpp"

#include "CO_main.h"

#include "wifi.hpp"

#include "data_logger.hpp"
#include "reset_reason.hpp"

#include "webserver.hpp"

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

#if MOTOR_TYPE == MOTOR_TYPE_LINMOT
  #include "motor/linmot.hpp"
  LinmotMotor* motor;
#elif MOTOR_TYPE == MOTOR_TYPE_VIRTUAL
  #include "motor/virtualMotor.h"
  VirtualMotor* motor;
#endif

StrokeEngine* engine;

void loop() {
  vTaskDelay(200 / portTICK_PERIOD_MS);
  // TODO - fix this stack overflow
  //ws.cleanupClients();
}

void boot_setup() {
  Serial.begin(UART_SPEED);
  print_reset_reason();

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  FastLED.addLeds<NEOPIXEL, 21>(leds, NUM_LEDS);
  leds[0] = CHSV(0, 180, 64);
  FastLED.show();
  
  MDNS.begin("ossm");
}

#if LVGL_AVAILABLE == 1
#include "lvgl_gui.hpp"
#include "screen/boot.hpp"
#include "screen/status.hpp"

BootScreen* bootScreen = NULL;
void boot_show_logo () {
  bootScreen = new BootScreen();
  LVGLGui::getInstance()->activate(bootScreen);

  // Allow the boot screen to show for a bit
  for (uint8_t i = 0; i < (BOOT_SCREEN_WAIT / 50); i++) { vTaskDelay(50 / portTICK_PERIOD_MS); }
}

StatusScreen* statusScreen = NULL;
void boot_show_status() {
  statusScreen = new StatusScreen();
  LVGLGui::getInstance()->activate(statusScreen);
}
#elif
void boot_setup_lvgl() {}
void boot_show_logo() {}
#endif

void storage_setup() {
  log_i("Mounting SPIFFS");
  if(!SPIFFS.begin(true)){
    log_e("An Error has occurred while mounting SPIFFS");
    return;
  }
}

/*
void espnow_setup() {
  WiFi.mode(WIFI_STA);
  log_i("MAC Address: %s", WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    log_e("Error initializing ESP-NOW");
    return;
  }
}
*/

/*
void blynk_setup() {
  log_i("Starting Blynk!");
  blynk = new BlynkController();
}
*/

void setup() {
  boot_setup();

  Config::functionality.read([&](FunctionalityState& state) {
    if (state.lvglEnabled) {
      LVGLGui::getInstance(); // TODO - Hack to initialize LVGL
      boot_show_logo();
    }
  });

  wifi_setup();
  //blynk_setup();
  storage_setup();
  server_setup();
  
  Config::functionality.read([&](FunctionalityState& state) {
    if (state.lvglEnabled) {
      boot_show_status();
    }
  });

/*
  if (CONTROLLER_USED) {
    log_i("Initializing Hardware Controller");
    controller = new CANFuckController();
    if (controller->start() == false) {
      log_e("Unable to initialize Hardware Controller");
      esp_restart();
    }
  } else {
    log_i("No Hardware Controller, only Blynk Controls will be available!");
  }
  */

  log_i("Configuring Motor");

#if MOTOR_TYPE == MOTOR_TYPE_LINMOT
  motor = new LinmotMotor();

  motor->setMachineGeometry(110, -7, 10);
  motor->setMaxSpeed(5000); // 5 m/s
  motor->setMaxAcceleration(500); // 25 m/s^2

  motor->CO_setNodeId(CANOPEN_NODEID_LINMOT);
  motor->CO_setStatusUInt16(OD_ENTRY_H2110_linMotStatusUInt16);
  motor->CO_setMonitorSInt16(OD_ENTRY_H2114_linMotStatusSInt16);
  motor->CO_setMonitorSInt32(OD_ENTRY_H2115_linMotStatusSInt32);
  motor->CO_setControl(OD_ENTRY_H2111_linMotControlWord);
  motor->CO_setCmdHeader(OD_ENTRY_H2112_linMotCMD_Header);
  motor->CO_setCmdParameters(OD_ENTRY_H2113_linMotCMD_Parameters);
#elif MOTOR_TYPE == MOTOR_TYPE_VIRTUAL
  motor = new VirtualMotor();

  motor->setMaxSpeed(5000); // 5 m/s
  motor->setMaxAcceleration(500); // 25 m/s^2
  motor->setMachineGeometry(0, 100, 10);
#endif

  engine = new StrokeEngine();
  log_i("Attaching Motor to Stroke Engine");
  engine->attachMotor(motor);

/*
  if (controller != NULL) {
  log_i("Attaching Controller to Stroke Engine");
    controller->attachEngine(engine);
  }

  log_i("Attaching Blynk to Motor and Engine");
  blynk->attach(engine, motor);
  */

  log_i("Configuring Stroke Engine");
  engine->setParameter(StrokeParameter::RATE, 50);
  engine->setParameter(StrokeParameter::DEPTH, 100);
  engine->setParameter(StrokeParameter::STROKE, 50);
  engine->setParameter(StrokeParameter::SENSATION, 0);
  
  /*
  // TODO - Move controller tasks into init/attach
  if (controller != NULL) {
    controller->registerTasks();
  }
  */

  log_i("Homing Motor");
  motor->enable();
  motor->home();
}
