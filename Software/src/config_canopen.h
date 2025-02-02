#define CANOPEN_DEFAULT FUNCTIONALITY_ENABLED
#define CANOPEN_RX_PIN GPIO_NUM_36
#define CANOPEN_TX_PIN GPIO_NUM_35
#define CANOPEN_BAUD_RATE 500 /* kbit */
#define CANOPEN_START_WAIT 500
#define CANOPEN_NODEID_SELF 0x01
#define CANOPEN_NODEID_LINMOT 0x3F
#define CANOPEN_OD_EXTENSIONS CANOPEN_OD_A1100

#define LVGL_DEFAULT FUNCTIONALITY_ENABLED

#define MODBUS_DEFAULT FUNCTIONALITY_DISABLED
#define MODBUS_RX_PIN GPIO_NUM_NC
#define MODBUS_TX_PIN GPIO_NUM_NC
#define MODBUS_BAUD_RATE 57600
#define MODBUS_SERIAL SERIAL_8E1

#define VCP_DEFAULT 1
#define VCP_BUFFER_SIZE 1024
#define VCP_BAUD_RATE 38400
#define VCP_RX_PIN GPIO_NUM_44
#define VCP_TX_PIN GPIO_NUM_43

#define I2C_SDA_PIN 36
#define I2C_SCL_PIN 37
